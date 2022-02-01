#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;
void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ZADANIE: Przesledz kod i komentarze
	// ZADANIE: Zmien kolor tla sceny, przyjmujac zmiennoprzecinkowy standard RGBA
	glClearColor(0.5f, 0.3f, 0.6f, 1.0f);

	// Powinno byc wywolane po kazdej klatce
	glutSwapBuffers();
}

void init()
{

}

void shutdown()
{

}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	// Tworzenie okna przy uzyciu freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");

	// Inicjalizacja rozszerzen OpenGL
	glewInit();

	// Inicjalizacja programu (ladowanie shaderow, tekstur, etc.)
	init();

	// Funkcja, ktorej uzywamy do odswiezenia ekranu, przekazana jest jako argument do glutDisplayFunc
	glutDisplayFunc(renderScene);
	// Informujemy freeglut o tym, ktora funkcja ma byc wywolywana w glownej petli programu
	glutIdleFunc(idle);

	// startujemy glowna petle
	glutMainLoop();

	// Garbage collection
	shutdown();

	return 0;
}
