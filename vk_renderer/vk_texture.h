#pragma once

#include "vk_types.h"
#include "vk_engine.h"

using namespace Coral3D;
namespace vkutil
{
	bool load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& outImage);
}