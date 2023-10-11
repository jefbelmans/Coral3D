#include "coral_window.h"
#include "coral_input.h"

#include <stdexcept>

using namespace coral_3d;

coral_window::coral_window(int width, int height, const std::string& name)
	: width_(width)
	, height_(height)
	, window_name_(name)
{
	init_window();
}

coral_window::~coral_window()
{
	glfwDestroyWindow(pWindow_);
	glfwTerminate();
}

void coral_window::framebuffer_resize_callback(GLFWwindow* pWindow, int width, int height)
{
	auto window = reinterpret_cast<coral_window*>(glfwGetWindowUserPointer(pWindow));

	window->is_framebuffer_resized_ = true;
	window->width_ = width;
	window->height_ = height;
}

void coral_window::init_window()
{
	glfwInit();

	// Tell GLFW to not use OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	pWindow_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

	// Set window user pointer
	glfwSetWindowUserPointer(pWindow_, this);
	glfwSetFramebufferSizeCallback(pWindow_, framebuffer_resize_callback);

	// Set mouse input mode
	glfwSetInputMode(pWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize input
    coral_input::initialize(pWindow_);
}

void coral_window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, pWindow_, nullptr, surface) != VK_SUCCESS)
		throw std::runtime_error(
			"ERROR! coral_window::create_window_surface() >> Failed to create window surface!");
}