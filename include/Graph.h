
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <vector>

using namespace std;

struct Vertex {
	float x;
	float y;
	bool selected;
	vector<Vertex*> adjout;
	vector<Vertex*> adjin;
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
	int arrowType(Vertex *v, Vertex *w);

	void remove(Vertex *v);
	void mergeSelected();
	void select(Vertex *v, bool s);
	void selectAll(bool s);

	void setDisconnected(Vertex *v, Vertex *w);
	void setConnected(Vertex *v, Vertex *w);
	void setConnected(Vertex *v, Vertex *w, int mode);
	void setArrow(Vertex *v, Vertex *w);
	void addArrow(Vertex *v, Vertex *w);

	vector<Vertex*> vertices;
	vector<Vertex*> selected;
};

#endif
