#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

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