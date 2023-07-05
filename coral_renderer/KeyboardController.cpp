#include "KeyboardController.h"
#include <limits>

using namespace coral_3d;

void KeyboardController::move_in_plane_xz(GLFWwindow* pWindow, float dt, coral_gameobject& gameobject)
{
	glm::vec3 rotate{0.f};
	if (glfwGetKey(pWindow, keys.look_right) == GLFW_PRESS) rotate.y += 1.f;
	if (glfwGetKey(pWindow, keys.look_left) == GLFW_PRESS) rotate.y -= 1.f;
	if (glfwGetKey(pWindow, keys.look_up) == GLFW_PRESS) rotate.x += 1.f;
	if (glfwGetKey(pWindow, keys.look_up) == GLFW_PRESS) rotate.x -= 1.f;
		
	if(glm::length(rotate) > std::numeric_limits<float>::epsilon())
		gameobject.transform_.rotation += look_speed * dt * glm::normalize(rotate);

	gameobject.transform_.rotation.x = glm::clamp(gameobject.transform_.rotation.x, -1.5f, 1.5f);
	gameobject.transform_.rotation.y = glm::mod(gameobject.transform_.rotation.y, glm::two_pi<float>());

	float yaw = gameobject.transform_.rotation.y;
	const glm::vec3 forward_dir{sin(yaw), 0.f, cos(yaw)};
	const glm::vec3 right_dir{forward_dir.z, 0.f, -forward_dir.x};
	const glm::vec3 up_dir{0.f, -1.f, 0.f};

	glm::vec3 move_dir{0.f};
	if (glfwGetKey(pWindow, keys.move_forward) == GLFW_PRESS) move_dir += forward_dir;
	if (glfwGetKey(pWindow, keys.move_backward) == GLFW_PRESS)  move_dir -= forward_dir;
	if (glfwGetKey(pWindow, keys.move_right) == GLFW_PRESS) move_dir += right_dir;
	if (glfwGetKey(pWindow, keys.move_left) == GLFW_PRESS) move_dir -= right_dir;
	if (glfwGetKey(pWindow, keys.move_up) == GLFW_PRESS) move_dir += up_dir;
	if (glfwGetKey(pWindow, keys.move_down) == GLFW_PRESS) move_dir -= up_dir;

	if (glm::length(move_dir) > std::numeric_limits<float>::epsilon())
		gameobject.transform_.translation += move_speed * dt * glm::normalize(move_dir);
}
