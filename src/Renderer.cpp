#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

// constants
const int WIDTH = 600;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH / (float)HEIGHT;
const float FOV = 55.0f;
const int SPHERE_SECTORS = 36;
const int SPHERE_STACKS = 18;
float xLast = (float)WIDTH  / 2.0f;
float yLast = (float)HEIGHT / 2.0f;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos, Camera* cam) {
    xPos = static_cast<float>(xPos);
    yPos = static_cast<float>(yPos);

    if (firstMouse)
    {
        xLast = xPos;
        yLast = yPos;
        firstMouse = false;
    }

    // get displacement from previous frame
    float xOffset = xPos - xLast;
    float yOffset = yLast - yPos;
    xLast = xPos;
    yLast = yPos;

    cam->handleMouseMovement(xOffset, yOffset);
}

void key_callback(GLFWwindow* window, Camera* cam, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cam->handleKeyboard(CameraMove::DOWN, deltaTime);
    }
}

Renderer::Renderer()
: window(nullptr), baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS), shader(NULL, NULL), cam(glm::vec3(0.0f))
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

    Body earth(glm::vec3(0.0f, 0.0f, 0.0f), 10e10f, 1.0f);
    system.push_back(earth);

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
    glm::mat4 model = glm::translate(glm::mat4(1.0f), body.position); // transforms
    model = glm::scale(model, glm::vec3(body.radius)); // scales
    
    shader.setMat4("model", model);
    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::processRendering()
{
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    oldTime = currentTime;
    
    key_callback(window, &cam, deltaTime);

    //render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment for wireframe rendering

    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
    glm::mat4 view = cam.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    shader.setInt("tex", 0);
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glBindVertexArray(sphereVAO);
    for (auto& b : system)
    {
        renderBody(b);
    }

    glfwSwapBuffers(window);
}

GLFWwindow* Renderer::getWindow()
{
	return window;
}