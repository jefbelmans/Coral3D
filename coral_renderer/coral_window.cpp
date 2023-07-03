#include "coral_window.h"

using namespace coral_3d;

coral_window::coral_window(int width, int height, const std::string& name)
	: cWidth(width)
	, cHeight(height)
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

	pWindow_ = glfwCreateWindow(cWidth, cHeight, window_name_.c_str(), nullptr, nullptr);
}
