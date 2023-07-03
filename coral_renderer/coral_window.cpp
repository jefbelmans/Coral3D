#include "coral_window.h"

#include <stdexcept>

using namespace coral_3d;

coral_window::coral_window(int width, int height, const std::string& name)
	: c_width(width)
	, c_height(height)
	, window_name_(name)
{
	init_window();
}

coral_window::~coral_window()
{
	glfwDestroyWindow(pWindow_);
	glfwTerminate();
}

void coral_window::init_window()
{
	glfwInit();

	// Tell GLFW to not use OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	pWindow_ = glfwCreateWindow(c_width, c_height, window_name_.c_str(), nullptr, nullptr);
}

void coral_window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, pWindow_, nullptr, surface) != VK_SUCCESS)
		throw std::runtime_error(
			"ERROR! coral_window::create_window_surface() >> Failed to create window surface!");
}
