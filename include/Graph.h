
#ifndef GRAPH_H_
#define GRAPH_H_

#include <cstdio>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;

struct Vertex {
	float x;
	float y;
	int color;
	bool selected;
	unordered_map<Vertex*, bool> adjout;
	unordered_set<Vertex*> adjin;
};

class Graph {
public:
	Graph();
	virtual ~Graph();

	Vertex *insert(float x, float y, int color = 0);
	Vertex *getVertex(float x, float y, float r);
	vector<Vertex*> getVertices(float minx, float miny, float maxx, float maxy);
	vector<Vertex*> getComponent(Vertex *v);
	bool adjacent(Vertex *v, Vertex *w);
	int arrowType(Vertex *v, Vertex *w);

	void remove(Vertex *v);
	void mergeSelected();
	void select(Vertex *v, bool s);
	void selectAll(bool s);
	void selectEdge(Vertex *v, Vertex *w);

	void setDisconnected(Vertex *v, Vertex *w);
	void setConnected(Vertex *v, Vertex *w);
	void setConnected(Vertex *v, Vertex *w, int mode);
	void setArrow(Vertex *v, Vertex *w);
	void addArrow(Vertex *v, Vertex *w);

	unordered_set<Vertex*> vertices;
	unordered_set<Vertex*> selected;
};

#endif
