#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "models/Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <iostream>
#include <thread>

// constants
const int WIDTH = 600;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH / (float)HEIGHT;
const float FOV = 55.0f;
const int SPHERE_SECTORS = 36;
const int SPHERE_STACKS = 18;
const float G = 6.674e-11;
const float distScale = 1e11;
const float TIMESTEP = 0.015f;
float xLast = (float)WIDTH  / 2.0f;
float yLast = (float)HEIGHT / 2.0f;
bool firstMouse = true;
bool trailsOn = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

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

    r->handleCameraMovement(xOffset, yOffset);
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
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

Renderer::Renderer()
: window(nullptr), sphereShader(nullptr), trailShader(nullptr), cam(nullptr), baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS), currentTime(0.0f), oldTime(0.0f), deltaTime(0.0f), elapsedTime(0.0f)
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
    glfwSetWindowUserPointer(window, this);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: failed to load GLAD" << std::endl;
        return -1;
    }

    // Callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
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

    // Creating trail objects
    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);

    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Initialise camera and shaders
    cam = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    sphereShader = new Shader("shaders/shader.vert", "shaders/shader.frag");
    trailShader = new Shader("shaders/trailShader.vert", "shaders/trailShader.frag");

    Body blue(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 10e10f, 0.1f);
    system.push_back(blue);

    Body green(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.1f), glm::vec3(0.0f, 1.0f, 0.0f), 10e10f, 0.1f);
    system.push_back(green);

	return 0;
}

void Renderer::drop()
{
    glDeleteBuffers(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    delete sphereShader;
    sphereShader = nullptr;
    delete cam;
    cam = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::processPhysics()
{
    for (auto& a : system)
    {
        glm::vec3 acceleration(0.0f);

        for (auto& b : system)
        {
            if (&a == &b) continue;

            glm::vec3 direction = b.position - a.position;
            float distance = glm::length(direction) + 0.0001f;
            float magnitudeAcceleration = (G * b.mass) / (distance * distance);

            acceleration += magnitudeAcceleration * glm::normalize(direction);
        }

        a.acceleration = acceleration;
    }

    for (auto&a : system)
    {
        glm::vec3 temp = a.position;
        a.position = (2.0f * a.position) - (a.previousPosition) + (a.acceleration * TIMESTEP * TIMESTEP);
        a.previousPosition = temp;
        a.trail.push_back(a.position);
    }

    std::cout << deltaTime << std::endl;
}

void Renderer::renderBody(Body& body, bool withTrail)
{
    glBindVertexArray(sphereVAO);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), body.position); // transforms
    model = glm::scale(model, glm::vec3(body.radius)); // scales
    
    sphereShader->setVec3("colour", body.colour);
    sphereShader->setMat4("model", model);
    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

    if (withTrail)
    {
        glBindVertexArray(trailVAO);
        glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
        glBufferData(GL_ARRAY_BUFFER, body.trail.size() * sizeof(glm::vec3), body.trail.data(), GL_DYNAMIC_DRAW);

        trailShader->use();
        glm::mat4 projection = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
        glm::mat4 view = cam->getViewMatrix();

        trailShader->setMat4("projection", projection);
        trailShader->setMat4("view", view);
        trailShader->setVec3("colour", body.colour);
        glDrawArrays(GL_LINE_STRIP, 0, body.trail.size());
        sphereShader->use();
    }
}

void Renderer::processRendering()
{// the game loop
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    if (deltaTime > 0.25) deltaTime = 0.25;
    oldTime = currentTime;
    elapsedTime += deltaTime;

    key_callback(window, cam, deltaTime);

    //render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment for wireframe rendering

    sphereShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 model = glm::mat4(0.0f);

    sphereShader->setInt("tex", 0);
    sphereShader->setMat4("projection", projection);
    sphereShader->setMat4("view", view);

    while (elapsedTime >= TIMESTEP)
    {
        processPhysics();
        elapsedTime -= TIMESTEP;
    }

    for (auto& b : system)
    {
        renderBody(b, true);
    }

    glfwPollEvents();
    glfwSwapBuffers(window);
}

GLFWwindow* Renderer::getWindow()
{
	return window;
}

void Renderer::handleCameraMovement(float xOffset, float yOffset)
{
    cam->handleMouseMovement(xOffset, yOffset);
}