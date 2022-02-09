#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

float frustumScale = 1.0f;
//float loadingTime;
static const int NUMBER_FISHES = 30;

GLuint programColor;
GLuint programTexture;
GLuint mainTexture;

Core::Shader_Loader shaderLoader;

obj::Model submarineModel;
obj::Model backgroundModel;
obj::Model fishModel;
obj::Model rock;
obj::Model coral;
obj::Model sharkModel;


glm::vec3 fishPosition[NUMBER_FISHES];
float rot_tab[NUMBER_FISHES];

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir; // camera forward vector
glm::vec3 cameraSide; // camera up vector

float mouseX;
int mouseY;
float changeX;
float changeY;
float prevY;
float prevX;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

GLuint textureSubmarine;
GLuint textureBackground;
GLuint textureFish;
GLuint textureCoral;
GLuint textureRock;
GLuint textureShark;

glm::quat rotation = glm::quat(1, 0, 0, 0);

void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.1f;
	float moveSpeed = 0.2f;
	glm::quat z;
	switch (key)
	{
	case 'z':
		z = glm::angleAxis(glm::radians(angleSpeed + 0.5f), glm::vec3(0, 0, 1.0));
		rotation = normalize(z * rotation);
		break;
	case 'x':
		z = glm::angleAxis(glm::radians(-1 * (angleSpeed + 0.5f)), glm::vec3(0, 0, 1.0));
		rotation = normalize(z * rotation);
		break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}


void mouse(int x, int y)
{
	changeX = x - prevX;
	changeY = y - prevY;
	prevX = x;
	prevY = y;
}

glm::mat4 createCameraMatrix()
{
	float k = 0.01;
	glm::quat rotationChangeX = glm::quat(glm::angleAxis(changeX * k, glm::vec3(0, 1, 0)));
	glm::quat rotationChangeY = glm::quat(glm::angleAxis(changeY * k, glm::vec3(1, 0, 0)));
	changeX = 0;
	changeY = 0;
	glm::quat rotationChange = glm::normalize(rotationChangeX * rotationChangeY);
	rotation = rotationChange * rotation;
	rotation = glm::normalize(rotation);
	cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);


	return Core::createViewMatrixQuat(cameraPos, rotation);

}


void drawObjectColor(obj::Model* model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void drawObjectTextureMain(obj::Model* model, glm::mat4 modelMatrix, GLuint textureID)
{
	GLuint program = mainTexture;

	// Aktywowanie shadera
	glUseProgram(program);
	//Obliczanie oœwietlenia obiektu, natê¿enia œwiat³a za pomoc¹ danego shadera
	//Odwo³anie do zmiennej "sampler2dtype" w shader_tex1.frag
	//Ustawianie zmiennej sampler2D na wczytan¹ wczeœniej teksturê przekazan¹ jako parametr
	Core::SetActiveTexture(textureID, "sampler2dtype", 1, 0);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	// "transformation" jest automatycznie inicjalizowane macierza jednostkowa 4 x 4
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;

	// Polecenie glUniformMatrix4fv wysyla zmienna "transformation" do GPU i przypisuje ja do zmiennej typu mat4 o nazwie "modelViewProjectionMatrix" w shaderze.
	// Shader uzyje tej macierzy do transformacji wierzcholkow podczas rysowania.
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	// Wylaczenie shadera
	glUseProgram(0);
}

void drawObjectTexture(obj::Model* model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	Core::SetActiveTexture(textureId, "textureSampler", 1, 0);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	//Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void renderScene()
{
	//Tworzenie macierzy perspektywy za pomoca createPerspectiveMatrix(), uzyskujemy obraz 3D
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 1000, frustumScale);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Zmienna "time"  zawiera liczbe sekund od uruchomienia programu
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; //-loadingTime;


	glm::mat4 attachShip = glm::mat4_cast(inverse(rotation));

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.25f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, -1)) * glm::scale(glm::vec3(0.000115f));
	glm::mat4 submarineModelMatrix = glm::translate(cameraPos + cameraDir * 1.5f) * attachShip * shipInitialTransformation;
	submarineModelMatrix *= glm::rotate(sin(cameraPos[0] + cameraPos[1] + cameraPos[2] + cameraAngle) / 8, glm::vec3(0, 1, 0));
	drawObjectTexture(&submarineModel, submarineModelMatrix, textureSubmarine);
	

	glm::mat4 rotationFishMatrix[NUMBER_FISHES];
	// dla ruchu po okregach
	glm::mat4 rotation_round[NUMBER_FISHES];
	// dla ruchu lewo-prawo
	glm::mat4 rotation;

	// dla ruchu po okregach
	for (int i = 0; i < NUMBER_FISHES; i++) {
		rotation_round[i][0][2] = cos(rot_tab[i] * time);
		rotation_round[i][2][0] = sin(rot_tab[i] * time);
		rotation_round[i][0][2] = -sin(rot_tab[i] * time);
		rotation_round[i][2][2] = cos(rot_tab[i] * time);
	}

	//dla ruchu lewo-prawo
	rotation[0][0] = cos(time);
	rotation[2][0] = sin(time);
	rotation[0][2] = -sin(time);
	rotation[2][2] = cos(time);

	for (int i = 0; i < NUMBER_FISHES; i++) {
		
		if (i % 3 == 0) {
			drawObjectTexture(&fishModel, glm::translate(fishPosition[i]) * glm::translate(glm::vec3(sin(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.2f)), textureFish);
		}

		else if(i % 3 == 1) {
			drawObjectTexture(&fishModel, glm::translate(fishPosition[i]) * glm::translate(glm::vec3(-cos(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.2f)), textureFish);
		}

		else {
			rotationFishMatrix[i][3][0] = (10 + i) * sin(rot_tab[i] * time);
			rotationFishMatrix[i][3][2] = (10 + i) * cos(rot_tab[i] * time);
			drawObjectTexture(&fishModel, glm::translate(fishPosition[i]) * rotationFishMatrix[i] * rotation_round[i] * glm::scale(glm::vec3(0.2f)), textureFish);
		}
	}


	drawObjectTexture(&sharkModel, glm::translate(glm::vec3(3, -2, 2)) * glm::scale(glm::vec3(0.005f)), textureShark);

	drawObjectTextureMain(&backgroundModel, glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(100.0f)), textureBackground);

	
	glutSwapBuffers();
}


void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	mainTexture = shaderLoader.CreateProgram("shaders/shader_tex1.vert", "shaders/shader_tex1.frag");

	submarineModel = obj::loadModelFromFile("models/submarine.obj");
	backgroundModel = obj::loadModelFromFile("models/backg.obj");
	fishModel = obj::loadModelFromFile("models/fish.obj");
	sharkModel = obj::loadModelFromFile("models/shark.obj");

	textureSubmarine = Core::LoadTexture("textures/s.png");
	textureBackground = Core::LoadTexture("textures/water.png");
	textureFish = Core::LoadTexture("textures/fish.png");
	textureShark = Core::LoadTexture("textures/shark.png");


	for (int i = 0; i < NUMBER_FISHES; i++) {
		fishPosition[i] = glm::sphericalRand(30.f);
		//fishPosition[i] = glm::vec3(rand() % 20 - 10, rand() % 10 - 5, rand() % 20 - 10);
		//rot_tab[i] = ((float)rand() / (float)(RAND_MAX)) * 2.0;
		rot_tab[i] = 0.05 + (rand() / (RAND_MAX / (1.0 - 0.05)));
	}


	//loadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown()
{
	//Usuwanie za³adowanych shaderów z pamiêci
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(mainTexture);
}

void idle()
{
	glutPostRedisplay();
}


void onReshape(int width, int height)
{
	// Elminacja niezkszta³cenia obrazu przy zmianie wielkosci okna
	frustumScale = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
	// Stworzenie okna przy u¿yciu biblioteki freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Projekt");
	// Inicjalizacja rozszerzen OpenGL
	glewInit();

	// Inicjalizacja programu (ladowanie shaderow, tekstur itp)
	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	// Poinformowanie freegluta jaka funkcja bedzie sluzyc do odswiezania obrazu
	glutDisplayFunc(renderScene);
	// Poinformowanie freegluta jaka funkcja bedzie wywolywana w glownej petli programu
	glutIdleFunc(idle);
	glutReshapeFunc(onReshape);

	// Uruchomienie glownej petli
	glutMainLoop();

	// Sprzatanie (usuwanie shaderow itp)
	shutdown();

	return 0;
}
