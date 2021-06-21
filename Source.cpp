// 3D Graphics and Animation - Main Template
// Visual Studio 2019
// Last Changed 01/10/2019

#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")
#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include <GL/glew.h>			// Add library to extend OpenGL to newest version
#include <GLFW/glfw3.h>			// Add library to launch a window
#include <GLM/glm.hpp>			// Add helper maths library
#include <GLM/gtx/transform.hpp>

#include <stb_image.h>			// Add library to load images for textures

#include "Mesh.h"				// Simplest mesh holder and OBJ loader - can update more - from https://github.com/BennyQBD/ModernOpenGLTutorial


// MAIN FUNCTIONS
void setupRender();
void startup();
void update(GLfloat currentTime);
void render(GLfloat currentTime);
void endProgram();

// HELPER FUNCTIONS OPENGL
void hintsGLFW();
string readShader(string name);
void checkErrorShader(GLuint shader);
void errorCallbackGLFW(int error, const char* description);
void debugGL();
static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);

// CALLBACK FUNCTIONS FOR WINDOW
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);


// VARIABLES
GLFWwindow*		window;											// Keep track of the window
int				windowWidth = 640;				
int				windowHeight = 480;
bool			running = true;									// Are we still running?
glm::mat4		proj_matrix;									// Projection Matrix
glm::vec3		cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);	// Week 5 lecture
glm::vec3		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float           aspect = (float)windowWidth / (float)windowHeight;
float			fovy = 45.0f;
bool			keyStatus[1024];
GLfloat			deltaTime = 0.0f;
GLfloat			lastTime = 0.0f;
GLuint			program;
GLint			proj_location;

const int		noModels = 15;
Mesh			model[noModels];			// Array of loaded models
glm::vec3		modelPosition[noModels];	// Array of model positions
glm::vec3		modelRotation[noModels];	// array of model rotations

const int noTextures = 3;
string textureName[noTextures];
GLuint texture[noTextures];
int tex_location;

GLfloat		ka = 1.5;								// Ambient constant
glm::vec4	ia = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Ambient colour
glm::vec4	id = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Diffuse colour
glm::vec4	is = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);	// Specular colour

Mesh		lightModel;
glm::vec3   lightDisp = glm::vec3(-0.369f, 0.652f, -0.332f);

// FPS camera variables
GLfloat			yaw = -90.0f;					// init pointing to inside
GLfloat			pitch = 0.0f;					// start centered
GLfloat			lastX = windowWidth /  2.0f;	// start middle screen
GLfloat			lastY = windowHeight / 2.0f;	// start middle screen
bool			firstMouse = true;



int main()
{
	if (!glfwInit()) {							// Checking for GLFW
		cout << "Could not initialise GLFW...";
		return 0;
	}

	glfwSetErrorCallback(errorCallbackGLFW);	// Setup a function to catch and display all GLFW errors.

	hintsGLFW();								// Setup glfw with various hints.		

												// Start a window using GLFW
	string title = "My OpenGL Application";

	// Fullscreen
	//const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	// windowWidth = mode->width; windowHeight = mode->height; //fullscreen
	// window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL); // fullscreen

	// Window
	window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
	if (!window) {								// Window or OpenGL context creation failed
		cout << "Could not initialise GLFW...";
		endProgram();
		return 0;
	}

	glfwMakeContextCurrent(window);				// making the OpenGL context current

												// Start GLEW (note: always initialise GLEW after creating your window context.)
	glewExperimental = GL_TRUE;					// hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
	GLenum errGLEW = glewInit();
	if (GLEW_OK != errGLEW) {					// Problems starting GLEW?
		cout << "Could not initialise GLEW...";
		endProgram();
		return 0;
	}

	debugGL();									// Setup callback to catch openGL errors.	

	// Setup all the message loop callbacks.
	glfwSetWindowSizeCallback(window, onResizeCallback);			// Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);						// Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);		// Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);			// Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);			// Set callback for mouse wheel.
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor. Fullscreen
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS fullscreen.

	setupRender();								// setup some render variables.
	startup();									// Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	do {												// run until the window is closed
		GLfloat currentTime = (GLfloat)glfwGetTime();	// retrieve timelapse
		deltaTime = currentTime - lastTime;				// Calculate delta time
		lastTime = currentTime;							// Save for next frame calculations.
		glfwPollEvents();								// poll callbacks
		update(currentTime);							// update (physics, animation, structures, etc)
		render(currentTime);							// call render function.

		glfwSwapBuffers(window);						// swap buffers (avoid flickering and tearing)

		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);	// exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram();			// Close and clean everything up...

	cout << "\nPress any key to continue...\n";
	cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);

	if (firstMouse) {
		lastX = (float)mouseX; lastY = (float)mouseY; firstMouse = false;
	}

	GLfloat xoffset = mouseX - lastX;
	GLfloat yoffset = lastY - mouseY; // Reversed
	lastX = (float)mouseX; lastY = (float)mouseY;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity; yoffset *= sensitivity;

	yaw += xoffset; pitch += yoffset;

	// check for pitch out of bounds otherwise screen gets flipped
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	cameraFront = glm::normalize(front);
}

void errorCallbackGLFW(int error, const char* description) {
	cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW() {
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);			// Create context in debug mode - for debug message callback
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_SAMPLES, 4);
	//glEnable(GL_MULTISAMPLE);
}

void endProgram() {
	glfwMakeContextCurrent(window);		// destroys window handler
	glfwTerminate();	// destroys all windows and releases resources.
}

void setupRender() {
	glfwSwapInterval(1);	// Ony render when synced (V SYNC)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 2);
	glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void startup() {

	// Load main object model and shaders
	model[0].LoadModel("fridge/obj_files/fridge_body_hollow.obj");
	model[1].LoadModel("fridge/obj_files/fridge_door.obj");
	model[2].LoadModel("fridge/obj_files/fridge_drawer.obj");
	model[3].LoadModel("fridge/obj_files/fridge_shelf_border.obj");
	model[4].LoadModel("fridge/obj_files/fridge_shelf_border.obj");
	model[5].LoadModel("fridge/obj_files/fridge_shelf_border.obj");
	model[6].LoadModel("fridge/obj_files/fridge_shelf_border.obj");
	model[7].LoadModel("fridge/obj_files/fridge_shelf_door_large.obj");
	model[8].LoadModel("fridge/obj_files/fridge_shelf_door_small.obj");
	model[9].LoadModel("fridge/obj_files/fridge_shelf_door_small.obj");
	model[10].LoadModel("fridge/obj_files/fridge_shelf_door_small.obj");
	model[11].LoadModel("fridge/obj_files/fridge_shelf_large.obj");
	model[12].LoadModel("fridge/obj_files/fridge_shelf_large.obj");
	model[13].LoadModel("fridge/obj_files/fridge_shelf_large.obj");
	model[14].LoadModel("fridge/obj_files/fridge_shelf_small.obj");
	lightModel.LoadModel("basic_cube.obj");

	printf("*** %d models loaded ***\n", (int)(sizeof(model) / sizeof(*model)));

	// Start all models from the centre
	for (int i = 0; i < noModels; i++) {
		modelPosition[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		modelRotation[i] = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// Manually align middle shelves
	modelPosition[4] = glm::vec3(0.0f, -0.25f, 0.0f);
	modelPosition[5] = glm::vec3(0.0f, -0.50f, 0.0f);
	modelPosition[6] = glm::vec3(0.0f, -0.78f, 0.05f);

	modelPosition[9] = glm::vec3(0.0f, -0.35f, 0.0f);
	modelPosition[10] = glm::vec3(0.0f, -0.7f, 0.0f);

	modelPosition[12] = glm::vec3(0.0f, -0.25f, 0.0f);
	modelPosition[13] = glm::vec3(0.0f, -0.50f, 0.0f);


	// Load Texture OPENGL 4.3 
	textureName[0] = "fridge/uv_maps/fridge_body.png";
	textureName[1] = "fridge/uv_maps/fridge_door.png";
	textureName[2] = "fridge/uv_maps/fridge_trans_plastic.png";

	// Generate the number of textures we need
	glGenTextures(noTextures, texture);

	for (int t = 0; t < noTextures; t++) {
		// Load image Information.
		stbi_set_flip_vertically_on_load(true);
		int iWidth, iHeight, iChannels;
		unsigned char* iData = stbi_load(textureName[t].c_str(), &iWidth, &iHeight, &iChannels, 4);

		// Load and create a texture 
		// All upcoming operations now have effect on this texture object	
		glBindTexture(GL_TEXTURE_2D, texture[t]);						
		glTexStorage2D(GL_TEXTURE_2D, t+1, GL_RGBA8, iWidth, iHeight);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iData);

		// This only works for 2D Textures...
		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate mipmaps (next lecture)
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	program = glCreateProgram();

	tex_location = glGetUniformLocation(program, "tex");

	string vs_text = readShader("vs_model_shaded.glsl"); const char* vs_source = vs_text.c_str();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	checkErrorShader(vs);
	glAttachShader(program, vs);

	string fs_text = readShader("fs_model_shaded.glsl"); const char* fs_source = fs_text.c_str();
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	checkErrorShader(fs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glUseProgram(program);
	
	// A few optimizations.
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Calculate proj_matrix for the first time.
	aspect = (float)windowWidth / (float)windowHeight;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update(GLfloat currentTime) {

	// Calculate movement
	GLfloat cameraSpeed = 1.0f * deltaTime;
	if (keyStatus[GLFW_KEY_W]) cameraPosition += cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_S]) cameraPosition -= cameraSpeed * cameraFront;
	if (keyStatus[GLFW_KEY_A]) cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keyStatus[GLFW_KEY_D]) cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	// Use the arrow keys to translate our light
	if (keyStatus[GLFW_KEY_LEFT])		lightDisp.x += 1.0f * cameraSpeed;
	if (keyStatus[GLFW_KEY_RIGHT])		lightDisp.x -= 1.0f * cameraSpeed;
	if (keyStatus[GLFW_KEY_UP])			lightDisp.z += 1.0f * cameraSpeed;
	if (keyStatus[GLFW_KEY_DOWN])		lightDisp.z -= 1.0f * cameraSpeed;
	if (keyStatus[GLFW_KEY_PAGE_UP])	lightDisp.y += 1.0f * cameraSpeed;
	if (keyStatus[GLFW_KEY_PAGE_DOWN])	lightDisp.y -= 1.0f * cameraSpeed;

	//printf("%f %f %f\n", lightDisp.x, lightDisp.y, lightDisp.z);

	int doorPieces[5]	= { 1, 7, 8, 9, 10 };
	int shelves[8]		= { 3, 12, 5, 14, 11, 4, 13, 6 };

	// Rotate door
	if (keyStatus[GLFW_KEY_1]) {
		if (modelRotation[doorPieces[1]].y <= 1.0f) {
			for (int i = 0; i < 5; i++) {
				modelRotation[doorPieces[i]].y += 1.0f * cameraSpeed;
			}
		}
	}
	if (keyStatus[GLFW_KEY_2]) {
		if (modelRotation[doorPieces[1]].y >= -2.065f) {
			for (int i = 0; i < 5; i++) {
				modelRotation[doorPieces[i]].y -= 1.0f * cameraSpeed;
			}
		}
	}

	// Pull / Push shelves
	if (keyStatus[GLFW_KEY_3]) {
		for (int i = 0; i < 8; i++) {
			modelPosition[shelves[i]].z += (1.0f + (i % 4)) * (0.2f * deltaTime);
		}
	}
	if (keyStatus[GLFW_KEY_4]) {
		for (int i = 0; i < 8; i++) {
			// We cap the backwards distance by shelf no. 6 because
			// it sticks out slightly
			if (modelPosition[shelves[7]].z >= 0.05f) {
				modelPosition[shelves[i]].z -= (1.0f + (i % 4)) * (0.2f * deltaTime);
			}
		}
	}
}

void render(GLfloat currentTime) {
	glViewport(0, 0, windowWidth, windowHeight);

	// Clear colour buffer
	glm::vec4 backgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f); glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

	// Clear deep buffer
	static const GLfloat one = 1.0f; glClearBufferfv(GL_DEPTH, 0, &one);

	// Enable blend
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader programs
	glUseProgram(program);

	glUniform4f(glGetUniformLocation(program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
	glUniform4f(glGetUniformLocation(program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
	glUniform4f(glGetUniformLocation(program, "ia"), ia.r, ia.g, ia.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "ka"), ka);
	glUniform4f(glGetUniformLocation(program, "id"), id.r, id.g, id.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "kd"), 1.0f);
	glUniform4f(glGetUniformLocation(program, "is"), is.r, is.g, is.b, 1.0);
	glUniform1f(glGetUniformLocation(program, "ks"), 1.0f);
	glUniform1f(glGetUniformLocation(program, "shininess"), 32.0f);

	glm::mat4 viewMatrix = glm::lookAt(
		cameraPosition,					// eye
		cameraPosition + cameraFront,	// centre
		cameraUp);						// up

	for (int i = 0; i < (noModels); i++) {
		// For each model...

		// ...do some translations, rotations and scaling
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), modelPosition[i]);
		modelMatrix = glm::rotate(modelMatrix, modelRotation[i].x, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, modelRotation[i].y, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

		glm::mat4 mv_matrix = viewMatrix * modelMatrix;

		// ...bind textures
		if (i == 1) {
			// ...if the model is the fridge door:
			glBindTexture(GL_TEXTURE_2D, texture[1]);
		}
		else if (i == 2 || i >= 7)
		{
			// ...if the model is a transparent shelf:
			glBindTexture(GL_TEXTURE_2D, texture[2]);
		}
		else
		{
			// ...otherwise, use a standard texture:
			glBindTexture(GL_TEXTURE_2D, texture[0]);
		}

		glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "proj_matrix"), 1, GL_FALSE, &proj_matrix[0][0]);

		// Draw the model
		model[i].Draw();

	}

	//// Render a cube representing our light
	//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), lightDisp);
	//modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
	//glm::mat4 mv_matrix = viewMatrix * modelMatrix;

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture[0]);
	//glUniform1i(tex_location, 0);

	//glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(program, "view_matrix"),  1, GL_FALSE, &viewMatrix[0][0]);
	//glUniformMatrix4fv(glGetUniformLocation(program, "proj_matrix"),  1, GL_FALSE, &proj_matrix[0][0]);

	//// Draw the model
	//lightModel.Draw();
}

void onResizeCallback(GLFWwindow* window, int w, int h) {
	windowWidth = w;
	windowHeight = h;

	aspect = (float)w / (float)h;
	proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) keyStatus[key] = true;
	else if (action == GLFW_RELEASE) keyStatus[key] = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	int yoffsetInt = static_cast<int>(yoffset);
}

void debugGL() {
	// Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;

	// Enable Opengl Debug
	//glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam) {

	cout << "---------------------opengl-callback------------" << endl;
	cout << "Message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << " --- ";

	cout << "id: " << id << " --- ";
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "NOTIFICATION";
	}
	cout << endl;
	cout << "-----------------------------------------" << endl;
}

string readShader(string name) {
	string vs_text;
	std::ifstream vs_file(name);

	string vs_line;
	if (vs_file.is_open()) {

		while (getline(vs_file, vs_line)) {
			vs_text += vs_line;
			vs_text += '\n';
		}
		vs_file.close();
	}
	return vs_text;
}

void  checkErrorShader(GLuint shader) {
	// Get log lenght
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}
}
