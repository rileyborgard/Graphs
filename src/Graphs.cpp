
#include <iostream>
#include <GL/freeglut.h>

using namespace std;

const int width = 800;
const int height = 600;

void init() {
	// set clear color to 20% gray
	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glPointSize(4.0f);
	glMatrixMode(GL_PROJECTION);    //coordinate system
	//glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFlush();
	glutSwapBuffers();
}

void mouse_press(int button, int state, int x, int y) {

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

	// set empty display function
	glutDisplayFunc(display);

	// start glut loop
	glutMainLoop();

	return 0;
}
