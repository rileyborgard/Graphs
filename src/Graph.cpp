
#include "Graph.h"
#include <queue>

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
	v->selected = false;
	v->adj = set<Vertex*>();
	vertices.insert(v);
	return v;
}
Vertex * Graph::getVertex(float x, float y, float r) {
	for(Vertex *v : vertices) {
		float dist = (v->x - x) * (v->x - x) + (v->y - y) * (v->y - y);
		if(dist < r * r) {
			return v;
		}
	}
	return NULL;
}
set<Vertex*> Graph::getVertices(float minx, float miny, float maxx, float maxy) {
	set<Vertex*> result;
	for(Vertex *v : vertices) {
		if(v->x >= minx && v->x <= maxx && v->y >= miny && v->y <= maxy) {
			result.insert(v);
		}
	}
	return result;
}
set<Vertex*> Graph::getComponent(Vertex *v) {
	set<Vertex*> result;
	queue<Vertex*> q;
	q.push(v);
	while(!q.empty()) {
		v = q.front();
		q.pop();
		result.insert(v);
		for(Vertex *v2 : v->adj) {
			if(result.find(v2) == result.end()) {
				q.push(v2);
			}
		}
	}
	return result;
}

void Graph::remove(Vertex *v) {
	// get rid of all references to v, including from another vertex's adjacency list.
	for(Vertex *v2 : vertices) {
		v2->adj.erase(v);
	}
	vertices.erase(v);
	selected.erase(v);
	delete v;
}

void Graph::select(Vertex *v, bool s) {
	v->selected = s;
	if(s) {
		selected.insert(v);
	}else {
		selected.erase(v);
	}
}
void Graph::selectAll(bool s) {
	if(s) {
		for(Vertex *v : vertices) {
			v->selected = true;
			selected.insert(v);
		}
	}else {
		for(Vertex *v : selected) {
			v->selected = false;
		}
		selected.clear();
	}
}

void Graph::setConnected(Vertex *v, Vertex *w, bool b) {
	if(v == w) {
		return;
	}
	if(b) {
		v->adj.insert(w);
		w->adj.insert(v);
	}else {
		v->adj.erase(w);
		w->adj.erase(v);
	}
}
