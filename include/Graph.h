
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <set>

using namespace std;

struct Vertex {
	float x;
	float y;
	bool selected;
	set<Vertex*> adj;
};

class Graph {
public:
	Graph();
	virtual ~Graph();

	Vertex *insert(float x, float y);
	Vertex *getVertex(float x, float y, float r);
	void remove(Vertex *v);
	void select(Vertex *v, bool s);
	void selectAll(bool s);

	void setConnected(Vertex *v, Vertex *w, bool b);

	set<Vertex*> vertices;
	set<Vertex*> selected;
};

#endif
