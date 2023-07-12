#pragma once

// LIBS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

#include <GLFW/glfw3.h>

namespace coral_3d
{
	struct KeyBindings
	{
		int move_forward = GLFW_KEY_W;
		int move_backward = GLFW_KEY_S;
		int move_left = GLFW_KEY_A;
		int move_right = GLFW_KEY_D;
		int move_up = GLFW_KEY_E;
		int move_down = GLFW_KEY_Q;
		int sprint = GLFW_KEY_LEFT_SHIFT;
	};

	class coral_camera final
	{
	public:
		coral_camera(glm::vec3 position, float movement_speed = 3.f, float mouse_sensitivity = 0.1f);
		~coral_camera() = default;

		void set_ortographic_projection(
			float left, float right, float top, float bottom, float near, float far);
		void set_perspective_projection(float fovy, float aspect, float near, float far);

		void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void set_view_yxz(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& get_projection() const { return projection_matrix_; }
		const glm::mat4& get_view() const { return view_matrix_; }
		const glm::vec3& get_position() const { return position_; }

		void update_input(GLFWwindow* pWindow, float dt);

	private:
		void update_camera_vectors();

		KeyBindings keys_{};

		// Camera frame
		glm::vec3 position_{};
		glm::vec3 forward_{};
		glm::vec3 right_{};
		glm::vec3 up_{};
		glm::vec3 world_up_{0.f, 1.f, 0.f};
		
		// Mouse
		float last_mouse_x_{};
		float last_mouse_y_{};

		// Rotation
		float yaw_{0.f};
		float pitch_{0.f};

		float movement_speed_{ 5.f };
		float sprint_movement_speed_{ 16.f };
		float mouse_sensitivity_{ 1.5f };

		glm::mat4 projection_matrix_{ 1.f };
		glm::mat4 view_matrix_{0.1f};
	};
}