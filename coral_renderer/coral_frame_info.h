#pragma once

#include "coral_camera.h"
#include "coral_gameobject.h"

// LIBS
#include <vulkan/vulkan.h>

namespace coral_3d
{
	struct FrameInfo
	{
		int frame_index{};
		float frame_tine{};
		VkCommandBuffer command_buffer{};
		coral_camera& camera;
		VkDescriptorSet global_descriptor_set;
		coral_gameobject::Map& gameobjects;
	};
}