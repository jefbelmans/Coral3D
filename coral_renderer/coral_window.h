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

		bool should_close() const { return glfwWindowShouldClose(pWindow_); }
		VkExtent2D get_extent() const { return { static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}; }
		bool was_window_resized() const { return is_framebuffer_resized_; }
		void reset_window_resized() { is_framebuffer_resized_ = false; }
		GLFWwindow* get_glfw_window() const { return pWindow_; }

		void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebuffer_resize_callback(GLFWwindow* pWindow, int width, int height);
		void init_window();

		int width_;
		int height_;
		bool is_framebuffer_resized_{ false };

		std::string window_name_;
		GLFWwindow* pWindow_;
	};
}