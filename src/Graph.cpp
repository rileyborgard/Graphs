
#include "Graph.h"

Graph::Graph() {
	// TODO Auto-generated constructor stub

}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

Vertex * Graph::insert(float x, float y) {
	Vertex *v = new Vertex;
	v->x = x;
	v->y = y;
	v->adj = set<Vertex*>();
	vertices.insert(v);
	return v;
}

void Graph::remove(Vertex *v) {
	// get rid of all references to v, including from another vertex's adjacency list.
	for(Vertex *v2 : vertices) {
		v2->adj.erase(v);
	}
	vertices.erase(v);
}

void Graph::setConnected(Vertex *v, Vertex *w, bool b) {
	if(b) {
		v->adj.insert(w);
		w->adj.insert(v);
	}else {
		v->adj.erase(w);
		w->adj.erase(v);
	}
}
