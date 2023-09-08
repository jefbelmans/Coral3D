#include "coral_camera.h"

#include <gtc/matrix_transform.hpp>
#include <algorithm>

using namespace coral_3d;

coral_camera::coral_camera(glm::vec3 position, float movement_speed, float mouse_sensitivity)
	: movement_speed_{ movement_speed }
	, mouse_sensitivity_{ mouse_sensitivity }
	, position_{ position }
{
}

void coral_camera::set_orthographic_projection(float left, float right, float top, float bottom, float near, float far)
{
	projection_matrix_ = glm::orthoLH(left, right, bottom, top, near, far);
}

void coral_camera::set_perspective_projection(float fovy, float aspect, float near, float far) 
{
	projection_matrix_ = glm::perspectiveLH(fovy, aspect, near, far);
}

void coral_camera::set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	view_matrix_ = glm::lookAt(position, position + direction, up);
}

void coral_camera::set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
	target.x *= -1.f;
	view_matrix_ = glm::lookAtLH(position, target, up);
}

void coral_camera::set_view_yxz(glm::vec3 position, glm::vec3 rotation)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{(c1* c3 + s1 * s2 * s3), (c2* s3), (c1* s2* s3 - c3 * s1)};
	const glm::vec3 v{(c3* s1* s2 - c1 * s3), (c2* c3), (c1* c3* s2 + s1 * s3)};
	const glm::vec3 w{(c2* s1), (-s2), (c1* c2)};
	view_matrix_ = glm::mat4{ 1.f };
	view_matrix_[0][0] = u.x;
	view_matrix_[1][0] = u.y;
	view_matrix_[2][0] = u.z;
	view_matrix_[0][1] = v.x;
	view_matrix_[1][1] = v.y;
	view_matrix_[2][1] = v.z;
	view_matrix_[0][2] = w.x;
	view_matrix_[1][2] = w.y;
	view_matrix_[2][2] = w.z;
	view_matrix_[3][0] = -glm::dot(u, position);
	view_matrix_[3][1] = -glm::dot(v, position);
	view_matrix_[3][2] = -glm::dot(w, position);
}

void coral_camera::update_input(GLFWwindow* pWindow, float dt)
{
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
	if(glfwGetKey(pWindow, keys_.move_forward) == GLFW_PRESS) position_ -= forward_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_backward) == GLFW_PRESS) position_ += forward_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_left) == GLFW_PRESS) position_ -= right_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_right) == GLFW_PRESS) position_ += right_ * velocity;

	if(glfwGetKey(pWindow, keys_.move_up) == GLFW_PRESS) position_ += up_ * velocity;
	if(glfwGetKey(pWindow, keys_.move_down) == GLFW_PRESS) position_ -= up_ * velocity;
}

void coral_camera::update_camera_vectors()
{
	glm::vec3 forward
	{
		glm::cos(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_)),
		glm::sin(glm::radians(pitch_)),
		glm::sin(glm::radians(yaw_)) * glm::cos(glm::radians(pitch_))
	};
	
	forward_ = glm::normalize(forward);
	right_ = glm::normalize(glm::cross(forward_, world_up_));
	up_ = glm::normalize(glm::cross(right_, forward_));
}