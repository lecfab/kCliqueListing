#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>

using namespace std;
using namespace chrono;

class edge {
public:
  edge(int s = 0, int t = 0) : s(s), t(t) {}

  int s;
  int t;
};

class iddeg {
public:
  int id;
  int degree;
};

class Graph {
public:
  Graph();
  Graph(const Graph &obj);
  ~Graph();
  void number_of_nodes_and_edges(string edgelist);
  void readedgelist(string edgelist);
  void coreDecomposition();
  void mkGraph();
  int outLargeClique();
  bool isEdge(int, int);
  Graph *mksub(int, ...);
  // Graph* mksubMark(int*, int, int*);
  int color(int *);
  bool isCliqueV(int *, int);
  void orderByDegree();
  void orderByDegeneracy();
  void initAdjList();
  void setInOutAdj();
  // bool isEdge(int, int);

  int n;
  int m;
  int maxDeg, maxDegv;
  edge *edges;

  int *deg, *degv, *indegv;
  int *cd, *cdv, *incdv;
  int *adj, *adjv, *inadjv;
  int *coreRank; // increasing core number order
  int *coreNum;  // coreNum[i] is the core number of node i.
  int *bin;
  int *Ordering;
};

inline int max3(int a, int b, int c) {
  a = (a > b) ? a : b;
  return (a > c) ? a : c;
}

Graph::Graph(void) {}
Graph::~Graph(void) {
  if (deg != NULL)
    delete[] deg;
  if (cd != NULL)
    delete[] cd;
  if (adj != NULL)
    delete[] adj;
  if (degv != NULL)
    delete[] degv;
  if (cdv != NULL)
    delete[] cdv;
  if (adjv != NULL)
    delete[] adjv;
  if (coreRank != NULL)
    delete[] coreRank;
  if (coreNum != NULL)
    delete[] coreNum;
  if (bin != NULL)
    delete[] bin;
  if (Ordering != NULL)
    delete[] Ordering;
}
Graph::Graph(const Graph &obj) {
  n = obj.n, m = obj.m, maxDeg = obj.maxDeg, edges = obj.edges;
  edges = obj.edges;

  if (deg != NULL)
    delete[] deg;
  if (obj.deg != NULL)
    deg = new int[n], memcpy(deg, obj.deg, n * sizeof(int));

  if (cd != NULL)
    delete[] cd;
  if (obj.cd != NULL)
    cd = new int[n + 1], memcpy(cd, obj.cd, (n + 1) * sizeof(int));

  if (adj != NULL)
    delete[] adj;
  if (obj.adj != NULL)
    adj = new int[2 * m], memcpy(adj, obj.adj, 2 * m * sizeof(int));

  if (coreRank != NULL)
    delete[] coreRank;
  if (obj.coreRank != NULL)
    coreRank = new int[n], memcpy(coreRank, obj.coreRank, n * sizeof(int));

  if (coreNum != NULL)
    delete[] coreNum;
  if (obj.coreNum != NULL)
    coreNum = new int[n], memcpy(coreNum, obj.coreNum, n * sizeof(int));

  if (bin != NULL)
    delete[] bin;
  if (obj.bin != NULL)
    bin = new int[maxDeg + 2], memcpy(bin, obj.bin, (maxDeg + 2) * sizeof(int));

  if (Ordering != NULL)
    delete[] Ordering;
  if (obj.Ordering != NULL)
    Ordering = new int[n], memcpy(Ordering, obj.Ordering, n * sizeof(int));
}

// modification for standard input dataset format (without giving n and m)
void Graph::number_of_nodes_and_edges(string edgelist) {
  ifstream file;
  file.open(edgelist);
  m = 0, n = 0;
  int u, v;
  while (file >> u >> v) {
    m++;
    n = max3(n, u + 1, v + 1);
  }
  file.close();
}
void Graph::readedgelist(string edgelist) {
  number_of_nodes_and_edges(edgelist); // file >> n >> m;
  ifstream file;
  file.open(edgelist);
  edges = new edge[m];
  m = 0;
  while (file >> edges[m].s >> edges[m].t)
    m++;
  file.close();
}

void Graph::mkGraph() {
  deg = new int[n]();
  cd = new int[n + 1];
  adj = new int[2 * m];
  maxDeg = 0;
  for (int i = 0; i < m; i++) {
    deg[edges[i].s]++;
    deg[edges[i].t]++;
    maxDeg = max3(maxDeg, deg[edges[i].s], deg[edges[i].t]);
  }
  cd[0] = 0;
  for (int i = 1; i < n + 1; i++) {
    cd[i] = cd[i - 1] + deg[i - 1];
    deg[i - 1] = 0;
  }

  for (int i = 0; i < m; i++) {
    adj[cd[edges[i].s] + deg[edges[i].s]++] = edges[i].t;
    adj[cd[edges[i].t] + deg[edges[i].t]++] = edges[i].s;
  }

  for (int i = 0; i < n; i++)
    sort(adj + cd[i], adj + cd[i] + deg[i]);
}

bool cmp(const pair<int, int> &a, const pair<int, int> &b) {
  return a.second > b.second;
}
bool IGCmp(const iddeg &a, const iddeg &b) {
  return a.degree == b.degree ? (a.id < b.id) : (a.degree > b.degree);
}

bool Graph::isEdge(int a, int b) {
  if (deg[a] > deg[b])
    a = a ^ b, b = a ^ b, a = a ^ b;
  for (int i = cd[a]; i < cd[a] + deg[a]; i++)
    if (adj[i] == b)
      return true;
  return false;
}

void Graph::coreDecomposition() {
  bin = new int[maxDeg + 2]();

  for (int i = 0; i < n; i++)
    bin[deg[i]]++;

  int lastBin = bin[0], nowBin;
  bin[0] = 0;
  for (int i = 1; i <= maxDeg; i++) {
    nowBin = lastBin + bin[i - 1];
    lastBin = bin[i];
    bin[i] = nowBin;
  }
  int *vert = new int[n](), *pos = new int[n](), *tmpDeg = new int[n]();
  for (int i = 0; i < n; i++) {
    pos[i] = bin[deg[i]];

    vert[bin[deg[i]]++] = i;
    tmpDeg[i] = deg[i];
  }

  bin[0] = 0;
  for (int i = maxDeg; i >= 1; i--) {
    bin[i] = bin[i - 1];
  }

  // int core = 0;
  int *cNum = new int[n];
  // int *cNum = (int *)malloc(g->n * sizeof(int));
  for (int i = 0; i < n; i++) {
    int id = vert[i], nbr, binFrontId;
    // if (i == bin[core + 1]) ++core;
    cNum[id] = tmpDeg[id];
    for (int i = cd[id]; i < cd[id] + deg[id]; i++) {
      nbr = adj[i];

      if (tmpDeg[nbr] > tmpDeg[id]) {
        binFrontId = vert[bin[tmpDeg[nbr]]];
        if (binFrontId != nbr) {

          pos[binFrontId] = pos[nbr];
          pos[nbr] = bin[tmpDeg[nbr]];
          vert[bin[tmpDeg[nbr]]] = nbr;
          vert[pos[binFrontId]] = binFrontId;
        }
        bin[tmpDeg[nbr]]++;
        tmpDeg[nbr]--;
      }
    }
  }

  coreNum = cNum;

  coreRank = vert;

  delete[] tmpDeg;
  delete[] pos;
}

void Graph::orderByDegree() {
  for (int i = 0; i < n; i++)
    for (int j = cd[i]; j < cd[i] + deg[i]; j++) {
      int v = adj[j];
      if ((deg[i] < deg[v]) || ((deg[i] == deg[v]) && (i < v)))
        degv[i]++, indegv[v]++;
    }

  int *counter = new int[n]();
  for (int i = 0; i < n; i++)
    counter[deg[i]]++;
  for (int i = 1; i < n; i++)
    counter[i] += counter[i - 1];
  for (int i = n - 1; i >= 0; i--)
    Ordering[i] = --counter[deg[i]];
  delete[] counter;
  // cout << "445566" << endl; // ???
}

void Graph::orderByDegeneracy() {
  coreDecomposition();
  for (int i = 0; i < n; i++)
    Ordering[coreRank[i]] = i;

  for (int i = 0; i < n; i++)
    for (int j = cd[i]; j < cd[i] + deg[i]; j++) {
      int v = adj[j];
      if (Ordering[i] < Ordering[v])
        degv[i]++, indegv[v]++;
    }
}

void Graph::initAdjList() {
  degv = new int[n]();
  cdv = new int[n];
  adjv = new int[m];

  indegv = new int[n]();
  incdv = new int[n];
  inadjv = new int[m];

  Ordering = new int[n];
}

void Graph::setInOutAdj() {
  cdv[0] = 0, degv[n - 1] = 0;
  incdv[0] = 0, indegv[n - 1] = 0;
  for (int i = 1; i < n; i++) {
    maxDegv = max(degv[i - 1], maxDegv);
    cdv[i] = cdv[i - 1] + degv[i - 1];
    degv[i - 1] = 0;

    incdv[i] = incdv[i - 1] + indegv[i - 1];
    indegv[i - 1] = 0;
  }
  cout << "maxDegv : " << maxDegv << endl;

  for (int i = 0; i < n; i++)
    for (int j = cd[i]; j < cd[i] + deg[i]; j++) {
      int v = adj[j];
      // if ((deg[i] < deg[v]) || ((deg[i] == deg[v]) && (i < v))) adjv[cdv[i] +
      // degv[i]++] = v, inadjv[incdv[v] + indegv[v]++] = i;
      if (Ordering[i] < Ordering[v])
        adjv[cdv[i] + degv[i]++] = v, inadjv[incdv[v] + indegv[v]++] = i;
    }
}

// using L (long edge of the triangle) leads to complexity:
// sum_(u in V) [ sum_(uw in Nu+) 1 + sum_(v in Nu+) sum_(w in Nv-) 1 ]
// = sum_u [ du+ + sum_(v in Nu+) dv-]
// = m + sum_v dv- * dv-
// So it seems to be a comlexity dmm
long long LwithOrdering(Graph *g) { // correction by F Lecuyer, 2021
  long long ans = 0;
  bool *mark = new bool[g->n]();
  for (int i = 0; i < g->n; i++) { // for each node u
    int u = i, v;
    for (int j = 0; j < g->degv[u]; j++) { // mark each successor w of u
      int w = g->adjv[g->cdv[u] + j];
      mark[w] = true;
    }
    for (int j = 0; j < g->degv[u]; j++) { // for each successor v of u
      v = g->adjv[g->cdv[u] + j];
      for (int k = 0; k < g->indegv[v]; k++) { // take each predecessor w of v
        int w = g->inadjv[g->incdv[v] + k];
        if (mark[w]) // if w is marked, count a triangle
          ans++;
      }
    }
    for (int j = 0; j < g->degv[u]; j++) { // unmark
      int w = g->adjv[g->cdv[u] + j];
      mark[w] = false;
    }
  }
  delete[] mark;
  return ans;
}

int main(int argc, char **argv) {
  high_resolution_clock::time_point precis_t0 = high_resolution_clock::now();
  time_t t0, t1, t2;
  t1 = time(NULL);
  t0 = t1;

  Graph *g = new Graph;

  int k = atoi(argv[1]);
  cout << "Reading edgelist from file " << argv[2] << endl;
  g->readedgelist(argv[2]);
  // cout << "Reading edgelist finished!" << endl;
  printf("Number of nodes = %u\n", g->n);
  printf("Number of edges = %u\n", g->m);

  int typeoforder = atoi(argv[3]); // Degree or Smallest-first ordering

  g->mkGraph();
  // g->mv = g->m / 2;

  t2 = time(NULL);
  printf("- Time = %ldh%ldm%lds\n", (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60,
         ((t2 - t1) % 60));
  t1 = t2;

  // cout << "mkGraph finished!" << endl;
  g->initAdjList();
  if (typeoforder == 1) {
    printf("Ordering by degree\n");
    g->orderByDegree();
  }
  if (typeoforder == 2) {
    printf("Ordering by degeneracy\n");
    g->orderByDegeneracy();
  }
  g->setInOutAdj();
  t2 = time(NULL);
  printf("- Time = %ldh%ldm%lds\n", (t2 - t1) / 3600, ((t2 - t1) % 3600) / 60,
         ((t2 - t1) % 60));
  t1 = t2;

  auto precis_t1 = high_resolution_clock::now();
  long long triNum = LwithOrdering(g); // Degree or Smallest-first ordering

  cout << "Number of triangles: " << triNum << endl;

  auto precis_t2 = high_resolution_clock::now();
  auto precis_t1_2 =
      chrono::duration_cast<chrono::microseconds>(precis_t2 - precis_t1)
          .count();
  auto precis_t0_2 =
      chrono::duration_cast<chrono::microseconds>(precis_t2 - precis_t0)
          .count();
  // cout << "compute time: " << t1_2 / 1e6 << endl;
  // cout << "Toltal time: " << t0_2 / 1e6 << endl;
  t2 = time(NULL);
  printf("- Time = %ldh%ldm%lds\t\t(%ldms)\n", (t2 - t1) / 3600,
         ((t2 - t1) % 3600) / 60, ((t2 - t1) % 60),
         (long int)(precis_t1_2 / 1e3));
  t1 = t2;
  // precis_t1 = precis_t2;


  printf("- Overall time = %ldh%ldm%lds\t\t(%ldms)\n", (t2 - t0) / 3600,
         ((t2 - t0) % 3600) / 60, ((t2 - t0) % 60),
         (long int)(precis_t0_2 / 1e3));

  return 0;
}
