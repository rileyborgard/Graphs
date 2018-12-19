
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <Graph.h>
#include <set>
#include <cstdlib>

#define MODE_SELECT 0
#define MODE_CREATE 1

using namespace std;

int width = 800;
int height = 600;

Graph graph;

float vertRadius = 16;
float vertLockRadius = 32;
float lineWidth = 3;
int vertPrecision = 30;
float zoomAmt = 1.05;
int mode = MODE_SELECT;

float translateX = 0;
float translateY = 0;
float zoom = 1;

float mouseX;
float mouseY;
float boxMouseX;
float boxMouseY;
bool boxSelect = false;
bool dragging = false;
bool connecting = false;
bool translating = false;

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

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLineWidth(lineWidth);

	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(translateX, translateX + width * zoom, translateY + height * zoom, translateY);
	//gluOrtho2D(0, width, height, 0);

	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	for(Vertex *v : graph.vertices) {
		for(Vertex *w : v->adj) {
			glVertex2f(v->x, v->y);
			glVertex2f(w->x, w->y);
		}
	}
	if(connecting) {
		Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
		if(v == NULL) {
			glColor3f(1, 1, 0);
			glVertex2f(connectVert->x, connectVert->y);
			glVertex2f(mouseX, mouseY);
		}else {
			if(v->adj.find(connectVert) == v->adj.end()) {
				glColor3f(0, 1, 0);
			}else {
				glColor3f(1, 0, 0);
			}
			glVertex2f(connectVert->x, connectVert->y);
			glVertex2f(v->x, v->y);
		}
	}
	glEnd();

	for(Vertex *v : graph.vertices) {
		if(v->selected) {
			glColor3f(0, 0, 1);
		}else {
			glColor3f(1, 0, 0);
		}
		fillCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
		glColor3f(1, 1, 1);
		drawCircle(v->x, v->y, vertRadius * zoom, vertPrecision);
	}

	if(boxSelect) {
		glColor3f(1, 1, 0);
		drawRect(boxMouseX, boxMouseY, mouseX, mouseY);
	}

	glPopMatrix();

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
					set<Vertex *> s = graph.getComponent(v);
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
		}else if(button == GLUT_MIDDLE_BUTTON) {
			translating = true;
		}
	}else if(state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		if(mode == MODE_SELECT) {
			if(boxSelect) {
				float minx = min(boxMouseX, mouseX);
				float miny = min(boxMouseY, mouseY);
				float maxx = max(boxMouseX, mouseX);
				float maxy = max(boxMouseY, mouseY);
				set<Vertex*> verts = graph.getVertices(minx, miny, maxx, maxy);
				for(Vertex *v : verts) {
					graph.select(v, true);
				}
				boxSelect = false;
			}
			dragging = false;
		}else if(mode == MODE_CREATE) {
			if(connecting) {
				Vertex *v = graph.getVertex(mouseX, mouseY, vertLockRadius * zoom);
				if(v != NULL) {
					graph.setConnected(connectVert, v, connectVert->adj.find(v) == connectVert->adj.end());
				}else {
					v = graph.insert(mouseX, mouseY);
					graph.setConnected(connectVert, v, true);
				}
				connecting = false;
			}
		}else if(button == GLUT_MIDDLE_BUTTON) {
			translating = false;
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
		mode = MODE_SELECT;
	}else if(key == 'c') {
		mode = MODE_CREATE;
	}else if(key == 'd') {
		for(Vertex *v : graph.selected) {
			graph.remove(v);
		}
	}else if(key == 1 && (glutGetModifiers() & GLUT_ACTIVE_CTRL)) {
		// Ctrl + A
		if(graph.vertices.size() == graph.selected.size()) {
			graph.selectAll(false);
		}else {
			graph.selectAll(true);
		}
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
