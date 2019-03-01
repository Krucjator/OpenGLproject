
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <chrono>
#include <stdlib.h>     
#include <time.h> 
#include <algorithm>
#include "shader.h"
#include "stb_image.h"
#include "CarCamera.h"
#include "StaticCamera.h"
#include "FollowCamera.h"
#include "SphereGenerator.h"
#include "Camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void checkShaderCompilation(unsigned int vertexShader);
void checkShaderProgramLinking(unsigned int shaderProgram);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
glm::mat4 myLookAt(glm::vec3 camPos, glm::vec3 targetPos, glm::vec3 up);


// settings
float SCR_WIDTH = 800;
float SCR_HEIGHT = 600;
bool cursor_locked = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


//camera
CarCamera carCamera(glm::vec3(0.0f, 0.0f, 9.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


glm::vec3 chandelier = glm::vec3(0.0f, 20.0f, 0.0f);
glm::vec3 Front = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0001f, 1.0f, 0.0f);
StaticCamera staticCamera(chandelier, chandelier + Front, up);

FollowCamera followCamera(chandelier);


enum ActiveCamera {
	staticCam,
	carCam,
	followCam
};
ActiveCamera activeCam = staticCam;
Camera activeCamera = staticCamera;




//shaders
Shader phongPhongShader("..\\OpenGLproject\\Phong.vs.txt", "..\\OpenGLproject\\Phong.fs.txt");
Shader phongFlatShader("..\\OpenGLproject\\Flat.vs.txt", "..\\OpenGLproject\\Flat.fs.txt");
Shader phongGouraudShader("..\\OpenGLproject\\Gouraud.vs.txt", "..\\OpenGLproject\\Gouraud.fs.txt");
Shader lampShader("..\\OpenGLproject\\lamp.vs.txt", "..\\OpenGLproject\\lamp.fs.txt");
Shader GnomeShader("..\\OpenGLproject\\Gnome.vs.txt", "..\\OpenGLproject\\Gnome.fs.txt");

Shader activeShader(-1);

enum LightingModeEnum {
	Blinn,
	Phong
};

LightingModeEnum lightingMode = Phong;

using std::string;

int main() {

#pragma region glfw setup
	glfwInit();
	//choose OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RacingSUPAstar", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//register callback function, later we could add key input
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#pragma endregion

#pragma region GLAD
	//initialize GLAD before calling any OpenGL function, GLAD manages function pointers for OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
#pragma endregion

	phongPhongShader.load();
	phongFlatShader.load();
	phongGouraudShader.load();
	lampShader.load();
	GnomeShader.load();
	activeShader.ID = phongPhongShader.ID;


	glm::vec3 cubePositions[] = {
	  glm::vec3(2.0f,  1.0f,  3.0f),
	  glm::vec3(1.0f,  3.2f, -3.0f),
	  glm::vec3(-1.5f, 2.2f, -2.5f),
	  glm::vec3(-3.8f, 1.0f, -12.3f),
	  glm::vec3(2.4f, 0.4f, -3.5f),
	  glm::vec3(-1.7f,  3.0f, -7.5f),
	  glm::vec3(1.3f, 5.2f, -2.5f),
	  glm::vec3(1.5f,  2.0f, -2.5f),
	  glm::vec3(1.5f,  0.2f, -1.5f),
	  glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	float roadVertices[] = {
		-0.5f,-1.0f,-0.5f ,0.0f,0.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,-1.0f,-0.5f ,0.5f,0.0f, 0.0f,  1.0f,  0.0f,
		-0.5f,-1.0f, 0.5f ,0.0f,1.0f, 0.0f,  1.0f,  0.0f,
		 0.5f,-1.0f, 0.5f ,1.0f,1.0f, 0.0f,  1.0f,  0.0f
	};

	unsigned int roadIndices[] = {  // note that we start from 0!
	0, 1, 2,   // first triangle
	1, 2, 3    // second triangle
	};

	//vertices, texCoords, normals
	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
	};

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(7.0f,  2.0f,  7.0f),
		glm::vec3(7.0f,  2.0f, -7.0f),
		glm::vec3(-7.0f, 2.0f, -7.0f),
		glm::vec3(-7.0f,  2.0f, 7.0f)
	};

	//sphere positions
	glm::vec3 earthPositions[] = {
		glm::vec3(8.0f,  1.0f,  8.0f),
		glm::vec3(5.0f,  2.0f, -7.0f),
		glm::vec3(-6.0f, 1.0f, -5.0f),
		glm::vec3(-5.0f,  2.0f, 4.0f)
	};

	srand(time(NULL));
	std::vector<std::pair<int, int>> gnomePositions;
	for (int i = -10; i < 11; i++)
	{
		for (int j = -10; j < 11; j++)
		{
			if (rand() % 5 == 0) {
				gnomePositions.push_back(std::make_pair(i, j));
			}
		
		}
	}

	//vertex array objects and vertex buffer objects
	unsigned int VBO, VAO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//index,size,flag,flag,stride,offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);


	unsigned int roadVBO, roadVAO;
	glGenBuffers(1, &roadVBO);
	glGenVertexArrays(1, &roadVAO);
	glBindVertexArray(roadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(roadVertices), roadVertices, GL_STATIC_DRAW);
	//index,size,flag,flag,stride,offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int texture1, texture2;
	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false);
	unsigned char *data = stbi_load("..\\OpenGLproject\\textures\\earth2048.bmp", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("..\\OpenGLproject\\textures\\road.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	unsigned int roadTexture;
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &roadTexture);
	glBindTexture(GL_TEXTURE_2D, roadTexture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("..\\OpenGLproject\\textures\\wood.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	unsigned int carTexture;
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &carTexture);
	glBindTexture(GL_TEXTURE_2D, carTexture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("..\\OpenGLproject\\textures\\red.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//load gnomes
	unsigned int gnomes[2];
	glGenTextures(2, gnomes);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gnomes[0]);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(false);
	data = stbi_load("..\\OpenGLproject\\textures\\Gnome.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gnomes[1]);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("..\\OpenGLproject\\textures\\gnomed.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	//ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
	//// either set it manually like so:
	//glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
	//// or set it via the texture class
	//ourShader.setInt("texture2", 1);

	//create sphere
	std::vector<float> spVertices;
	std::vector<float> spNormals;
	std::vector<float> spTexCoords;
	std::vector<int> spIndices;
	GenerateSphere(spVertices, spTexCoords, spIndices, spNormals);

	std::vector<float> joinSpVTex;
	joinSpVTex.insert(joinSpVTex.end(), spVertices.begin(), spVertices.end());
	joinSpVTex.insert(joinSpVTex.end(), spTexCoords.begin(), spTexCoords.end());
	joinSpVTex.insert(joinSpVTex.end(), spNormals.begin(), spNormals.end());
	unsigned int spVBO, spVAO;
	glGenBuffers(1, &spVBO);
	glGenVertexArrays(1, &spVAO);
	glBindVertexArray(spVAO);
	glBindBuffer(GL_ARRAY_BUFFER, spVBO);
	glBufferData(GL_ARRAY_BUFFER, joinSpVTex.size() * sizeof(float), &(joinSpVTex[0]), GL_STATIC_DRAW);
	//index,size,flag,flag,stride,offset
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(float) * spVertices.size()));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(sizeof(float) * (spVertices.size() + spTexCoords.size())));
	glEnableVertexAttribArray(2);
	int spVerSize = spVertices.size();

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
	activeShader.use();
	glEnable(GL_DEPTH_TEST);
	double previousTime = glfwGetTime();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	


	int frameCount = 0;
	//render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		frameCount++;
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		// If a second has passed.
		if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() >= 1.0)
		{
			// Display the frame count here any way you want.
			std::cout << "FPS: "<<frameCount << std::endl;
			frameCount = 0;
			begin = std::chrono::steady_clock::now();
		}


		// input
		// -----
		processInput(window);

		//rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		activeShader.use();
		//setup lights
		for (int i = 0; i < 4; i++)
		{
			string number = std::to_string(i);
			activeShader.setVec3(("pointLights[" + number + "].position").c_str(), pointLightPositions[i]);
			activeShader.setVec3(("pointLights[" + number + "].ambient").c_str(), glm::vec3(0.05f, 0.05f, 0.05f));
			activeShader.setVec3(("pointLights[" + number + "].diffuse").c_str(), glm::vec3(0.8f, 0.8f, 0.8f));
			activeShader.setVec3(("pointLights[" + number + "].specular").c_str(), glm::vec3(1.0f, 1.0f, 1.0f));
			activeShader.setFloat(("pointLights[" + number + "].constant").c_str(), 1.0f);
			activeShader.setFloat(("pointLights[" + number + "].linear").c_str(), 0.09f);
			activeShader.setFloat(("pointLights[" + number + "].quadratic").c_str(), 0.032f);
		}

		// spotLight properties
		activeShader.setVec3("spotLight.position", carCamera.Position);
		activeShader.setVec3("spotLight.direction", carCamera.Front);
		activeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		activeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
		activeShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
		activeShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
		activeShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		activeShader.setFloat("spotLight.constant", 1.0f);
		activeShader.setFloat("spotLight.linear", 0.09f);
		activeShader.setFloat("spotLight.quadratic", 0.032f);

		switch (lightingMode) {
		case Blinn:
			activeShader.setInt("mode", 1);
			break;
		case Phong:
			activeShader.setInt("mode", 2);
			break;
		}


		glm::mat4 view;
		switch (activeCam) {
		case staticCam:
			view = staticCamera.GetViewMatrix();
			activeShader.setVec3("viewPos", staticCamera.Position);
			break;
		case carCam:
			view = carCamera.GetViewMatrix();
			activeShader.setVec3("viewPos", carCamera.Position);

			break;
		case followCam:
			glm::vec3 carPos = carCamera.GetCarPosition();
			view = followCamera.GetViewMatrix(carPos);
			activeShader.setVec3("viewPos", followCamera.Position);
			break;
		}

		activeShader.setMat4("view", view);
		projection = glm::perspective(glm::radians(45.0f), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		activeShader.setMat4("projection", projection);
		
		
		glBindVertexArray(VAO);
		activeShader.setInt("texture1", 1);
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle) * (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
			activeShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindVertexArray(VAO);
		//draw car
		activeShader.setInt("texture1", 3);
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(1, 1, 2));
		model = glm::translate(model, carCamera.GetCarPosition());
		model = glm::rotate(model, -glm::radians(carCamera.CarYaw), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2, 1, 1));
		activeShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		
		
		//gnomes
		if (activeShader.ID == GnomeShader.ID) {
			activeShader.setInt("texture1", 6);
			
			glBindVertexArray(VAO);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model,glm::vec3(0.0f,-5.0f,0.0f));
			model = glm::scale(model, glm::vec3(2, 1, 1));
			activeShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);


			activeShader.setInt("texture1", 2);
			activeShader.setInt("texture2", 5);
		}

		//draw floor
		glBindVertexArray(roadVAO);
		activeShader.setInt("texture1", 2);
		
		for (int i = -10; i < 11; i++)
		{
			for (int j = -10; j < 11; j++)
			{
				//gnomes
				if (activeShader.ID == GnomeShader.ID) {
					if (std::find(gnomePositions.begin(), gnomePositions.end(), std::make_pair(i, j)) != gnomePositions.end())
					{
						activeShader.setInt("gnomeshere", 1);
					}
					else {
						activeShader.setInt("gnomeshere", 0);
					}
				}
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3((float)i, 0.0f, (float)j));
				activeShader.setMat4("model", model);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, roadIndices);
			}
		}
		

		//draw sphere (in the center)
		glBindVertexArray(spVAO);
		activeShader.setInt("sphere", 1);
		activeShader.setInt("texture1", 0);
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		activeShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, spVerSize * sizeof(float), GL_UNSIGNED_INT, &spIndices[0]);
		activeShader.setInt("sphere", 0);
		//draw earths
		for (size_t i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, earthPositions[i]);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			activeShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, spVerSize * sizeof(float), GL_UNSIGNED_INT, &spIndices[0]);
		}

		// also draw the lamp object
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		
		
		glBindVertexArray(spVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // a smaller sphere
			lampShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, spVerSize * sizeof(float), GL_UNSIGNED_INT, &spIndices[0]);
		}

		//gnome
		if (activeShader.ID == GnomeShader.ID) {
			for (size_t i = 0; i < gnomePositions.size(); i++)
			{
				if (((carCamera.Position.x - gnomePositions[i].first)*(carCamera.Position.x - gnomePositions[i].first) +
					(carCamera.Position.z - gnomePositions[i].second)*(carCamera.Position.z - gnomePositions[i].second)) < 0.25f) {
					//gnomed
					carCamera.MovementSpeed = 0.0f;
					carCamera.MouseSensitivity = 0.0f;
					carCamera.Yaw = -90.0f;
					carCamera.Pitch = 0.0f;
					carCamera.Position = glm::vec3(0.0f, -5.0f, 3.0f);
				}

			}
		}

		//double buffering prevents flickering, swaps buffer and presents the ready one
		glfwSwapBuffers(window);
		//checks if any events are triggered (like keyboard or mouse movement input), then calls appropriate callbacks
		glfwPollEvents();
	}
	//cleanup
	glfwTerminate();


	return 0;
}

void checkShaderCompilation(unsigned int shader)
{
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
}

void checkShaderProgramLinking(unsigned int shaderProgram)
{
	int  success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
}

//Changes rendering area each time glfw window resizes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		carCamera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		carCamera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		carCamera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		carCamera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		activeCam = staticCam;
		activeCamera = staticCamera;
	}

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		activeCam = carCam;
		activeCamera = carCamera;
	}

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		activeCam = followCam;
		activeCamera = followCamera;
	}

	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		activeShader = phongFlatShader.ID;
	}

	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		activeShader = phongGouraudShader.ID;
	}

	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		activeShader = phongPhongShader.ID;
	}
		
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		lightingMode = Phong;
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		lightingMode = Blinn;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		activeShader = GnomeShader.ID;
		carCamera.Position = glm::vec3(0.0f, 0.0f, 9.0f);
	}


	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	carCamera.ProcessMouseMovement(xoffset, yoffset);
}
