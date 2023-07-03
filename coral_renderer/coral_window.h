#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace coral_3d
{
	class coral_window final
	{
	public:
		coral_window(int width, int height, const std::string& name);
		~coral_window();

		coral_window(const coral_window&) = delete;
		coral_window& operator=(const coral_window&) = delete;

		bool should_close() { return glfwWindowShouldClose(pWindow_); }

	private:
		const int cWidth;
		const int cHeight;

		std::string window_name_;
		GLFWwindow* pWindow_;

		void init_window();
	};
}