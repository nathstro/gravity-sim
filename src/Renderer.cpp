#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "models/Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

// constants
const int WIDTH = 600;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH / (float)HEIGHT;
const float FOV = 55.0f;
const int SPHERE_SECTORS = 36;
const int SPHERE_STACKS = 18;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

Renderer::Renderer()
: window(nullptr), baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS), shader(NULL, NULL)
{}

int Renderer::init()
{
	// Initialise GLFW
	if (!glfwInit()) {
		std::cout << "ERROR: failed to initialise GLFW" << std::endl;
		return -1;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
    	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

    // Initialise window
    window = glfwCreateWindow(WIDTH, HEIGHT, "Gravity Simulator", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: failed to load GLAD" << std::endl;
        return -1;
    }

    // Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Creating sphere and objects
    baseSphere = Sphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS);

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    auto& sphereVertices = baseSphere.getVertices();
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    auto& sphereIndices = baseSphere.getIndices();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	return 0;
}

void Renderer::drop()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

/*void Renderer::processPhysics(Body body)
{

}*/

void Renderer::renderBody(Body body)
{

}

void Renderer::processRendering(std::vector<Body>& toRender)
{
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    oldTime = currentTime;
    
    //processInput(window, deltaTime);

    //render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment for wireframe rendering

    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
    glm::mat4 view = cam.getViewMatrix();
    
    shader.setInt("tex", 0);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(sphereVAO);
    for (auto& b : toRender)
    {
        renderBody(b);
    }

    glfwSwapBuffers(window);
}

GLFWwindow* Renderer::getWindow()
{
	return window;
}