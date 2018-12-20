
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <vector>

using namespace std;

struct Vertex {
	float x;
	float y;
	bool selected;
	vector<Vertex*> adj;
	unsigned int index;
};

class Graph {
public:
	Graph();
	virtual ~Graph();

	Vertex *insert(float x, float y);
	Vertex *getVertex(float x, float y, float r);
	vector<Vertex*> getVertices(float minx, float miny, float maxx, float maxy);
	vector<Vertex*> getComponent(Vertex *v);
	bool adjacent(Vertex *v, Vertex *w);
	void remove(Vertex *v);
	void mergeSelected();
	void select(Vertex *v, bool s);
	void selectAll(bool s);

	void setConnected(Vertex *v, Vertex *w, bool b);

	vector<Vertex*> vertices;
	vector<Vertex*> selected;
};

#endif
