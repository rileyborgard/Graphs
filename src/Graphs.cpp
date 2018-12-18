
#include <iostream>
#include <GL/freeglut.h>
#include <cmath>
#include <Graph.h>
#include <set>

using namespace std;

int width = 800;
int height = 600;

Graph graph;

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glPointSize(4.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	Vertex *v1 = graph.insert(100, 100);
	Vertex *v2 = graph.insert(300, 200);
	Vertex *v3 = graph.insert(200, 100);

	graph.setConnected(v1, v2, true);
	graph.setConnected(v2, v3, true);
	graph.setConnected(v3, v1, false);
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

	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	for(Vertex *v : graph.vertices) {
		for(Vertex *w : v->adj) {
			glVertex2f(v->x, v->y);
			glVertex2f(w->x, w->y);
		}
	}
	glEnd();

	glColor3f(1, 0, 0);
	for(Vertex *v : graph.vertices) {
		fillCircle(v->x, v->y, 10, 10);
	}

	glFlush();
	glutSwapBuffers();
}

void mouse_press(int button, int state, int x, int y) {
	if(state == GLUT_DOWN) {
		if(button == GLUT_LEFT_BUTTON) {
			graph.insert(x, y);
		}else if(button == GLUT_RIGHT_BUTTON) {
			Vertex *v = graph.getVertex(x, y, 10);
			if(v != NULL) {
				graph.remove(v);
			}
		}
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
	glutReshapeFunc(resize);

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}
