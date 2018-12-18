
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <set>

using namespace std;

struct Vertex {
	float x;
	float y;
	set<Vertex*> adj;
};

class Graph {
public:
	Graph();
	virtual ~Graph();

	Vertex *insert(float x, float y);
	Vertex *getVertex(float x, float y, float r);
	void remove(Vertex *v);

	void setConnected(Vertex *v, Vertex *w, bool b);

	set<Vertex*> vertices;
};

#endif
