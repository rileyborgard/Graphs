
#include <Graph.h>
#include <queue>
#include <algorithm>
#include <iostream>

Graph::Graph() {
	// TODO Auto-generated constructor stub

}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

void removeFromList(vector<Vertex*> &list, Vertex *v) {
	vector<Vertex*>::iterator it = find(list.begin(), list.end(), v);
	if(it != list.end()) {
		list.erase(it);
	}
}
void addToList(vector<Vertex*> &list, Vertex *v) {
	vector<Vertex*>::iterator it = find(list.begin(), list.end(), v);
	if(it == list.end()) {
		list.push_back(v);
	}
}

Vertex * Graph::insert(float x, float y) {
	Vertex *v = new Vertex;
	v->x = x;
	v->y = y;
	v->selected = false;
	v->adj = vector<Vertex*>();
	v->index = vertices.size();
	vertices.push_back(v);
	return v;
}
Vertex * Graph::getVertex(float x, float y, float r) {
	float mindist = r * r;
	Vertex *minV = NULL;
	for(Vertex *v : vertices) {
		float dist = (v->x - x) * (v->x - x) + (v->y - y) * (v->y - y);
		if(dist <= mindist) {
			mindist = dist;
			minV = v;
		}
	}
	return minV;
}
vector<Vertex*> Graph::getVertices(float minx, float miny, float maxx, float maxy) {
	vector<Vertex*> result;
	for(Vertex *v : vertices) {
		if(v->x >= minx && v->x <= maxx && v->y >= miny && v->y <= maxy) {
			result.push_back(v);
		}
	}
	return result;
}
vector<Vertex*> Graph::getComponent(Vertex *v) {
	vector<Vertex*> result;
	vector<bool> visited(vertices.size(), false);
	queue<Vertex*> q;
	q.push(v);
	while(!q.empty()) {
		v = q.front();
		q.pop();
		result.push_back(v);
		for(Vertex *v2 : v->adj) {
			if(!visited[v2->index]) {
				visited[v2->index] = true;
				q.push(v2);
			}
		}
	}
	return result;
}

bool Graph::adjacent(Vertex *v, Vertex *w) {
	return find(v->adj.begin(), v->adj.end(), w) != v->adj.end();
}

void Graph::remove(Vertex *v) {
	// get rid of all references to v, including from another vertex's adjacency list.
	for(Vertex *v2 : vertices) {
		removeFromList(v2->adj, v);
	}
	removeFromList(vertices, v);
	removeFromList(selected, v);

	// now update the indices of following vertices
	for(unsigned int i = v->index; i < vertices.size(); i++) {
		vertices[i]->index = i;
	}
	delete v;
}

void Graph::mergeSelected() {
	if(selected.empty()) {
		return;
	}
	float x = 0;
	float y = 0;
	for(Vertex *v : selected) {
		x += v->x;
		y += v->y;
	}
	x /= selected.size();
	y /= selected.size();
	Vertex *u = insert(x, y);
	for(Vertex *v : selected) {
		for(Vertex *v2 : v->adj) {
			setConnected(u, v2, true);
		}
	}
	while(!selected.empty()) {
		remove(selected[0]);
	}
	select(u, true);
}

void Graph::select(Vertex *v, bool s) {
	v->selected = s;
	if(s) {
		addToList(selected, v);
	}else {
		removeFromList(selected, v);
	}
}
void Graph::selectAll(bool s) {
	if(s) {
		selected.clear();
		for(Vertex *v : vertices) {
			v->selected = true;
			selected.push_back(v);
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
		addToList(v->adj, w);
		addToList(w->adj, v);
	}else {
		removeFromList(v->adj, w);
		removeFromList(w->adj, v);
	}
}
