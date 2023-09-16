#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm.hpp>

#include <deque>
#include <functional>

struct AllocatedBuffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct AllocatedImage
{
	VkImage image;
	VmaAllocation allocation;
};

struct DeletionQueue
{
	std::deque <std::function<void()>> deletors;

	void flush()
	{
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
			(*it)();

		deletors.clear();
	}
};

struct GlobalUBO
{
    // MATRICES
    glm::mat4 view{1.f};
    glm::mat4 view_inverse{1.f};
    glm::mat4 view_projection{1.f};

    // GLOBAL LIGHT
    glm::vec4 global_light_direction{ glm::normalize(glm::vec4{ -0.577f, -0.577f, -0.577f, 0.f})}; // w is ignored
    glm::vec4 ambient_light_color{1.f, .82f, .863f, .01f}; // w is intensity
};

struct PushConstant
{
    glm::mat4 node_transform;
};