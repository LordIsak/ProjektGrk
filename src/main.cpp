#include <filesystem>
namespace fs = std::filesystem;

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "stb_image.h"
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

float frustumScale = 1.0f;
float loadingTime;
static const int NUMBER_FISHES = 30;

GLuint programColor;
GLuint programTexture;
GLuint mainTexture;
GLuint skyboxShader;

Core::Shader_Loader shaderLoader;

obj::Model submarineModel;
obj::Model backgroundModel;
obj::Model fishModel;
obj::Model rock;
obj::Model coral;
obj::Model sharkModel;

glm::vec3 fishPosition[NUMBER_FISHES];

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

unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
unsigned int cubemapTexture;

float skyboxVertices[] =
{
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

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

void drawSkybox()
{
	GLuint program = skyboxShader;

	glUniform1i(glGetUniformLocation(program, "skybox"), 0);

	glDepthFunc(GL_LEQUAL);

	glUseProgram(program);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraDir, cameraSide)));
	projection = glm::perspective(glm::radians(45.0f), (float)800 / 800, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Switch back to the normal depth function
	glDepthFunc(GL_LESS);
}


void renderScene()
{
	//Tworzenie macierzy perspektywy za pomoca createPerspectiveMatrix(), uzyskujemy obraz 3D
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 1000, frustumScale);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// Zmienna "time"  zawiera liczbe sekund od uruchomienia programu
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - loadingTime;


	glm::mat4 attachShip = glm::mat4_cast(inverse(rotation));

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.25f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, -1)) * glm::scale(glm::vec3(0.0001f));
	glm::mat4 submarineModelMatrix = glm::translate(cameraPos + cameraDir * 1.5f) * attachShip * shipInitialTransformation;
	submarineModelMatrix *= glm::rotate(sin(cameraPos[0] + cameraPos[1] + cameraPos[2] + cameraAngle) / 8, glm::vec3(0, 1, 0));
	drawObjectTexture(&submarineModel, submarineModelMatrix, textureSubmarine);
	
	for (int i = 0; i < NUMBER_FISHES; i++) {
		drawObjectTexture(&fishModel, glm::translate(fishPosition[i]) * glm::scale(glm::vec3(0.2f)), textureFish);
	}

	drawObjectTexture(&sharkModel, glm::translate(glm::vec3(3, -2, 2)) * glm::scale(glm::vec3(0.005f)), textureShark);

	//drawObjectTextureMain(&backgroundModel, glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(100.0f)), textureBackground);

	drawSkybox();
	
	glutSwapBuffers();
}


void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	mainTexture = shaderLoader.CreateProgram("shaders/shader_tex1.vert", "shaders/shader_tex1.frag");
	skyboxShader = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");

	submarineModel = obj::loadModelFromFile("models/submarine.obj");
	//backgroundModel = obj::loadModelFromFile("models/backg.obj");
	fishModel = obj::loadModelFromFile("models/fish.obj");
	sharkModel = obj::loadModelFromFile("models/shark.obj");

	textureSubmarine = Core::LoadTexture("textures/s.png");
	//textureBackground = Core::LoadTexture("textures/water.png");
	textureFish = Core::LoadTexture("textures/fish.png");
	textureShark = Core::LoadTexture("textures/shark.png");


	for (int i = 0; i < NUMBER_FISHES; i++) {
		fishPosition[i] = glm::vec3(glm::sphericalRand(35.0f));
	}

	loadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	// Create VAO, VBO, and EBO for the skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string curDir = (fs::current_path()).string();

	std::string facesCubemap[6] =
	{
		curDir + "/skybox/left.jpg",
		curDir + "/skybox/right.jpg",
		curDir + "/skybox/top.jpg",
		curDir + "/skybox/bottom.jpg",
		curDir + "/skybox/front.jpg",
		curDir + "/skybox/back.jpg"
	};

	// Creates the cubemap texture object
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}

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
	// Kiedy rozmiar okna sie zmieni, obraz jest znieksztalcony.
	// Dostosuj odpowiednio macierz perspektywy i viewport.
	// Oblicz odpowiednio globalna zmienna "frustumScale".
	// Ustaw odpowiednio viewport - zobacz:
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glViewport.xhtml
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
