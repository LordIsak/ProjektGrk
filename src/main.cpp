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

static const int NUMBER_FISHES = 30;
static const int NUMBER_ROCKS = 4;

GLuint programColor;
GLuint programTexture;
GLuint mainProgramTextur;

Core::Shader_Loader shaderLoader;

obj::Model submarineModel;
obj::Model backgroundModel;
obj::Model fishModel, fishModel_2;
obj::Model rockModel, rockModel_2;
obj::Model coralModel;
obj::Model coralModel_2;
obj::Model coralModel_3;
obj::Model coralModel_4;
obj::Model turtleModel;
obj::Model turtleModel_2;
//obj::Model sharkModel;
obj::Model chestModel;
obj::Model skeletonModel;
obj::Model phormiumModel;
obj::Model pillarModel;
obj::Model crabModel;
obj::Model goldModel;


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
GLuint textureFish, textureFish_2, textureFish_3;
GLuint textureCoral;
GLuint textureRock;
GLuint textureTurtle;
GLuint textureTurtle_2;
//GLuint textureShark;
GLuint textureChest;
GLuint textureSkeleton;
GLuint textureCrab;
GLuint textureGold;
//GLuint texturePhormium;


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
	GLuint program = mainProgramTextur;

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
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 1000, frustumScale);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	// poczatkowa pozycja statku, przyczepienie do statku kamery, imitacja ruchu statku, rysowanie statku
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

	// rysowanie:
	// ryby
	for (int i = 0; i < NUMBER_FISHES; i++) {
		
		if (i % 3 == 0) {
			drawObjectTexture(&fishModel, glm::translate(fishPosition[i]) * glm::translate(glm::vec3(sin(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.2f)), textureFish);
		}

		else if(i % 3 == 1) {
			drawObjectTexture(&fishModel_2, glm::translate(fishPosition[i]) * glm::translate(glm::vec3(-cos(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.2f)), textureFish_2);
		}

		else {
			rotationFishMatrix[i][3][0] = (10 + i) * sin(rot_tab[i] * time);
			rotationFishMatrix[i][3][2] = (10 + i) * cos(rot_tab[i] * time);
			drawObjectTexture(&fishModel_2, glm::translate(fishPosition[i]) * rotationFishMatrix[i] * rotation_round[i] * glm::scale(glm::vec3(0.2f)), textureFish_3);
		}
	}


	// p³ywaj¹ce ¿ó³wie
	drawObjectTexture(&turtleModel, glm::translate(glm::vec3(3, 20.f, 2)) * glm::translate(glm::vec3(sin(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.03f)), textureTurtle);
	drawObjectTexture(&turtleModel, glm::translate(glm::vec3(10.f, -20.f, 2)) * glm::translate(glm::vec3(sin(time) * 20.f, sin(time) * 0.5, 0)) * rotation * glm::scale(glm::vec3(0.03f)), textureTurtle);
	
	// statyczny ¿ó³w
	drawObjectTexture(&turtleModel_2, glm::translate(glm::vec3(10.f, -97.f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.3f)), textureTurtle_2);
	//drawObjectTexture(&sharkModel, glm::translate(glm::vec3(10.f, -50.f, 2)) * glm::translate(glm::vec3(0, sin(time) * 0.5, cos(time) * 45.f)) * rotation
		// * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.09f)), textureShark);
	
	// korale
	drawObjectTexture(&coralModel, glm::translate(glm::vec3(3.f, -95.f, 28.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.1f)), textureCoral);
	drawObjectTexture(&coralModel_2, glm::translate(glm::vec3(20.f, -96.f, -10.f)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.9f)), textureCoral);
	drawObjectTexture(&coralModel_3, glm::translate(glm::vec3(5.f, -93.f, 35.f)) * glm::rotate(glm::radians(175.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.0f)), textureCoral);
	drawObjectTexture(&coralModel_2, glm::translate(glm::vec3(40.f, -90.f, 10.f)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.0f)), textureCoral);
	drawObjectTexture(&coralModel_4, glm::translate(glm::vec3(0.f, -90.f, -40.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.5f)), textureRock);
	// ska³y
	drawObjectTexture(&rockModel, glm::translate(glm::vec3(50.f, -80.f, 0.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.5f)), textureRock);
	drawObjectTexture(&rockModel_2, glm::translate(glm::vec3(-40.f, -85.f, 0.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.5f)), textureRock);
	drawObjectTexture(&rockModel, glm::translate(glm::vec3(30.f, -85.f, 20.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.5f)), textureRock);
	drawObjectTexture(&rockModel_2, glm::translate(glm::vec3(-40.f, -83.f, -35.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.5f)), textureRock);
	//drawObjectTexture(&phormiumModel, glm::translate(glm::vec3(25.f, -70.f, 0.f)) * glm::rotate(glm::radians(-110.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(0.5f)), textureCoral);
	drawObjectTexture(&pillarModel, glm::translate(glm::vec3(15.f, -98.f, -15.f)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, -1)) * glm::scale(glm::vec3(1.5f)), textureCoral);

	// skrzynia
	drawObjectTexture(&chestModel, glm::translate(glm::vec3(-35.f, -86.f, -35.f)) * glm::scale(glm::vec3(1.f)), textureChest);
	
	// szkielet
	drawObjectTexture(&skeletonModel, glm::translate(glm::vec3(-35.f, -88.f, -30.f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0.6f, 1)) * glm::scale(glm::vec3(5.f)), textureSkeleton);

	// krab
	drawObjectTexture(&crabModel, glm::translate(glm::vec3(35.f, -91.f, 15.f)) * glm::rotate(glm::radians(-90.0f), glm::vec3(1.f, 1.f, 1.f))  * glm::scale(glm::vec3(0.2f)), textureCrab);

	// z³oto
	drawObjectTexture(&goldModel, glm::translate(glm::vec3(-34.f, -88.f, -33.f)) * glm::scale(glm::vec3(0.8f)), textureGold);

	// Pseudo Skybox
	drawObjectTextureMain(&backgroundModel, glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(100.0f)), textureBackground);

	
	glutSwapBuffers();
}


void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	// wczytanie shaderów
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	mainProgramTextur = shaderLoader.CreateProgram("shaders/shader_main.vert", "shaders/shader_main.frag");
	
	// wczytanie modeli
	submarineModel = obj::loadModelFromFile("models/submarine.obj");
	backgroundModel = obj::loadModelFromFile("models/backg.obj");
	fishModel = obj::loadModelFromFile("models/fish.obj");
	fishModel_2 = obj::loadModelFromFile("models/fish2.obj");
	turtleModel = obj::loadModelFromFile("models/turtle.obj");
	turtleModel_2 = obj::loadModelFromFile("models/turtle2.obj");
	//sharkModel = obj::loadModelFromFile("models/shark.obj");
	coralModel = obj::loadModelFromFile("models/coral.obj");
	coralModel_2 = obj::loadModelFromFile("models/coral2.obj");
	coralModel_3 = obj::loadModelFromFile("models/coral3.obj");
	coralModel_4 = obj::loadModelFromFile("models/coral4.obj");
	rockModel = obj::loadModelFromFile("models/stone.obj");
	rockModel_2 = obj::loadModelFromFile("models/stone2.obj");
	chestModel = obj::loadModelFromFile("models/chest.obj");
	skeletonModel = obj::loadModelFromFile("models/skeleton.obj");
	phormiumModel = obj::loadModelFromFile("models/phormium.obj");
	pillarModel = obj::loadModelFromFile("models/pillar.obj");
	crabModel = obj::loadModelFromFile("models/crab.obj");
	goldModel = obj::loadModelFromFile("models/gold_bar.obj");

	// wczytanie tekstur
	textureSubmarine = Core::LoadTexture("textures/s2.png");
	textureBackground = Core::LoadTexture("textures/water.png");
	textureFish = Core::LoadTexture("textures/fish.png");
	textureFish_2 = Core::LoadTexture("textures/fish2.png");
	textureFish_3 = Core::LoadTexture("textures/fish3.png");
	textureTurtle = Core::LoadTexture("textures/turtle.png");
	textureTurtle_2 = Core::LoadTexture("textures/turtle2.png");
	//textureShark = Core::LoadTexture("textures/shark.png");
	textureCoral = Core::LoadTexture("textures/coral.png");
	textureRock = Core::LoadTexture("textures/stone.png");
	textureChest = Core::LoadTexture("textures/chest.png");
	textureSkeleton = Core::LoadTexture("textures/skeleton.png");
	textureCrab = Core::LoadTexture("textures/crab.png");
	textureGold = Core::LoadTexture("textures/gold_bar.png");
	//texturePhormium = Core::LoadTexture("textures/phormium.jpg");

	// Losowa pozycja dla ryb uzyskana za pomoc¹ sphericalRand
	for (int i = 0; i < NUMBER_FISHES; i++) {
		fishPosition[i] = glm::sphericalRand(30.f);
		//fishPosition[i] = glm::vec3(rand() % 20 - 10, rand() % 10 - 5, rand() % 20 - 10);
		//rot_tab[i] = ((float)rand() / (float)(RAND_MAX)) * 2.0;

		// Randomowe z pewnym zakresem zmienne, które pos³u¿¹ do wytycznia ruchu po okrêgu
		rot_tab[i] = 0.05 + (rand() / (RAND_MAX / (1.0 - 0.05)));
	}

}

void shutdown()
{
	//Usuwanie za³adowanych shaderów z pamiêci
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(mainProgramTextur);
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
