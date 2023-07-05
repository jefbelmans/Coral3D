#pragma once

#include "coral_gameobject.h"
#include "coral_window.h"

namespace coral_3d
{
	class KeyboardController final
	{
	public:
		struct KeyMappings
		{
			int move_left = GLFW_KEY_A;
			int move_right = GLFW_KEY_D;
			int move_forward = GLFW_KEY_W;
			int move_backward = GLFW_KEY_S;
			int move_up = GLFW_KEY_E;
			int move_down = GLFW_KEY_Q;
			int look_left = GLFW_KEY_LEFT;
			int look_right = GLFW_KEY_RIGHT;
			int look_up = GLFW_KEY_UP;
			int look_down = GLFW_KEY_DOWN;
		};

		void move_in_plane_xz(GLFWwindow* pWindow, float dt, coral_gameobject& gameobject);

		KeyMappings keys{};
		float move_speed{ 3.f };
		float look_speed{ 1.5f };
	};
}