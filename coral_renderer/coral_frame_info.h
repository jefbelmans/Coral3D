#pragma once

#include "coral_camera.h"

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
	};
}