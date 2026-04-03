#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "models/Sphere.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <string>

// constants
const int INIT_WIDTH = 1470; // initial width of window
const int INIT_HEIGHT = 956; // initial height of window
const int SPHERE_SECTORS = 36;
const int SPHERE_STACKS = 18;

const int STAR_COUNT = 500; // number of stars
const float STAR_RADIUS = 50.0f; // radius of star dome
const float DT = 0.0167f;
int timeScale = 1;

float universeTime = 0.0f;
int universeSeconds = 0;
float G = 0.002753f; // gravitational constant
bool emissivesOn = true;

float xLast = (float)INIT_WIDTH  / 2.0f;
float yLast = (float)INIT_HEIGHT / 2.0f;
bool isRightMouseDown = false;
bool isLeftMouseDown = false;
bool firstMouse = true;

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    r->updateWindowSize(width, height);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    r->updateFramebufferSize(width, height);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            isRightMouseDown = true;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE)
        {
            isRightMouseDown = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (ImGui::GetIO().WantCaptureMouse) return;

        switch (r->currentState)
        {
            case Renderer::EDITING:
                if (action == GLFW_PRESS)
                {
                    switch(r->currentEditorState)
                    {
                        case Renderer::EDITING_POSITION:
                            r->currentEditorState = Renderer::EDITING_VELOCITY;
                            break;

                        default:
                            break;
                    }
                }
                else if (action == GLFW_RELEASE)
                {
                    switch(r->currentEditorState)
                    {
                        case Renderer::EDITING_POSITION:
                            break;

                        case Renderer::EDITING_VELOCITY:
                            r->currentEditorState = Renderer::CREATED;
                            break;

                        case Renderer::CREATED:
                            r->currentEditorState = Renderer::EDITING_POSITION;
                            break;
                    }
                }
                
                break;

            case Renderer::NORMAL:
                if (action == GLFW_RELEASE)
                {
                    r->selectBody();
                }
                break;
        }
    }
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
        return;
    }

    // get displacement from previous frame
    float xOffset = xPos - xLast;
    float yOffset = yLast - yPos;
    xLast = xPos;
    yLast = yPos;

    if (!isRightMouseDown) return;
    r->handleCameraMovement(xOffset, yOffset);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    Renderer* r = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    yOffset = static_cast<float>(yOffset);

    if (!ImGui::GetIO().WantCaptureMouse)
        r->handleCameraZoom(yOffset);
}

void keyCallback(GLFWwindow* window, Camera* cam, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
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
}

Renderer::Renderer()
:   window(nullptr),
    width(INIT_WIDTH),
    height(INIT_HEIGHT),
    shader(nullptr),
    starShader(nullptr),
    postProcessingShader(nullptr),
    blurShader(nullptr),
    hoveredBody(0),
    selectedBody(0),
    nextID(1),
    cam(nullptr),
    baseSphere(1.0f, SPHERE_SECTORS, SPHERE_STACKS),
    currentTime(0.0f),
    oldTime(0.0f),
    deltaTime(0.0f),
    elapsedTime(0.0f),
    currentState(NORMAL),
    currentEditorState(EDITING_POSITION)
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    glfwGetWindowSize(window, &width, &height);
    updateFramebufferSize(width, height);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    updateFramebufferSize(fbWidth, fbHeight);

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

    // Initialise path
    glGenVertexArrays(1, &pathVAO);
    glGenBuffers(1, &pathVBO);
    pathShader = new Shader("shaders/path.vert", "shaders/path.frag");

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

    stars.reserve(STAR_COUNT * 5);
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

    /*
    auto blue = std::make_unique<Body>(
        "Aecicon",
        glm::vec3(-2.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        50,
        0.5f,
        false
    );
    addBody(std::move(blue));

    auto green = std::make_unique<Body>(
        "Dravent", 
        glm::vec3(2.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 0.0f, 1.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f), 
        50, 
        0.5f, 
        false
    );
    addBody(std::move(green));

    auto red = std::make_unique<Body>(
        "Taarmin", 
        glm::vec3(0.0f, 0.0f, 3.46f), 
        glm::vec3(1.0f, 0.0f, 0.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f), 
        50, 
        0.5f, 
        false
    );
    addBody(std::move(red));

    auto white = std::make_unique<Body>(
        "Reiclam", 
        glm::vec3(0.0f, 0.0f, -3.46f), 
        glm::vec3(-1.0f, 0.0f, 0.0f), 
        glm::vec3(1.0f, 1.0f, 1.0f), 
        50, 
        0.5f, 
        false
    );
    addBody(std::move(white));
    */

    auto Sun = std::make_unique<Body>(
        "Sun",
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        333000.0f,
        1.2f,
        true
    );

    addBody(std::move(Sun));

    auto Mercury = std::make_unique<Body>(
        "Mercury",
        glm::vec3(6.975f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 12.45f),
        glm::vec3(0.8f, 0.8f, 0.8f),
        0.055f,
        0.2f,
        false
    );

    addBody(std::move(Mercury));

    auto Venus = std::make_unique<Body>(
        "Venus",
        glm::vec3(10.79f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 9.198f),
        glm::vec3(0.9f, 0.8f, 0.6f),
        0.815f,
        0.38f,
        false
    );

    addBody(std::move(Venus));

    auto Earth = std::make_unique<Body>(
        "Earth",
        glm::vec3(14.9f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 7.871f),
        glm::vec3(0.0f, 0.2f, 1.0f),
        1.0f,
        0.4f,
        false
    );

    addBody(std::move(Earth));

    auto Mars = std::make_unique<Body>(
        "Mars",
        glm::vec3(22.74f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 6.24f),
        glm::vec3(1.0f, 0.1f, 0.0f),
        0.107f,
        0.3f,
        false
    );

    addBody(std::move(Mars));

    auto Jupiter = std::make_unique<Body>(
        "Jupiter",
        glm::vec3(77.79f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 3.40f),
        glm::vec3(0.9f, 0.7f, 0.5f),
        317.8f,
        0.9f,
        false
    );

    addBody(std::move(Jupiter));

    auto Saturn = std::make_unique<Body>(
        "Saturn",
        glm::vec3(143.30f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 2.51f),
        glm::vec3(0.9f, 0.8f, 0.6f),
        95.2f,
        0.8f,
        false
    );

    addBody(std::move(Saturn));

    auto Uranus = std::make_unique<Body>(
        "Uranus",
        glm::vec3(287.23f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.76f),
        glm::vec3(0.6f, 0.8f, 0.9f),
        14.5f,
        0.6f,
        false
    );

    addBody(std::move(Uranus));

    auto Neptune = std::make_unique<Body>(
        "Neptune",
        glm::vec3(450.30f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.40f),
        glm::vec3(0.3f, 0.5f, 0.9f),
        17.1f,
        0.6f,
        false
    );

    addBody(std::move(Neptune));

    blurShader = new Shader("shaders/postProcessingShader.vert", "shaders/gaussianBlur.frag");
    postProcessingShader = new Shader("shaders/postProcessingShader.vert", "shaders/postProcessingShader.frag");

	return 0;
}

int Renderer::initUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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
    delete starShader;
    starShader = nullptr;
    delete postProcessingShader;
    postProcessingShader = nullptr;
    delete blurShader;
    blurShader = nullptr;
    delete pathShader;
    pathShader = nullptr;
    delete cam;
    cam = nullptr;

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

unsigned int Renderer::pickObject(const Camera::Ray& ray)
{
    Body* closest = nullptr;
    float minT = FLT_MAX;

    for (auto& a : system)
    {
        glm::vec3 direction = ray.origin - a->position;
        float b = 2.0f * glm::dot(direction, ray.direction);
        float c = glm::dot(direction, direction) - (a->radius * a->radius);

        float dsc = b * b - 4 * c;
        if (dsc < 0) continue;

        float t1 = (-b - sqrt(dsc)) / 2;
        float t2 = (-b + sqrt(dsc)) / 2;
        float newMinT;

        if (t1 > 0.001f) newMinT = t1;
        else if (t2 > 0.001f) newMinT = t2;
        else continue;

        if (newMinT < minT) {
            minT = newMinT;
            closest = a.get();
        }
    }

    if (closest)
        return closest->ID;
    else
        return 0;
}

glm::vec3 pickPointOnPlane(const Camera::Ray& ray)
{
    glm::vec3 point(0.0f);

    if (ray.direction.y < 0.0f)
    {
        float t = (-ray.origin.y) / ray.direction.y;
        point = ray.origin + t * ray.direction;
    }

    return point;
}

void Renderer::processLighting()
{
    if (!emissivesOn)
    {
        shader->setInt("emissiveCount", 0);
        return;
    }
    
    shader->setInt("emissiveCount", emissives.size());

    if (emissives.size() > 0)
    {
        for (size_t i = 0; i < emissives.size(); i++)
        {
            Body* b = getBody(emissives[i]);
            std::string index = "emissiveBodies[" + std::to_string(i) + "].";
            shader->setVec3(index + "position", b->position);
            shader->setVec3(index + "colour", b->colour);
            shader->setFloat(index + "constant", 1.0f);
            shader->setFloat(index + "linear", 0.09f);
            shader->setFloat(index + "quadratic", 0.032f);
        }
    }
}

void Renderer::processPhysics()
{
    std::vector<unsigned int> removeBodies;

    for (auto& a : system)
    {
        if (a->getDisplacement(cam->getPosition()) > 800.0f)
        {
            removeBodies.push_back(a->ID);
        }
    }

    for (unsigned int ID : removeBodies)
    {
        removeBody(ID);
    }

    for (auto& a : system)
    {
        glm::vec3 acceleration(0.0f);

        for (auto& b : system)
        {
            if (&a == &b) continue;

            glm::vec3 direction = b->position - a->position;
            float distance = glm::length(direction) + 0.001f;
            float magnitudeAcceleration = (G * b->mass) / (distance * distance);

            acceleration += magnitudeAcceleration * glm::normalize(direction);
        }

        a->acceleration = acceleration;
    }

    for (auto&a : system)
    {
        glm::vec3 temp = a->position;
        a->position = (2.0f * a->position) - (a->previousPosition) + (a->acceleration * DT * DT);
        a->previousPosition = temp;
    }
}

void Renderer::renderBody(Body& body)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), body.position); // transforms
    glm::mat4 outlineModel = glm::scale(model, glm::vec3(body.radius * 1.1));
    model = glm::scale(model, glm::vec3(body.radius)); // scales

    shader->setVec3("viewPos", cam->getPosition());
    shader->setBool("emissive", body.emissive);

    if (body.ID == selectedBody) {
        glDisable(GL_DEPTH_TEST);
        shader->setBool("outline", true);
        shader->setFloat("time", currentTime);
        shader->setMat4("model", outlineModel);
        glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
        glEnable(GL_DEPTH_TEST);
    }
    
    shader->setBool("outline", false);
    shader->setVec3("colour", body.colour);
    shader->setMat4("model", model);
    
    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::renderEditingBody()
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), editingBody->position); // transforms
    model = glm::scale(model, glm::vec3(editingBody->radius)); // scales

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader->setVec3("viewPos", cam->getPosition());
    shader->setBool("emissive", false);
    shader->setBool("outline", false);
    shader->setVec3("colour", editingBody->colour);
    shader->setMat4("model", model);

    glDrawElements(GL_TRIANGLES, baseSphere.getIndices().size(), GL_UNSIGNED_INT, (void*)0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::renderStars()
{
    glBindVertexArray(starsVAO);

    glEnable(GL_PROGRAM_POINT_SIZE);
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

void Renderer::processPath()
{
    std::vector<Body> simulation;
    simulation.reserve(system.size());
    predictedPath.clear();

    for (const auto& a : system)
    {
        simulation.push_back(*a);
    }

    simulation.push_back(*editingBody);
    const int STEPS = 200;
    const float RENDER_INTERVAL = 0.02f;
    float accumulator = 0.0f;

    for (int i = 0; i < STEPS; i++)
    {
        for (auto& a : simulation)
        {
            glm::vec3 acceleration(0.0f);

            for (auto& b : simulation)
            {
                if (&a == &b) continue;

                glm::vec3 direction = b.position - a.position;
                float distance = glm::length(direction) + 0.001f;
                float magnitudeAcceleration = (G * b.mass) / (distance * distance);

                acceleration += magnitudeAcceleration * glm::normalize(direction);
            }

            a.acceleration = acceleration;
        }

        for (auto&a : simulation)
        {
            glm::vec3 temp = a.position;
            a.position = (2.0f * a.position) - (a.previousPosition) + (a.acceleration * DT * DT);
            a.previousPosition = temp;
        }

        accumulator += DT;
        if (accumulator >= RENDER_INTERVAL)
        {
            predictedPath.push_back(simulation.back().position);
            accumulator = 0.0f;
        }
    }
}

void Renderer::renderPath()
{
    if (predictedPath.size() == 0) return;

    glBindVertexArray(pathVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pathVBO);
    glBufferData(GL_ARRAY_BUFFER, predictedPath.size() * sizeof(glm::vec3), predictedPath.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, predictedPath.size());
}

void Renderer::processRendering()
{// the game loop
    currentTime = glfwGetTime();
    deltaTime = currentTime - oldTime;
    if (deltaTime > 0.25)
            deltaTime = 0.25;
    oldTime = currentTime;

    universeTime += deltaTime * timeScale;
    universeSeconds = (int)floor(universeTime);

    elapsedTime += deltaTime * timeScale;

    keyCallback(window, cam, deltaTime);

    if (glfwWindowShouldClose(window))
        return;

    // first pass
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    switch (currentState)
    {
        case NORMAL:
            renderNormal();
            break;
        case EDITING:
            renderEditing();
            break;
    }
      
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

    // BOTH

    //imgui // NORMAL
    ImGui::Begin("Tools");
    if (ImGui::Button("Spawn new body"))
    {
        timeScale = 0.0f;
        currentState = EDITING;
    }
    ImGui::SliderInt("Timescale", &timeScale, 0, 100);
    ImGui::InputFloat("G", &G);
    ImGui::Checkbox("Emissives on?", &emissivesOn);
    cam->showButtons();
    ImGui::End();

    ImGui::Begin("User");
    glm::vec3 camPos = cam->getPosition();
    ImGui::SeparatorText("Position:");
    ImGui::Text("x:%.3f, \ny:%.3f, \nz:%.3f\n",
                camPos.x, camPos.y, camPos.z);

    ImGui::SeparatorText("Direction:");
    ImGui::Text("x:%.1f degrees, \ny:%.1f degrees",
                cam->getYaw(), cam->getPitch());

    ImGui::SeparatorText("Zoom:");
    ImGui::Text("FOV:%.0f",
                cam->getFOV());
    
    if (hoveredBody != 0)
    {
        ImGui::SeparatorText("Hovered Body:");
        ImGui::Text("%s", getBody(hoveredBody)->name.c_str());
    }
    if (selectedBody != 0)
    {
        ImGui::SeparatorText("Selected Body:");
        ImGui::Text("%s", getBody(selectedBody)->name.c_str());
    }

    ImGui::SeparatorText("Current System:");
    ImGui::Text("Body count: %zu", system.size());
    ImGui::Text("Emissive body count: %zu", emissives.size());

    ImGui::SeparatorText("Time Passed:");
    ImGui::Text("%i years and %i months", universeSeconds / 12, universeSeconds % 12);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwPollEvents();
    glfwSwapBuffers(window);
}

void Renderer::renderNormal()
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    hoveredBody = pickObject(cam->getRay(mouseX, mouseY, width, height));

    // BOTH
    glm::mat4 projection = cam->getProjectionMatrix(fbWidth, fbHeight);
    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 model = glm::mat4(0.0f);

    // BOTH (do this before hover)
    starShader->use();
    starShader->setFloat("time", currentTime);
    starShader->setMat4("projection", projection);
    starShader->setMat4("view", glm::mat4(glm::mat3(view)));
    renderStars();

    // BOTH
    shader->use();
    
    shader->setInt("tex", 0);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(sphereVAO);

    if (timeScale > 0.0f)
    {
        while (elapsedTime >= DT)
        {
            processPhysics();
            elapsedTime -= DT;
        }
    }
    
    processLighting();

    for (auto& b : system)
    {
        renderBody(*b);
    }

    processBloom(5);

    if (selectedBody != 0)
    {
        Body* selected = getBody(selectedBody);
        ImGui::Begin("Selected Body:");

        ImGui::Text("%s", selected->name.c_str());

        ImGui::SeparatorText("Position:");
        ImGui::Text("x:%.3f, \ny:%.3f, \nz:%.3f\n",
                    selected->position.x, selected->position.y, selected->position.z);
        
        ImGui::SeparatorText("Velocity:");
        ImGui::Text("%.3f units/second\n", selected->getVelocity());

        ImGui::SeparatorText("Acceleration:");
        ImGui::Text("%.3f units/second squared\n", selected->getAcceleration());
        
        ImGui::SeparatorText("Properties:");
        //ImGui::Text("Emissive:"); to do: moving bodies around systems
        //ImGui::Checkbox("Emissive", &selectedBody->emissive);
        ImGui::ColorEdit3("Colour", &selected->colour.x);
        ImGui::InputFloat("Mass", &selected->mass);
        ImGui::InputFloat("Radius", &selected->radius);
        ImGui::Text("Emissive: %s", selected->emissive ? "yes" : "no");
        ImGui::Text("ID: %u", selected->ID);
        if (ImGui::Button("Delete"))
        {
            removeBody(selectedBody);
        }

        ImGui::End();
    }
}

void Renderer::renderEditing()
{
    if (!editingBody)
    {
        hoveredBody = 0;
        selectedBody = 0;
        currentEditorState = EDITING_POSITION;
        
        editingBody.emplace(
            "New Body",
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            100.0f,
            1.0f,
            false
        );
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    switch (currentEditorState)
    {
        case EDITING_POSITION:
            editingBody->position = pickPointOnPlane(cam->getRay(mouseX, mouseY, width, height));
            break;

        case EDITING_VELOCITY:
        {
            glm::vec3 distance = editingBody->position - pickPointOnPlane(cam->getRay(mouseX, mouseY, width, height));
            if (glm::length(distance) > 1e-6f)
                editingBody->setVelocity(distance, DT);
            else
                editingBody->setVelocity(glm::vec3(0.0f), DT);
            break;
        }

        case CREATED:
            break;
    }

    // BOTH
    glm::mat4 projection = cam->getProjectionMatrix(fbWidth, fbHeight);
    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 model = glm::mat4(0.0f);

    // BOTH
    starShader->use();
    starShader->setFloat("time", currentTime);
    starShader->setMat4("projection", projection);
    starShader->setMat4("view", glm::mat4(glm::mat3(view)));
    renderStars();

    // Render path
    processPath();

    pathShader->use();
    pathShader->setMat4("projection", projection);
    pathShader->setMat4("view", view);
    renderPath();

    // BOTH
    shader->use();
    
    shader->setInt("tex", 0);
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    glBindVertexArray(sphereVAO);

    if (timeScale > 0.0f)
    {
        while (elapsedTime >= DT)
        {
            processPhysics();
            elapsedTime -= DT;
        }
    }
    
    processLighting();
    
    // Render editing body
    renderEditingBody();

    // Render all other bodies
    for (auto& b : system)
    {
        renderBody(*b);
    }

    processBloom(10);

    ImGui::Begin("New Body");
    ImGui::SeparatorText("Position:");
    ImGui::Text("x:%.3f, \ny:%.3f, \nz:%.3f\n",
                 editingBody->position.x, editingBody->position.y, editingBody->position.z);

    ImGui::SeparatorText("Velocity:");
    ImGui::Text("%.3f units/second\n", editingBody->getVelocity());

    ImGui::SeparatorText("Properties:");
    ImGui::InputText("Name", &editingBody->name);
    ImGui::ColorEdit3("Colour", &editingBody->colour.x);
    ImGui::InputFloat("Mass", &editingBody->mass);
    ImGui::InputFloat("Radius", &editingBody->radius);
    ImGui::Checkbox("Emissive", &editingBody->emissive);

    if (currentEditorState == CREATED)
    {
        if (ImGui::Button("Create"))
        {
            addBody(std::make_unique<Body>(*editingBody));
            editingBody.reset();
            timeScale = 1.0f;
            currentState = NORMAL;
        } // works only if position is confirmed
    }

    if (ImGui::Button("Cancel"))
    {
        editingBody.reset();
        timeScale = 1.0f;
        currentState = NORMAL;
    } // set state to normal, delete pointer
    ImGui::End();
}

void Renderer::addBody(std::unique_ptr<Body> body)
{
    body->ID = nextID;
    nextID++;

    if (body->emissive)
    {
        emissives.push_back(body.get()->ID);
    }

    system.push_back(std::move(body));
}

void Renderer::removeBody(unsigned int ID)
{
    system.erase(std::remove_if(system.begin(), system.end(), 
        [&](const std::unique_ptr<Body>& b)
        {
            return b->ID == ID;
        }
        ), system.end());

    emissives.erase(std::remove(emissives.begin(), emissives.end(), ID), emissives.end());

    if (hoveredBody == ID) hoveredBody = 0;
    if (selectedBody == ID) selectedBody = 0;
}

Body* Renderer::getBody(unsigned int ID)
{
    if (ID != 0)
    {
        for (auto& a : system)
        {
            if (a->ID == ID)
                return a.get();
        }
    }
    return nullptr;
}

GLFWwindow* Renderer::getWindow()
{
	return window;
}

void Renderer::handleCameraMovement(float xOffset, float yOffset)
{
    cam->handleMouseMovement(xOffset, yOffset);
}

void Renderer::handleCameraZoom(float yOffset)
{
    cam->handleScroll(yOffset);
}

void Renderer::selectBody()
{
    selectedBody = hoveredBody;
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

    glViewport(0, 0, fbWidth, fbHeight);

    glBindTexture(GL_TEXTURE_2D, colourBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
        fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, lightBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
        fbWidth, fbHeight, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
        fbWidth, fbHeight);

    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, pingpongTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
            fbWidth, fbHeight, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}