
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <Graph.h>
#include <set>
#include <cstdlib>
#include <string>
#include <chrono>
#include <BitmapFontClass.h>

#define MODE_SELECT 0
#define MODE_CREATE 1
#define MODE_EXTEND 2
#define MODE_EXTRUDE 3
#define MODE_EDGESELECT 4

#define ARROW_UNDIRECTED 0
#define ARROW_FORWARD 1
#define ARROW_BACKWARD 2

#define FPS 30

using namespace std;

int width = 800;
int height = 600;

Graph graph;

float vertRadius = 16;
float vertLockRadius = 32;
float arrowLength = 16;
float lineWidth = 4;
int vertPrecision = 30;
float zoomAmt = 1.05;
int mode = MODE_SELECT;
int arrowMode = ARROW_UNDIRECTED;
string modeText[] = {"MODE: select", "MODE: create", "MODE: extend", "MODE: extrude", "MODE: edge select"};
string arrowText[] = {"ARROWS: undirected", "ARROWS: forward", "ARROWS: backward"};

float col[10][3] = {{1, 1, 1}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 1, 1},
		{1, 0, 1}, {1, 0.5f, 0}, {0.5f, 0.5f, 0.5f}, {0.5, 0.25, 0}};
float highlightCol[3] = {0, 0.75, 0};
float selectCol[3] = {0, 0.5, 1};
float edgeSelectCol[3] = {1, 0.5, 0};
float outlineCol[3] = {0, 0, 0};
float backgroundCol[3] = {1, 1, 1};
float removeCol[3] = {1, 0, 0};
float createCol[3] = {0, 0.75, 0};
float textCol[3] = {0, 0, 0};

float translateX = 0;
float translateY = 0;
float zoom = 1;
float gridSpace = 32;

float mouseX;
float mouseY;
float boxMouseX;
float boxMouseY;
float removeMouseX;
float removeMouseY;
float edgeSelectMouseX;
float edgeSelectMouseY;
bool boxSelect = false;
bool edgeSelect = false;
bool dragging = false;
bool connecting = false;
bool translating = false;
bool normalizing = false;
bool removing = false;
bool showGrid = false;

bool updateDisplay = false;

unordered_map<Vertex*, Vertex*> graphCopy;
float copyX;
float copyY;

CBitmapFont font;

Vertex *connectVert;

void init() {
	glClearColor(backgroundCol[0], backgroundCol[1], backgroundCol[2], 1.f);
	glPointSize(4.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	// Turn on antialiasing, and give hint to do the best
	// job possible.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	//glEnable(GL_POINT_SMOOTH);
	//glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);

	//glEnable(GL_POLYGON_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	if(!font.Load("font/Courier New.bff")) {
		cerr << "Error loading font" << endl;
	}
}

void fillCircle(float x, float y, float r, int n) {
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < n; i++) {
		glVertex2f(x + r * cos((2 * M_PI) * i / n), y + r * sin((2 * M_PI) * i / n));
	}

	glEnd();
}
void drawCircle(float x, float y, float r, int n) {
	glBegin(GL_LINE_LOOP);

	for(int i = 0; i < n; i++) {
		glVertex2f(x + r * cos((2 * M_PI) * i / n), y + r * sin((2 * M_PI) * i / n));
	}

	glEnd();
}
void drawRect(float x1, float y1, float x2, float y2) {
	glBegin(GL_LINE_LOOP);
	glVertex2f(x1, y1);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y1);
	glEnd();
}
void drawArrow(float x1, float y1, float x2, float y2, int type) {
	if(type == ARROW_BACKWARD) {
		swap(x1, x2);
		swap(y1, y2);
	}
	float ang = atan2(y2 - y1, x2 - x1);
	glVertex2f(x1 + vertRadius * zoom * cos(ang), y1 + vertRadius * zoom * sin(ang));
	glVertex2f(x2 - vertRadius * zoom * cos(ang), y2 - vertRadius * zoom * sin(ang));

	if(type != ARROW_UNDIRECTED && (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) > zoom * zoom * vertRadius * vertRadius) {
		x1 = x2 - vertRadius * zoom * cos(ang);
		y1 = y2 - vertRadius * zoom * sin(ang);

		glVertex2f(x1, y1);
		glVertex2f(x1 + arrowLength * zoom * cos(ang + 7 * M_PI / 8),
				y1 + arrowLength * zoom * sin(ang + 7 * M_PI / 8));

		glVertex2f(x1, y1);
		glVertex2f(x1 + arrowLength * zoom * cos(ang - 7 * M_PI / 8),
				y1 + arrowLength * zoom * sin(ang - 7 * M_PI / 8));
	}
}

int orientation(float px, float py, float qx, float qy, float rx, float ry) {
	float val = (qy - py) * (rx - qx) - (qx - px) * (ry - qy);
	if(val == 0) return 0;
	return val > 0 ? 1 : 2;
}
bool intersecting(float x1, float y1, float x2, float y2,
				float X1, float Y1, float X2, float Y2) {
	int o1 = orientation(x1, y1, x2, y2, X1, Y1);
	int o2 = orientation(x1, y1, x2, y2, X2, Y2);
	int o3 = orientation(X1, Y1, X2, Y2, x1, y1);
	int o4 = orientation(X1, Y1, X2, Y2, x2, y2);
	return o1 != o2 && o3 != o4;
}

float snapX(float x, float tx, float sz) {
	x = (x - translateX) / zoom;
	int i = round((x + tx) / sz);
	return (-tx + i * sz) * zoom + translateX;
}
float snapY(float y, float ty, float sz) {
	y = (y - translateY) / zoom;
	int i = round((y + ty) / sz);
	return (-ty + i * sz) * zoom + translateY;
}
float snapX(float x) {
	float sz = gridSpace / zoom;
	while(sz < gridSpace) {
		sz *= 2;
	}
	while(sz > 2 * gridSpace) {
		sz /= 2;
	}
	float tx = translateX / zoom - sz * floor(translateX / sz);
	return snapX(x, tx, sz);
}
float snapY(float y) {
	float sz = gridSpace / zoom;
	while(sz < gridSpace) {
		sz *= 2;
	}
	while(sz > 2 * gridSpace) {
		sz /= 2;
	}
	float ty = translateY / zoom - sz * floor(translateY / sz);
	return snapY(y, ty, sz);
}

unordered_map<Vertex*, Vertex*> copySelected(float &copyX, float &copyY) {
	unordered_map<Vertex*, Vertex*> g;

	copyX = 0;
	copyY = 0;
	for(Vertex *v : graph.selected) {
		Vertex *w = new Vertex;
		w->x = v->x;
		w->y = v->y;
		w->color = v->color;
		w->adjout = unordered_map<Vertex*, bool>();
		w->adjin = unordered_set<Vertex*>();
		w->selected = false;
		g[v] = w;
		copyX += v->x;
		copyY += v->y;
	}
	if(graph.selected.size() > 0) {
		copyX /= graph.selected.size();
		copyY /= graph.selected.size();
	}
	for(Vertex *v : graph.selected) {
		for(Vertex *w : graph.selected) {
			if(v < w) continue;
			if(graph.adjacent(v, w)) {
				g[v]->adjout[g[w]] = false;
				g[w]->adjin.insert(g[v]);
			}
			if(graph.adjacent(w, v)) {
				g[w]->adjout[g[v]] = false;
				g[v]->adjin.insert(g[w]);
			}
		}
	}
	return g;
}
unordered_map<Vertex*, Vertex*> pasteSubgraph(unordered_map<Vertex*, Vertex*> g, float x, float y) {
	unordered_map<Vertex*, Vertex*> vec;
	for(pair<Vertex*, Vertex*> p : g) {
		vec[p.second] = graph.insert(p.second->x + x, p.second->y + y, p.second->color);
	}
	graph.selectAll(false);
	for(pair<Vertex*, Vertex*> p : g) {
		for(pair<Vertex*, bool> p2 : p.second->adjout) {
			graph.addArrow(vec[p.second], vec[p2.first]);
		}
		graph.select(vec[p.second], true);
	}
	return vec;
}

void setMode(int m) {
	if(mode != m) {
		mode = m;
		connecting = false;
		dragging = false;
		boxSelect = false;
		edgeSelect = false;
		removing = false;
	}
}
void setArrowMode(int m) {
	if(arrowMode != m) {
		arrowMode = m;
	}
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLineWidth(lineWidth);

	float mx = mouseX;
	float my = mouseY;
	if(showGrid) {
		glBegin(GL_LINES);
		glColor3f(0.75, 0.75, 0.75);
		float sz = gridSpace / zoom;
		while(sz < gridSpace) {
			sz *= 2;
		}
		while(sz > 2 * gridSpace) {
			sz /= 2;
		}
		float tx = translateX / zoom - sz * floor(translateX / sz);
		float ty = translateY / zoom - sz * floor(translateY / sz);
		mx = snapX(mouseX, tx, sz);
		my = snapY(mouseY, ty, sz);
		for(int i = -1 + tx / sz; -tx + (i - 1) * sz < width; i++) {
			glVertex2f(-tx + i * sz, 0);
			glVertex2f(-tx + i * sz, height);
		}
		for(int i = -1 + ty / sz; -ty + (i - 1) * sz < height; i++) {
			glVertex2f(0, -ty + i * sz);
			glVertex2f(width, -ty + i * sz);
		}
		glEnd();
	}

	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(translateX, translateX + width * zoom, translateY + height * zoom, translateY);
	//gluOrtho2D(0, width, height, 0);
	Vertex *vMouse = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);

	glBegin(GL_LINES);
	for(Vertex *v : graph.vertices) {
		for(pair<Vertex*, bool> p : v->adjout) {
			if(removing && intersecting(v->x, v->y, p.first->x, p.first->y, removeMouseX, removeMouseY, mouseX, mouseY)) {
				glColor3f(removeCol[0], removeCol[1], removeCol[2]);
			}else if(p.second || (edgeSelect && intersecting(v->x, v->y, p.first->x, p.first->y, edgeSelectMouseX, edgeSelectMouseY, mouseX, mouseY))) {
				glColor3f(edgeSelectCol[0], edgeSelectCol[1], edgeSelectCol[2]);
			}else if(v->selected && p.first->selected) {
				glColor3f(selectCol[0], selectCol[1], selectCol[2]);
			}else {
				glColor3f(outlineCol[0], outlineCol[1], outlineCol[2]);
			}
			drawArrow(v->x, v->y, p.first->x, p.first->y, graph.adjacent(p.first, v) ? ARROW_UNDIRECTED : ARROW_FORWARD);
		}
	}
	if(connecting) {
		if(vMouse == NULL) {
			glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
			drawArrow(connectVert->x, connectVert->y, mx, my, arrowMode);
		}else if(connectVert != vMouse) {
			glColor3f(createCol[0], createCol[1], createCol[2]);
			drawArrow(connectVert->x, connectVert->y, vMouse->x, vMouse->y, arrowMode);
		}
	}
	if(removing) {
		glColor3f(removeCol[0], removeCol[1], removeCol[2]);
		glVertex2f(removeMouseX, removeMouseY);
		glVertex2f(mouseX, mouseY);
	}
	if(edgeSelect) {
		glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		glVertex2f(edgeSelectMouseX, edgeSelectMouseY);
		glVertex2f(mouseX, mouseY);
	}
	if(mode == MODE_EXTEND) {
		glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		if(vMouse == NULL) {
			for(Vertex *w : graph.selected) {
				drawArrow(w->x, w->y, mx, my, arrowMode);
			}
		}else {
			for(Vertex *w : graph.selected) {
				if(w != vMouse) {
					drawArrow(w->x, w->y, vMouse->x, vMouse->y, arrowMode);
				}
			}
		}
	}else if(mode == MODE_EXTRUDE) {
		float extrudeX = 0;
		float extrudeY = 0;
		unordered_map<Vertex*, Vertex*> mycopy = copySelected(extrudeX, extrudeY);
		//vector<Vertex*> selectedCopy = pasteSubgraph(mycopy, mouseX - extrudeX, mouseY - extrudeY);

		glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		for(Vertex *v : graph.selected) {
			drawArrow(v->x, v->y,
					mycopy[v]->x + mx - extrudeX, mycopy[v]->y + my - extrudeY, arrowMode);
			//graph.setConnected(selectedClone[i], selectedCopy[i], true);
			for(pair<Vertex*, bool> p : mycopy[v]->adjout) {
				drawArrow(mycopy[v]->x + mx - extrudeX, mycopy[v]->y + my - extrudeY,
						p.first->x + mx - extrudeX, p.first->y + my - extrudeY,
						graph.arrowType(mycopy[v], p.first));
			}
		}
		glEnd();
		for(pair<Vertex*, Vertex*> p : mycopy) {
			drawCircle(p.second->x + mx - extrudeX, p.second->y + my - extrudeY, vertRadius * zoom, vertPrecision);
		}
		glBegin(GL_LINES);
		for(pair<Vertex*, Vertex*> p : mycopy) {
			delete p.second;
		}
	}
	glEnd();

	for(Vertex *v : graph.vertices) {
		glColor3f(col[v->color][0], col[v->color][1], col[v->color][2]);
		fillCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
		if(v->selected) {
			glColor3f(selectCol[0], selectCol[1], selectCol[2]);
		}else if(v == vMouse) {
			glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		}else {
			glColor3f(outlineCol[0], outlineCol[1], outlineCol[2]);
		}
		drawCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
	}

	if(boxSelect) {
		glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		drawRect(boxMouseX, boxMouseY, mouseX, mouseY);
	}
	if((mode == MODE_CREATE || mode == MODE_EXTEND) && vMouse == NULL && !removing) {
		glColor3f(highlightCol[0], highlightCol[1], highlightCol[2]);
		drawCircle(mx, my, vertRadius * zoom, vertPrecision);
	}

	glPopMatrix();

	font.SetColor(textCol[0], textCol[1], textCol[2]);
	font.ezPrint(modeText[mode].c_str(), 0, 0);
	font.SetColor(textCol[0], textCol[1], textCol[2]);
	font.ezPrint(arrowText[arrowMode].c_str(), 0, 30);

	glFlush();
	glutSwapBuffers();
}

void mouse_press(int button, int state, int x, int y) {
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
	float mx = mouseX;
	float my = mouseY;
	if(showGrid) {
		mx = snapX(mouseX);
		my = snapY(mouseY);
	}
	if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		if(mode == MODE_SELECT) {
			// select vertex
			Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
			if(v != NULL) {
				bool doSelect = true;
				if(glutGetModifiers() & GLUT_ACTIVE_ALT) {
					doSelect = !v->selected;
				}else if(!(glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
					graph.selectAll(false);
				}
				graph.select(v, doSelect);
				if(glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
					vector<Vertex *> s = graph.getComponent(v);
					for(Vertex *v2 : s) {
						graph.select(v2, doSelect);
					}
				}
				dragging = doSelect;
			}else {
				// box select
				if(!(glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
					graph.selectAll(false);
				}
				boxMouseX = mouseX;
				boxMouseY = mouseY;
				boxSelect = true;
			}
		}else if(mode == MODE_CREATE) {
			connectVert = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
			if(connectVert == NULL) {
				connectVert = graph.insert(mx, my);
			}
			connecting = true;
			removing = false;
		}else if(mode == MODE_EXTEND) {
			Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
			if(v == NULL) {
				v = graph.insert(mx, my);
			}
			for(Vertex *w : graph.selected) {
				graph.setConnected(w, v, arrowMode);
			}
			graph.selectAll(false);
			graph.select(v, true);
		}else if(mode == MODE_EXTRUDE) {
			unordered_set<Vertex*> selectedClone = graph.selected;
			float extrudeX = 0;
			float extrudeY = 0;
			unordered_map<Vertex*, Vertex*> mycopy = copySelected(extrudeX, extrudeY);
			unordered_map<Vertex*, Vertex*> selectedCopy = pasteSubgraph(mycopy, mx - extrudeX, my - extrudeY);

			for(Vertex *v : selectedClone) {
				graph.setConnected(v, selectedCopy[mycopy[v]], arrowMode);
			}
			for(pair<Vertex*, Vertex*> p : mycopy) {
				delete p.second;
			}
		}else if(mode == MODE_EDGESELECT) {
			if(!(glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
				for(Vertex *v : graph.vertices) {
					for(pair<Vertex*, bool> p : v->adjout) {
						graph.selectEdge(v, p.first, false);
					}
				}
			}
			edgeSelectMouseX = mouseX;
			edgeSelectMouseY = mouseY;
			edgeSelect = true;
		}
	}else if(state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		if(mode == MODE_SELECT) {
			if(boxSelect) {
				float minx = min(boxMouseX, mouseX);
				float miny = min(boxMouseY, mouseY);
				float maxx = max(boxMouseX, mouseX);
				float maxy = max(boxMouseY, mouseY);
				vector<Vertex*> verts = graph.getVertices(minx, miny, maxx, maxy);
				for(Vertex *v : verts) {
					graph.select(v, true);
				}
				boxSelect = false;
			}
			dragging = false;
		}else if(mode == MODE_CREATE) {
			if(connecting) {
				Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
				if(v == NULL) {
					v = graph.insert(mx, my);
				}
				graph.setConnected(connectVert, v, arrowMode);
				connecting = false;
			}
		}else if(mode == MODE_EDGESELECT && edgeSelect) {
			for(Vertex *v : graph.vertices) {
				for(pair<Vertex*, bool> p : v->adjout) {
					if(intersecting(v->x, v->y, p.first->x, p.first->y, edgeSelectMouseX, edgeSelectMouseY, mouseX, mouseY)) {
						graph.selectEdge(v, p.first, true);
					}
				}
			}
			edgeSelect = false;
		}
	}else if(button == GLUT_MIDDLE_BUTTON) {
		translating = state == GLUT_DOWN;
	}else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if(mode == MODE_CREATE) {
			removeMouseX = mouseX;
			removeMouseY = mouseY;
			removing = true;
			connecting = false;
		}
	}else if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		if(mode == MODE_CREATE && removing) {
			for(Vertex *v : graph.vertices) {
				vector<Vertex*> vec;
				for(pair<Vertex*, bool> p : v->adjout) {
					if(intersecting(v->x, v->y, p.first->x, p.first->y, removeMouseX, removeMouseY, mouseX, mouseY)) {
						vec.push_back(p.first);
					}
				}
				for(Vertex *w : vec) {
					graph.setDisconnected(v, w);
				}
			}
			removing = false;
		}
	}

	if((button == 3 || button == 4) && state == GLUT_DOWN) {
		// mouse wheel event
		if(button == 3) {
			zoom /= zoomAmt;
			translateX = mouseX + (translateX - mouseX) / zoomAmt;
			translateY = mouseY + (translateY - mouseY) / zoomAmt;
		}else {
			zoom *= zoomAmt;
			translateX = mouseX + (translateX - mouseX) * zoomAmt;
			translateY = mouseY + (translateY - mouseY) * zoomAmt;
		}
		mouseX = x * zoom + translateX;
		mouseY = y * zoom + translateY;
	}
	updateDisplay = true;
}
void mouse_move(int x, int y) {
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
	updateDisplay = true;
}
void mouse_drag(int x, int y) {
	float lastMouseX = mouseX;
	float lastMouseY = mouseY;
	float lastMx = mouseX;
	float lastMy = mouseY;
	if(showGrid) {
		lastMx = snapX(lastMouseX);
		lastMy = snapY(lastMouseY);
	}
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
	float mx = mouseX;
	float my = mouseY;
	if(showGrid) {
		mx = snapX(mouseX);
		my = snapY(mouseY);
	}
	if(dragging) {
		for(Vertex *v : graph.selected) {
			v->x += mx - lastMx;
			v->y += my - lastMy;
		}
	}
	if(translating) {
		translateX += (lastMouseX - mouseX);
		translateY += (lastMouseY - mouseY);
		mouseX = x * zoom + translateX;
		mouseY = y * zoom + translateY;
	}
	updateDisplay = true;
}

void key_press(unsigned char key, int x, int y) {
	if(key == 'v') {
		setMode(MODE_SELECT);
	}else if(key == 'c') {
		setMode(MODE_CREATE);
	}else if(key == 'e') {
		setMode(MODE_EXTEND);
	}else if(key == 'x') {
		setMode(MODE_EXTRUDE);
	}else if(key == 's') {
		setMode(MODE_EDGESELECT);
	}else if(key == 'u') {
		setArrowMode(ARROW_UNDIRECTED);
	}else if(key == 'f') {
		setArrowMode(ARROW_FORWARD);
	}else if(key == 'b') {
		setArrowMode(ARROW_BACKWARD);
	}else if(key == 'g') {
		showGrid = !showGrid;
	}else if(key >= '0' && key <= '9') {
		for(Vertex *v : graph.selected) {
			v->color = (key - '0');
		}
	}else if(key == 'd') {
		while(!graph.selected.empty()) {
			graph.remove(*graph.selected.begin());
		}
	}else if(key == 'm') {
		graph.mergeSelected();
	}else if(key == 'n') {
		normalizing = true;
		graph.normalize(0.0001);
		updateDisplay = true;
	}else if(key == 1 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + A
		if(graph.vertices.size() == graph.selected.size()) {
			graph.selectAll(false);
		}else {
			graph.selectAll(true);
		}
	}else if(key == 3 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + C
		for(pair<Vertex*, Vertex*> p : graphCopy) {
			delete p.second;
		}
		graphCopy.clear();
		graphCopy = copySelected(copyX, copyY);
	}else if(key == 22 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + V
		float mx = mouseX;
		float my = mouseY;
		if(showGrid) {
			mx = snapX(mx);
			my = snapY(my);
		}
		pasteSubgraph(graphCopy, mx - copyX, my - copyY);
	}
	updateDisplay = true;
}
void key_release(unsigned char key, int x, int y) {
	if(key == 'n') {
		normalizing = false;
	}
}

void resize(int w, int h) {
	width = w;
	height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, height, 0.0);
	glViewport(0,0,width,height);
}

long long start = 0;
void idle() {
	long long end = std::chrono::system_clock::now().time_since_epoch().count();
	if(end - start > 1000 / FPS) {
		if(updateDisplay) {
			display();
		}
		start = end;
		updateDisplay = false;
	}
}

int main(int argc, char **argv) {
	// window initialization
	glutInit(&argc, argv);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(30, 30);

	// create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("Graphs");

	init();
	glutMouseFunc(mouse_press);
	glutPassiveMotionFunc(mouse_move);
	glutMotionFunc(mouse_drag);
	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}
