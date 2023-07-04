#include "coral_camera.h"

#include <gtc/matrix_transform.hpp>

using namespace coral_3d;

void coral_camera::set_ortographic_projection(float left, float right, float top, float bottom, float near, float far)
{
	projection_matrix_ = glm::orthoLH(left, right, bottom, top, near, far);
}

void coral_camera::set_perspective_projection(float fovy, float aspect, float near, float far) 
{
	projection_matrix_ = glm::perspectiveLH(fovy, aspect, near, far);
}

void coral_camera::set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	direction.x *= -1.f;
	view_matrix_ = glm::lookAtLH(position, direction, up);
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
