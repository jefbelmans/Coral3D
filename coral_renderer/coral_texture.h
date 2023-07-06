#pragma once
#include "vk_types.h"
#include "coral_device.h"

using namespace coral_3d;
namespace vkutil
{
	bool load_image_from_file(coral_device& engine, const std::string& file_name, AllocatedImage& out_image);
}
