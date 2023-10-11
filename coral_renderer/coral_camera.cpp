#include "coral_camera.h"

#include <gtc/matrix_transform.hpp>
#include <algorithm>
#include "imgui.h"

using namespace coral_3d;

coral_camera::coral_camera(glm::vec3 position, float movement_speed, float mouse_sensitivity)
    : position_{ position }
    , movement_speed_{ movement_speed }
	, mouse_sensitivity_{ mouse_sensitivity }
{
}

void coral_camera::set_orthographic_projection(float left, float right, float top, float bottom, float near, float far)
{
	projection_matrix_ = glm::ortho(left, right, bottom, top, near, far);
}

void coral_camera::set_perspective_projection(float fovy, float aspect, float near, float far) 
{
	projection_matrix_ = glm::perspective(fovy, aspect, near, far);
}

void coral_camera::set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	view_matrix_ = glm::lookAt(position, position + direction, up);
}

void coral_camera::update_input(GLFWwindow* pWindow, float dt)
{
    if(ImGui::IsWindowFocused()) return;
	// Mouse input
	double mouse_x, mouse_y;
	glfwGetCursorPos(pWindow, &mouse_x, &mouse_y);

	float mouse_dx = static_cast<float>(mouse_x - last_mouse_x_) * mouse_sensitivity_;
	float mouse_dy = static_cast<float>(last_mouse_y_ - mouse_y) * mouse_sensitivity_;
	last_mouse_x_ = static_cast<float>(mouse_x);
	last_mouse_y_ = static_cast<float>(mouse_y);

	yaw_ -= mouse_dx;
	pitch_ += mouse_dy;
	pitch_ = std::clamp(pitch_, -89.f, 89.f);

	update_camera_vectors();
	set_view_direction(position_, forward_, up_);

	// Keyboard input
	const float velocity{ glfwGetKey(pWindow, keys_.sprint) == GLFW_PRESS ? sprint_movement_speed_ * dt : movement_speed_ * dt };
	if(glfwGetKey(pWindow, keys_.move_forward) == GLFW_PRESS) position_ += forward_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_backward) == GLFW_PRESS) position_ -= forward_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_left) == GLFW_PRESS) position_ -= right_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_right) == GLFW_PRESS) position_ += right_ * velocity;

	if(glfwGetKey(pWindow, keys_.move_up) == GLFW_PRESS) position_ += world_up_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_down) == GLFW_PRESS) position_ -= world_up_ * velocity;
}

void coral_camera::update_camera_vectors()
{
    float yaw_rad{glm::radians(yaw_)};
    float pitch_rad{glm::radians(pitch_)};

	glm::vec3 forward
	{
		glm::cos(yaw_rad) * glm::cos(pitch_rad),
		glm::sin(pitch_rad),
		glm::sin(yaw_rad) * glm::cos(pitch_rad)
	};
	
	forward_ = glm::normalize(forward);
	right_ = glm::normalize(glm::cross(world_up_, forward_));
	up_ = glm::normalize(glm::cross(right_, forward_));
}