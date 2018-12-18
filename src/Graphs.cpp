
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <Graph.h>
#include <set>

using namespace std;

int width = 800;
int height = 600;

Graph graph;

float vertRadius = 16;
float lineWidth = 3;
int vertPrecision = 30;

bool vPress = false;
float mouseX;
float mouseY;
bool dragging = false;

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

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	for(Vertex *v : graph.vertices) {
		for(Vertex *w : v->adj) {
			glVertex2f(v->x, v->y);
			glVertex2f(w->x, w->y);
		}
	}
	glEnd();

	for(Vertex *v : graph.vertices) {
		if(v->selected) {
			glColor3f(0, 0, 1);
		}else {
			glColor3f(1, 0, 0);
		}
		fillCircle(v->x, v->y, vertRadius, vertPrecision);
		glColor3f(1, 1, 1);
		drawCircle(v->x, v->y, vertRadius, vertPrecision);
	}

	if(vPress) {
		glColor3f(1, 1, 0);
		drawCircle(mouseX, mouseY, vertRadius, vertPrecision);
	}

	glFlush();
	glutSwapBuffers();
}

void mouse_press(int button, int state, int x, int y) {
	mouseX = x;
	mouseY = y;
	if(state == GLUT_DOWN) {
		if(button == GLUT_LEFT_BUTTON) {
			if(vPress) {
				// create vertex
				Vertex *v = graph.insert(x, y);
				graph.selectAll(false);
				graph.select(v, true);
				dragging = true;
			}else {
				// select vertex
				Vertex *v = graph.getVertex(x, y, vertRadius);
				graph.selectAll(false);
				if(v != NULL) {
					graph.select(v, true);
					dragging = true;
				}
			}
		}else if(button == GLUT_RIGHT_BUTTON) {
			Vertex *v = graph.getVertex(x, y, vertRadius);
			if(v != NULL) {
				graph.remove(v);
			}
		}
	}else if(state == GLUT_UP) {
		dragging = false;
	}
}
void mouse_move(int x, int y) {
	mouseX = x;
	mouseY = y;
	display();
}
void mouse_drag(int x, int y) {
	if(dragging) {
		for(Vertex *v : graph.selected) {
			v->x += x - mouseX;
			v->y += y - mouseY;
		}
	}
	mouseX = x;
	mouseY = y;
	display();
}

void key_press(unsigned char key, int x, int y) {
	if(key == 'v') {
		vPress = true;
		display();
	}
}
void key_release(unsigned char key, int x, int y) {
	if(key == 'v') {
		vPress = false;
		display();
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
