#include "Renderer.hpp"
#include "Body.hpp"
#include "GLFW/glfw3.h"


// constants
const int WIDTH = 600;
const int HEIGHT = 600;
const float ASPECT_RATIO = (float)WIDTH / (float)HEIGHT;
const float FOV = 55.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

Renderer::Renderer()
: window(nullptr)
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
    // render sphere with radius of body
}

void Renderer::processRendering(std::vector<Body>& toRender)
{
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