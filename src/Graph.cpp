
#include <Graph.h>
#include <queue>
#include <algorithm>
#include <iostream>
#include <cmath>

Graph::Graph() {
	// TODO Auto-generated constructor stub

}

Graph::~Graph() {
	// TODO Auto-generated destructor stub
}

Vertex * Graph::insert(float x, float y, int color) {
	Vertex *v = new Vertex;
	v->x = x;
	v->y = y;
	v->selected = false;
	v->color = color;
	v->adjout = unordered_map<Vertex*, bool>();
	v->adjin = unordered_set<Vertex*>();
	vertices.insert(v);
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
	unordered_set<Vertex*> visited;
	queue<Vertex*> q;
	q.push(v);
	while(!q.empty()) {
		v = q.front();
		q.pop();
		result.push_back(v);
		for(pair<Vertex*, bool> p : v->adjout) {
			if(!visited.count(p.first)) {
				visited.insert(p.first);
				q.push(p.first);
			}
		}
		for(Vertex *v2 : v->adjin) {
			if(!visited.count(v2)) {
				visited.insert(v2);
				q.push(v2);
			}
		}
	}
	return result;
}

bool Graph::adjacent(Vertex *v, Vertex *w) {
	return v->adjout.count(w);
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
	for(Vertex *v2 : v->adjin) {
		v2->adjout.erase(v);
	}
	for(pair<Vertex*, bool> p : v->adjout) {
		p.first->adjin.erase(v);
	}
	vertices.erase(v);
	selected.erase(v);
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
		for(pair<Vertex*, bool> p : v->adjout) {
			addArrow(u, p.first);
		}
		for(Vertex *v2 : v->adjin) {
			addArrow(v2, u);
		}
	}
	while(!selected.empty()) {
		remove(*selected.begin());
	}
	select(u, true);
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
		selected.clear();
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
void Graph::selectEdge(Vertex *v, Vertex *w, bool s) {
	if(adjacent(v, w)) {
		v->adjout[w] = s;
	}
	if(adjacent(w, v)) {
		w->adjout[v] = s;
	}
}


void Graph::setDisconnected(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	v->adjout.erase(w);
	v->adjin.erase(w);
	w->adjout.erase(v);
	w->adjin.erase(v);
}
void Graph::setConnected(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	v->adjout[w] = false;
	v->adjin.insert(w);
	w->adjout[v] = false;
	w->adjin.insert(v);
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
	v->adjout[w] = false;
	w->adjin.insert(v);
	w->adjout.erase(v);
	v->adjin.erase(w);
}
void Graph::addArrow(Vertex *v, Vertex *w) {
	if(v == w) {
		return;
	}
	v->adjout[w] = false;
	w->adjin.insert(v);
}

void Graph::normalize(float dt) {
	unordered_map<Vertex*, pair<float, float>> m;
	for(Vertex *v : vertices) {
		float totx = 0, toty = 0;
		for(Vertex *w : vertices) {
			if(v == w) continue;
			float dx = w->x - v->x;
			float dy = w->y - v->y;
			float r = hypot(dx, dy);
			if(w->adjin.count(v)) {
				totx += dx * r;
				toty += dy * r;
			}
			totx -= dx / r * 10'000;
			toty -= dy / r * 10'000;
		}
		m[v] = {totx, toty};
	}
	for(Vertex *v : vertices) {
		float r = dt;
		v->x += m[v].first * r;
		v->y += m[v].second * r;
	}
}
