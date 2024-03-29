


#include <stdio.h>
#include "pch.h"
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <string>
#include <stdlib.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include<assimp/Importer.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "Material.h"
#include "CommonValues.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Model.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


// Window dimensions
const float toRadians = (3.14159265f/180.0f);
const float maxFOV = 47.0f;
const float minFOV = 44.0f;

Window mainWindow;
Camera camera;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

Texture brickTexture;
Texture dirtTexture;

Material shinyMaterial;
Material dullMaterial;


DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Model ironMan;

std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

bool showSpotlight = true;
bool showPointlight = false;
bool showWireframe, showNormal, showPoints, randomizeDuck, randomizeDuck2;
enum class RENDERMODE
{
	WIREFRAME = 0, NORMAL = 1, POINTS = 2
};
RENDERMODE renderMode, lastFrameRenderMode;	//0 for normal, 1 for wireframe, 2 for points

ImVec4 ambientColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Vertex Shader code
static const char* vShader = "shaders/vert.txt";

// Fragment Shader
static const char* fShader = "shaders/frag.txt";

//model movement
float currRotXAngle = 3.14f, currRotYAngle = 0.0f;
float fov = 45.0f;


float buttonPressTimer = 0.0f;


void CalculateAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, unsigned int vLenght, unsigned int normalOffset) {

	
	for (size_t i = 0; i < indiceCount; i += 3) {

		unsigned int in0 = indices [i]  * vLenght;         //gets indice to vertice by multiplying by 8 in this case , indice 3 * 8 = 24  ---> 24 on the vertices array
		unsigned int in1 = indices[i+1] * vLenght;
		unsigned int in2 = indices[i+2] * vLenght;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);
		
		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;


		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLenght; i++) {

		unsigned int nOffset = i * vLenght + normalOffset;  
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x;  vertices[nOffset+1] = vec.y;   vertices[nOffset+2] = vec.z;
	}
}

void CreateObjects()
{

	unsigned int indices[] = {
		0,3,1,
		1,3,2,
		2,3,0,
		0,1,2
	};
	GLfloat vertices[] = {
		//x      y     z         u     v        nx   ny  nz
		-1.0f, -1.0f, -0.6f,     0.0f, 0.0f,    0.0f  ,0.0f ,0.0f,
		 0.0f,-1.0f,1.0f,        0.5f, 0.0f,    0.0f  ,0.0f ,0.0f,
		1.0f, -1.0f, -0.6f,      1.0f, 0.0f,    0.0f  ,0.0f ,0.0f,
		0.0f, 1.0f, 0.0f ,      0.5f, 1.0f,    0.0f  ,0.0f ,0.0f
	};											     



	GLfloat floorVertices[] = {
   
		-10.0f,0.0f,-10.0f,    0.0f,0.0f,      0.0f  ,-1.0f ,0.0f,
		 10.0f,0.0f,-10.0f,    10.0f,0.0f,	   0.0f  ,-1.0f ,0.0f,
		-10.0f,0.0f,10.0f,     0.0f,10.0f,	   0.0f  ,-1.0f ,0.0f,
		 10.0f,0.0f,10.0f,     10.0f,10.0f,	   0.0f  ,-1.0f ,0.0f

	};
	unsigned int  floorIndices[]{
		0,2,1,
		1,2,3
	};

	CalculateAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh *obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh *obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);


	Mesh *obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);


}

void CreateShaders() {

	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

 
int main()
{	
	showNormal = true, showWireframe = false, showPoints = false;
	showSpotlight = true, showPointlight = true;
	renderMode = RENDERMODE::NORMAL;
	lastFrameRenderMode = (RENDERMODE)-1;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	mainWindow = Window(1920, 1080);
	mainWindow.initialise();

	CreateObjects();
	CreateShaders();

	//initialise camera class
	camera = Camera(glm::vec3(0.0f, 1.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f,5.0f,0.004f);

	//allocate memory to uniform varriables
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformAmbienIntensity = 0, uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0, uniformTimeElapsed = 0, uniformRandomize = 0;
	
	//initialise projection matrix
	glm::mat4 projection = glm::perspective(fov,((GLfloat)mainWindow.getBufferWidth()/ (GLfloat)mainWindow.getBufferHeight()),0.1f,100.0f);

	//texture initialisation
	brickTexture = Texture("textures/clouds.png");
	brickTexture.LoadTexture();
	dirtTexture = Texture("textures/dirt.png");
	dirtTexture.LoadTexture();

	//custom material materials initialisation
	shinyMaterial = Material("shiny", 1.0f, 32);
	dullMaterial = Material("dull", 0.3f, 4);




	//model class allocation and model initialisation loading it from file
	ironMan = Model();
	ironMan.LoadModel("models/duck.obj");

	//light system initialisation
	//1 directionalLight aka ambient light and various spotlights and pointlights
	mainLight = DirectionalLight(0.5f, 0.4f, 0.4f,
		0.4f, 0.2f,
		0.0f, 0.0f, -1.0f);

	unsigned int pointLightCount = 0;
	unsigned int spotLightCount = 0;
	pointLights[0] = PointLight(1.0f, 0.0f, 0.0f,
		0.8f, 1.3f,
		4.0f, 0.0f, 0.0f,
		0.3f, 0.2f, 0.1f
	);
	pointLightCount++;
	pointLights[1] = PointLight(1.0f, 0.0f, 0.0f,
		0.8f, 1.3f,
		-4.0f, 0.0f, 0.0f,
		0.3f, 0.2f, 0.1f
	);
	pointLightCount++;
	spotLights[0] = SpotLight(1.0f, 0.3f, 0.0f,
		12.0f, 12.0f,
		0.0f, 3.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		3.0f, 2.0f, 1.0f,
		30.0f);

	spotLightCount++;

	ambientColor = ImVec4(mainLight.getColor().r, mainLight.getColor().g, mainLight.getColor().b, 1.f);


	//IMGUI initialisation

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(mainWindow.mainWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	camera.MouseControl(mainWindow.getXChange(), mainWindow.getYChange());

	GLfloat oldScrollValue = 0.0f;
	// Loop until window closed
	while (!mainWindow.getShouldSlose())
	{

		if (lastFrameRenderMode != renderMode)
		{
			switch (renderMode)
			{

			case RENDERMODE::NORMAL:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				showPointlight = true, showSpotlight = true;
				mainLight.setAmbientIntensity(1.f);
			break;

			case RENDERMODE::WIREFRAME:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				showSpotlight = false, showPointlight = false;
				mainLight.setAmbientIntensity(0.f);
				break;

			case RENDERMODE::POINTS:
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				showSpotlight = false, showPointlight = false;
				mainLight.setAmbientIntensity(0.f);
				break;
			default:
				break;
			}

			/*GLint* param = new GLint();
			glGetIntegerv(GL_POLYGON_MODE, param);*/


			lastFrameRenderMode = renderMode;
		}

		//calculating delta time
		GLfloat now = glfwGetTime(); 
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle user input events
		glfwPollEvents();


		if (showPointlight) pointLightCount = 1;
		else pointLightCount = 0;

		if (showSpotlight) spotLightCount = 1;
		else spotLightCount = 0;


		//camera controller
		if (glfwGetMouseButton(mainWindow.mainWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::IsAnyItemHovered() && !ImGui::IsMouseHoveringAnyWindow() && !ImGui::IsAnyWindowFocused())
		{
			buttonPressTimer += deltaTime;

			//so if we just want to press buttons on the ui, the model wont rotate
			if (buttonPressTimer > 0.1f)
			{
				currRotXAngle += mainWindow.getXChange() * toRadians;
				currRotYAngle += mainWindow.getYChange() * toRadians;
			}
			
		}
		else buttonPressTimer = 0.0f;

		//calculate new FOV based on scroll wheel values
		GLfloat newScrollValue = mainWindow.getScroll();
		if (oldScrollValue != newScrollValue)
		{
			fov += (oldScrollValue - newScrollValue) * 0.05f;
			fov = glm::clamp(fov, minFOV, maxFOV);
			projection = glm::perspective(fov, ((GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight()), 0.1f, 100.0f);
			oldScrollValue = newScrollValue;

		}
		

		// Clear window
		if(renderMode != RENDERMODE::NORMAL)	glClearColor(1.f, 0.99f, 0.99f, 1.0f);
		else									glClearColor(0.8f, 0.8f, 0.8f, 1.f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();
		uniformTimeElapsed = shaderList[0].GetTimeElapsedLocation();
		uniformRandomize = shaderList[0].GetRandomizeLocation();

		Material::ShaderLocations matLoc = 
		{
			 uniformSpecularIntensity,
			 uniformShininess		
		};
		

		//feed into shader light values
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		
		
		

		//feed data into unfirm values
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		
		//render ground section
		glm::mat4 model=glm::mat4(1.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		dirtTexture.UseTexture();
		dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();
		//----------------------------------

		//Model render section
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

		//start rotation of the model
		model = glm::rotate(model, 3.14f, glm::vec3(0.0f, 1.0f, 1.0f));
		//incremented rotations
	    model = glm::rotate(model, currRotXAngle, glm::vec3(0.0f,0.0f,1.0f));
		model = glm::rotate(model, currRotYAngle, glm::vec3(1.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		// send deltaTime to shader
		glUniform1f(uniformTimeElapsed, (float)glfwGetTime()); 
		
		// send randomize duck type
		if (!randomizeDuck && !randomizeDuck2)
			glUniform1i(uniformRandomize, 0);
		else if (randomizeDuck && randomizeDuck2)
			glUniform1i(uniformRandomize, 3);
		else if (randomizeDuck)
			glUniform1i(uniformRandomize, 1);
		else if (randomizeDuck2)
			glUniform1i(uniformRandomize, 2);

		

		if (renderMode != RENDERMODE::NORMAL) ironMan.RenderModel(false, matLoc);
		else
		{
			dullMaterial.UseMaterial(matLoc.specularIntensityLocation, matLoc.shininessLocation);
			ironMan.RenderModel(true, matLoc);
		}

		//------------------------

		//IMGUI 
		{
			static float lightIntensity = 0.5f;
			static int counter = 0;

			ImGui::Begin("EDJD - Programacao 3D");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Wireframe" ,&showWireframe);
			if (showWireframe)
			{
				renderMode = RENDERMODE::WIREFRAME;
				showPoints = false;
			}
			ImGui::Checkbox("Points", &showPoints);
			if (showPoints)
			{
				renderMode = RENDERMODE::POINTS;
				showWireframe = false;
			}
			else if(renderMode == RENDERMODE::NORMAL)
			{
				ImGui::Checkbox("SpotLight", &showSpotlight);      // Edit bools storing our window open/close state
				ImGui::Checkbox("PointLight", &showPointlight);
			}
			
			ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("Light Color", (float*)&ambientColor); // Edit 3 floats representing a color

			ImGui::Checkbox("Randomize type 1", &randomizeDuck);
			ImGui::Checkbox("Randomize type 2", &randomizeDuck2);

			// Update light color and intensity if set values are diferent
			if (ambientColor.x * lightIntensity != mainLight.getColor().r ||
				ambientColor.y * lightIntensity != mainLight.getColor().g ||
				ambientColor.z * lightIntensity != mainLight.getColor().b){
					mainLight.setColor(glm::vec3(ambientColor.x * lightIntensity,
												 ambientColor.y * lightIntensity,
											     ambientColor.z * lightIntensity));
			}


			//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			//	counter++;
			//ImGui::SameLine();
			//ImGui::Text("counter = %d", counter);

			ImGui::Text("Latency %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		if (!showWireframe && !showPoints) renderMode = RENDERMODE::NORMAL;
		
		glUseProgram(0);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		mainWindow.swapBuffers();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	return 0;
}