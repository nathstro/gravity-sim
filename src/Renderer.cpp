#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "models/Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <cstdlib>
#include <iostream>
#include <random>

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
const int STAR_COUNT = 500;
const float STAR_RADIUS = 50.0f;

float xLast = (float)WIDTH  / 2.0f;
float yLast = (float)HEIGHT / 2.0f;
bool firstMouse = true;

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
}

Renderer::Renderer()
: window(nullptr), shader(nullptr), cam(nullptr), baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS), currentTime(0.0f), oldTime(0.0f), deltaTime(0.0f), elapsedTime(0.0f)
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
    
    // Initialise camera and shader
    cam = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    shader = new Shader("shaders/shader.vert", "shaders/shader.frag");

    // Initialise stars
    std::mt19937 rng(0);
    std::normal_distribution<float> starPos(0.0f, 1.0f);
    std::normal_distribution<float> starSize(1.5f, 0.75f);

    for (int i = 0; i < STAR_COUNT; i++)
    {
        float x = starPos(rng);
        float y = starPos(rng);
        float z = starPos(rng);
        float normalize = (1 / glm::length(glm::vec3(x, y, z))) * STAR_RADIUS;
        stars.push_back(x * normalize);
        stars.push_back(y * normalize);
        stars.push_back(z * normalize);
        stars.push_back(starSize(rng));
        stars.push_back((rand() / (float)RAND_MAX) * 6.28318530717f); // twinkle offset
    }

    glGenVertexArrays(1, &starsVAO);
    glGenBuffers(1, &starsVBO);

    glBindVertexArray(starsVAO);

    glBindBuffer(GL_ARRAY_BUFFER, starsVBO);
    glBufferData(GL_ARRAY_BUFFER, stars.size() * sizeof(float), stars.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    starShader = new Shader("shaders/starShader.vert", "shaders/starShader.frag");
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Initialise bodies
    Body earth(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 10e10f, 1.0f);
    system.push_back(earth);

    Body moon(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.8f, 0.8f, 0.8f), 8e10f, 0.5f);
    system.push_back(moon);

	return 0;
}

void Renderer::drop()
{
    glDeleteBuffers(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    delete shader;
    shader = nullptr;
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
            float distance = glm::length(direction) + 0.001f;
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
    }
}

void Renderer::renderBody(Body& body)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), body.position); // transforms
    model = glm::scale(model, glm::vec3(body.radius)); // scales
    
    shader->setVec3("colour", body.colour);
    shader->setMat4("model", model);
    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::renderStars()
{
    glBindVertexArray(starsVAO);

    glDrawArrays(GL_POINTS, 0, stars.size());
}

void Renderer::processRendering()
{// the game loop
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    oldTime = currentTime;
    elapsedTime += deltaTime;

    key_callback(window, cam, deltaTime);

    //render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment for wireframe rendering

    glm::mat4 projection = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 model = glm::mat4(0.0f);

    starShader->use();
    starShader->setFloat("time", currentTime);
    starShader->setMat4("projection", projection);
    starShader->setMat4("view", glm::mat4(glm::mat3(view)));
    renderStars();

    shader->use();
    
    shader->setInt("tex", 0);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(sphereVAO);

    while (elapsedTime >= TIMESTEP)
    {
        processPhysics();
        elapsedTime -= TIMESTEP;
    }

    for (auto& b : system)
    {
        renderBody(b);
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