
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

Vertex * Graph::insert(float x, float y, int color) {
	Vertex *v = new Vertex;
	v->x = x;
	v->y = y;
	v->selected = false;
	v->color = color;
	v->adjin = vector<Vertex*>();
	v->adjout = vector<Vertex*>();
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
		for(Vertex *v2 : v->adjout) {
			if(!visited[v2->index]) {
				visited[v2->index] = true;
				q.push(v2);
			}
		}
		for(Vertex *v2 : v->adjin) {
			if(!visited[v2->index]) {
				visited[v2->index] = true;
				q.push(v2);
			}
		}
	}
	return result;
}

bool Graph::adjacent(Vertex *v, Vertex *w) {
	return find(v->adjout.begin(), v->adjout.end(), w) != v->adjout.end();
}
int Graph::arrowType(Vertex *v, Vertex *w) {
	bool b1 = adjacent(v, w);
	bool b2 = adjacent(w, v);
	if(b1 && b2) return 0;
	else if(b1) return 1;
	else if(b2) return 2;
	return -1;
}

void Graph::remove(Vertex *v) {
	// get rid of all references to v, including from another vertex's adjacency list.
	for(Vertex *v2 : vertices) {
		removeFromList(v2->adjout, v);
		removeFromList(v2->adjin, v);
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
		for(Vertex *v2 : v->adjout) {
			addToList(u->adjout, v2);
			addToList(v2->adjin, u);
		}
		for(Vertex *v2 : v->adjin) {
			addToList(v2->adjout, u);
			addToList(u->adjin, v2);
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

void Graph::setDisconnected(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	removeFromList(v->adjout, w);
	removeFromList(v->adjin, w);
	removeFromList(w->adjout, v);
	removeFromList(w->adjin, v);
}
void Graph::setConnected(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	addToList(v->adjout, w);
	addToList(w->adjout, v);
	addToList(v->adjin, w);
	addToList(w->adjin, v);
}
void Graph::setConnected(Vertex *v, Vertex *w, int mode) {
	if(mode == 0) {
		setConnected(v, w);
	}else if(mode == 1) {
		setArrow(v, w);
	}else if(mode == 2) {
		setArrow(w, v);
	}
}
void Graph::setArrow(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	addToList(v->adjout, w);
	addToList(w->adjin, v);
	removeFromList(w->adjout, v);
	removeFromList(v->adjin, w);
}
void Graph::addArrow(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	addToList(v->adjout, w);
	addToList(w->adjin, v);
}
