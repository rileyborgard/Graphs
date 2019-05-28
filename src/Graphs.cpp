
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <Graph.h>
#include <set>
#include <cstdlib>
#include <string>
#include <BitmapFontClass.h>

#define MODE_SELECT 0
#define MODE_CREATE 1
#define MODE_EXTEND 2
#define MODE_EXTRUDE 3

#define ARROW_UNDIRECTED 0
#define ARROW_FORWARD 1
#define ARROW_BACKWARD 2

using namespace std;

int width = 800;
int height = 600;

Graph graph;

float vertRadius = 16;
float vertLockRadius = 32;
float arrowLength = 16;
float lineWidth = 3;
int vertPrecision = 30;
float zoomAmt = 1.05;
int mode = MODE_SELECT;
int arrowMode = ARROW_UNDIRECTED;
string modeText[] = {"MODE: select", "MODE: create", "MODE: extend", "MODE: extrude"};
string arrowText[] = {"ARROWS: undirected", "ARROWS: forward", "ARROWS: backward"};

float translateX = 0;
float translateY = 0;
float zoom = 1;

float mouseX;
float mouseY;
float boxMouseX;
float boxMouseY;
float removeMouseX;
float removeMouseY;
bool boxSelect = false;
bool dragging = false;
bool connecting = false;
bool translating = false;
bool removing = false;

vector<Vertex*> graphCopy;
float copyX;
float copyY;

CBitmapFont font;

Vertex *connectVert;

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
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

vector<Vertex*> copySelected(float &copyX, float &copyY) {
	vector<Vertex*> g;

	copyX = 0;
	copyY = 0;
	for(unsigned int i = 0; i < graph.selected.size(); i++) {
		Vertex *v = graph.selected[i];
		Vertex *w = new Vertex;
		w->x = v->x;
		w->y = v->y;
		w->adjout = vector<Vertex*>();
		w->adjin = vector<Vertex*>();
		w->selected = false;
		w->index = i;
		g.push_back(w);
		copyX += v->x;
		copyY += v->y;
	}
	if(graph.selected.size() > 0) {
		copyX /= graph.selected.size();
		copyY /= graph.selected.size();
	}
	for(unsigned int i = 0; i < graph.selected.size(); i++) {
		Vertex *v = graph.selected[i];
		for(unsigned int j = i + 1; j < graph.selected.size(); j++) {
			Vertex *w = graph.selected[j];
			if(graph.adjacent(v, w)) {
				g[i]->adjout.push_back(g[j]);
				g[j]->adjin.push_back(g[i]);
			}
			if(graph.adjacent(w, v)) {
				g[j]->adjout.push_back(g[i]);
				g[i]->adjin.push_back(g[j]);
			}
		}
	}
	return g;
}
vector<Vertex*> pasteSubgraph(vector<Vertex*> g, float x, float y) {
	vector<Vertex*> vec;
	for(unsigned int i = 0; i < g.size(); i++) {
		vec.push_back(graph.insert(g[i]->x + x, g[i]->y + y));
	}
	graph.selectAll(false);
	for(unsigned int i = 0; i < g.size(); i++) {
		for(Vertex *v : g[i]->adjout) {
			graph.addArrow(vec[i], vec[v->index]);
		}
		graph.select(vec[i], true);
	}
	return vec;
}

void setMode(int m) {
	if(mode != m) {
		mode = m;
		connecting = false;
		dragging = false;
		boxSelect = false;
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

	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(translateX, translateX + width * zoom, translateY + height * zoom, translateY);
	//gluOrtho2D(0, width, height, 0);
	Vertex *vMouse = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);

	glBegin(GL_LINES);
	for(Vertex *v : graph.vertices) {
		for(Vertex *w : v->adjout) {
			if(removing && intersecting(v->x, v->y, w->x, w->y, removeMouseX, removeMouseY, mouseX, mouseY)) {
				glColor3f(1, 0, 0);
			}else if(v->selected && w->selected) {
				glColor3f(0, 0.5, 1);
			}else {
				glColor3f(1, 1, 1);
			}
			drawArrow(v->x, v->y, w->x, w->y, graph.adjacent(w, v) ? ARROW_UNDIRECTED : ARROW_FORWARD);
		}
	}
	if(connecting) {
		if(vMouse == NULL) {
			glColor3f(1, 1, 0);
			drawArrow(connectVert->x, connectVert->y, mouseX, mouseY, arrowMode);
		}else if(connectVert != vMouse) {
			glColor3f(0, 1, 0);
			drawArrow(connectVert->x, connectVert->y, vMouse->x, vMouse->y, arrowMode);
		}
	}
	if(removing) {
		glColor3f(1, 0, 0);
		glVertex2f(removeMouseX, removeMouseY);
		glVertex2f(mouseX, mouseY);
	}
	if(mode == MODE_EXTEND) {
		glColor3f(1, 1, 0);
		if(vMouse == NULL) {
			for(Vertex *w : graph.selected) {
				drawArrow(w->x, w->y, mouseX, mouseY, arrowMode);
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
		vector<Vertex*> mycopy = copySelected(extrudeX, extrudeY);
		//vector<Vertex*> selectedCopy = pasteSubgraph(mycopy, mouseX - extrudeX, mouseY - extrudeY);

		glColor3f(1, 1, 0);
		for(unsigned int i = 0; i < mycopy.size(); i++) {
			drawArrow(graph.selected[i]->x, graph.selected[i]->y,
					mycopy[i]->x + mouseX - extrudeX, mycopy[i]->y + mouseY - extrudeY, arrowMode);
			//graph.setConnected(selectedClone[i], selectedCopy[i], true);
			for(unsigned int j = 0; j < mycopy[i]->adjout.size(); j++) {
				drawArrow(mycopy[i]->x + mouseX - extrudeX, mycopy[i]->y + mouseY - extrudeY,
						mycopy[i]->adjout[j]->x + mouseX - extrudeX, mycopy[i]->adjout[j]->y + mouseY - extrudeY,
						graph.arrowType(mycopy[i], mycopy[i]->adjout[j]));
			}
		}
		glEnd();
		for(Vertex *v : mycopy) {
			drawCircle(v->x + mouseX - extrudeX, v->y + mouseY - extrudeY, vertRadius * zoom, vertPrecision);
		}
		glBegin(GL_LINES);
		for(Vertex *v : mycopy) {
			delete v;
		}
	}
	glEnd();

	for(Vertex *v : graph.vertices) {
		if(v->selected) {
			glColor3f(0, 0.5, 1);
		}else {
			glColor3f(1, 0, 0);
		}
		fillCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
		if(v == vMouse) {
			glColor3f(1, 1, 0);
		}else {
			glColor3f(1, 1, 1);
		}
		drawCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
	}

	if(boxSelect) {
		glColor3f(1, 1, 0);
		drawRect(boxMouseX, boxMouseY, mouseX, mouseY);
	}
	if((mode == MODE_CREATE || mode == MODE_EXTEND) && vMouse == NULL && !removing) {
		glColor3f(1, 1, 0);
		drawCircle(mouseX, mouseY, vertRadius * zoom, vertPrecision);
	}

	glPopMatrix();

	font.SetColor(0, 0.75, 0.75);
	font.ezPrint(modeText[mode].c_str(), 0, 0);
	font.SetColor(0, 1, 0);
	font.ezPrint(arrowText[arrowMode].c_str(), 0, 30);

	glFlush();
	glutSwapBuffers();
}

void mouse_press(int button, int state, int x, int y) {
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
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
				connectVert = graph.insert(mouseX, mouseY);
			}
			connecting = true;
			removing = false;
		}else if(mode == MODE_EXTEND) {
			Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
			if(v == NULL) {
				v = graph.insert(mouseX, mouseY);
			}
			for(Vertex *w : graph.selected) {
				graph.setConnected(w, v, arrowMode);
			}
			graph.selectAll(false);
			graph.select(v, true);
		}else if(mode == MODE_EXTRUDE) {
			vector<Vertex*> selectedClone = graph.selected;
			float extrudeX = 0;
			float extrudeY = 0;
			vector<Vertex*> mycopy = copySelected(extrudeX, extrudeY);
			vector<Vertex*> selectedCopy = pasteSubgraph(mycopy, mouseX - extrudeX, mouseY - extrudeY);

			for(unsigned int i = 0; i < selectedCopy.size(); i++) {
				graph.setConnected(selectedClone[i], selectedCopy[i], arrowMode);
			}
			for(Vertex *v : mycopy) {
				delete v;
			}
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
					v = graph.insert(mouseX, mouseY);
				}
				graph.setConnected(connectVert, v, arrowMode);
				connecting = false;
			}
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
				for(Vertex *w : v->adjout) {
					if(intersecting(v->x, v->y, w->x, w->y, removeMouseX, removeMouseY, mouseX, mouseY)) {
						graph.setDisconnected(v, w);
					}
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
	display();
}
void mouse_move(int x, int y) {
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
	display();
}
void mouse_drag(int x, int y) {
	float lastMouseX = mouseX;
	float lastMouseY = mouseY;
	mouseX = x * zoom + translateX;
	mouseY = y * zoom + translateY;
	if(dragging) {
		for(Vertex *v : graph.selected) {
			v->x += mouseX - lastMouseX;
			v->y += mouseY - lastMouseY;
		}
	}
	if(translating) {
		translateX += (lastMouseX - mouseX);
		translateY += (lastMouseY - mouseY);
		mouseX = x * zoom + translateX;
		mouseY = y * zoom + translateY;
	}
	display();
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
	}else if(key == 'u') {
		setArrowMode(ARROW_UNDIRECTED);
	}else if(key == 'f') {
		setArrowMode(ARROW_FORWARD);
	}else if(key == 'b') {
		setArrowMode(ARROW_BACKWARD);
	}else if(key == 'd') {
		while(!graph.selected.empty()) {
			graph.remove(graph.selected[0]);
		}
	}else if(key == 'm') {
		graph.mergeSelected();
	}else if(key == 1 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + A
		if(graph.vertices.size() == graph.selected.size()) {
			graph.selectAll(false);
		}else {
			graph.selectAll(true);
		}
	}else if(key == 3 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + C
		for(Vertex *v : graphCopy) {
			delete v;
		}
		graphCopy.clear();
		graphCopy = copySelected(copyX, copyY);
	}else if(key == 22 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + V
		pasteSubgraph(graphCopy, mouseX - copyX, mouseY - copyY);
	}
	display();
}
void key_release(unsigned char key, int x, int y) {

}

void resize(int w, int h) {
	width = w;
	height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, height, 0.0);
	glViewport(0,0,width,height);
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

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}
