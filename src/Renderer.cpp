#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/detail/qualifier.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "models/Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <random>
#include <string>

// constants
const int INIT_WIDTH = 800;
const int INIT_HEIGHT = 600;
const float FOV = 55.0f;
const int SPHERE_SECTORS = 36;
const int SPHERE_STACKS = 18;
const float G = 6.674e-11;
const float distScale = 1e11;
const float TIMESTEP = 0.015f;
const int STAR_COUNT = 500;
const float STAR_RADIUS = 50.0f;

float xLast = (float)INIT_WIDTH  / 2.0f;
float yLast = (float)INIT_HEIGHT / 2.0f;
bool firstMouse = true;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    r->updateWindowSize(width, height);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    r->updateFramebufferSize(fbWidth, fbHeight);

    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
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

void keyCallback(GLFWwindow* window, Camera* cam, float deltaTime)
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
: window(nullptr), width(INIT_WIDTH), height(INIT_HEIGHT), shader(nullptr), cam(nullptr), baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS), currentTime(0.0f), oldTime(0.0f), deltaTime(0.0f), elapsedTime(0.0f)
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
    window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "Gravity Simulator", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR: failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "ERROR: failed to load GLAD" << std::endl;
        return -1;
    }

    // Callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

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

    // Initialise framebuffer
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &colourBufferTexture);
    glBindTexture(GL_TEXTURE_2D, colourBufferTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //

    glGenTextures(1, &lightBufferTexture);
    glBindTexture(GL_TEXTURE_2D, lightBufferTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colourBufferTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightBufferTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //

    unsigned int attachments[2] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1
    };

    glDrawBuffers(2, attachments);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: framebuffer incomplete" << std::endl;
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (int i = 0; i < 2; i++)
    {
        glGenFramebuffers(1, &pingpongFBO[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

        glGenTextures(1, &pingpongTexture[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTexture[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTexture[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "ERROR: ping pong framebuffer incomplete" << std::endl;
            return -1;
        }
    }

    // Framebuffer quad VAO
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
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
    Body earth(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 10e10f, 0.5f, false);
    system.push_back(earth);

    Body sun(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 8e10f, 1.0f, true);
    system.push_back(sun);

    blurShader = new Shader("shaders/postProcessingShader.vert", "shaders/gaussianBlur.frag");
    postProcessingShader = new Shader("shaders/postProcessingShader.vert", "shaders/postProcessingShader.frag");
    glEnable(GL_DEPTH_TEST);
    
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);

    std::cout << "Window: " << w << "x" << h << "\n";
    std::cout << "Framebuffer: " << fbw << "x" << fbh << "\n";
	return 0;
}

void Renderer::drop()
{
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    glDeleteVertexArrays(1, &starsVAO);
    glDeleteBuffers(1, &starsVBO);

    glDeleteFramebuffers(1, &FBO);

    delete shader;
    shader = nullptr;
    delete cam;
    cam = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::processLighting()
{
    // get emissive bodies
    std::vector<Body> emissives;
    for (auto& a : system)
    {
        if (a.emissive) emissives.push_back(a);
    }

    // set properties
    shader->setInt("emissiveCount", emissives.size());

    if (emissives.size() > 0)
    {
        for (size_t i = 0; i < emissives.size(); i++)
        {
            std::string index = "emissiveBodies[" + std::to_string(i) + "].";
            shader->setVec3(index + "position", emissives[i].position);
            shader->setVec3(index + "colour", emissives[i].colour);
            shader->setFloat(index + "constant", 1.0f);
            shader->setFloat(index + "linear", 0.045f);
            shader->setFloat(index + "quadratic", 0.0075f);
        }
    }
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
    shader->setVec3("viewPos", cam->getPosition());
    shader->setMat4("model", model);
    shader->setBool("emissive", body.emissive);
    
    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::renderStars()
{
    glBindVertexArray(starsVAO);

    glDrawArrays(GL_POINTS, 0, stars.size() / 5);
}

void Renderer::processBloom(int passes)
{
    bool horizontal = true;
    bool first = true;

    blurShader->use();
    blurShader->setInt("image", 0);

    for (int i = 0; i < passes; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glActiveTexture(GL_TEXTURE0);

        blurShader->setBool("horizontal", horizontal);

        glBindTexture(GL_TEXTURE_2D, first ? lightBufferTexture : pingpongTexture[!horizontal]);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
        glBindVertexArray(0);

        horizontal = !horizontal;
        if (first) first = false;
    }
}

void Renderer::processRendering()
{// the game loop
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    oldTime = currentTime;
    elapsedTime += deltaTime;

    keyCallback(window, cam, deltaTime);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment for wireframe rendering

    // first pass
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::perspective(glm::radians(FOV), (float)fbWidth / fbHeight, 0.1f, 100.0f);
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
    
    processLighting();

    for (auto& b : system)
    {
        renderBody(b);
    }

    processBloom(10);
      
    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);

    postProcessingShader->use();
    postProcessingShader->setInt("screenTexture", 0);
    postProcessingShader->setInt("bloomTexture", 1);

    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colourBufferTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongTexture[1]);

    glDrawArrays(GL_TRIANGLES, 0, 6); 

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

void Renderer::updateWindowSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void Renderer::updateFramebufferSize(int fbWidth, int fbHeight)
{
    this->fbWidth = fbWidth;
    this->fbHeight = fbHeight;

    // to do: update texture
}