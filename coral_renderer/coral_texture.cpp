#include "coral_texture.h"

// STD
#include <iostream>

// LIBS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vk_initializers.h"
#include "coral_device.h"
#include "coral_buffer.h"

bool vkutil::load_image_from_file(coral_device& device, const std::string& file_name, AllocatedImage& out_image)
{
	int tex_width, tex_height, tex_channels;

	stbi_uc* img{ stbi_load(file_name.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha)};

	if (!img)
	{
		std::cout << "Failed to load texture file " << file_name << std::endl;
		return false;
	}

	void* pPixels{ img };
	VkDeviceSize img_size{ static_cast<uint64_t>(tex_width * tex_height * 4) };

	// This format must match with the format loaded from stb_image
	VkFormat img_format{ VK_FORMAT_R8G8B8A8_SRGB };

	// Holds texture data to upload to GPU
	coral_buffer staging_buffer
	{
		device,
		img_size,
		1,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
	};

	// Copy data to buffer
	staging_buffer.map();
	staging_buffer.write_to_buffer(pPixels);

	// Free the pixels
	stbi_image_free(img);

	VkExtent3D img_extent;
	img_extent.width = static_cast<uint32_t>(tex_width);
	img_extent.height = static_cast<uint32_t>(tex_height);
	img_extent.depth = 1;

	VkImageCreateInfo image_ci{ vkinit::image_ci(img_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, img_extent) };

	AllocatedImage new_img;

	VmaAllocationCreateInfo image_ai{};
	image_ai.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(device.allocator(), &image_ci, &image_ai,
		&new_img.image, &new_img.allocation, nullptr);

	// Transition layout using barries and copy buffer to image
	device.transition_image_layout(new_img.image, img_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	device.copy_buffer_to_image(staging_buffer.get_buffer(), new_img, img_extent.width, img_extent.height, 1);
	device.transition_image_layout(new_img.image, img_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	std::cout << "Texture loaded successfully " << file_name << std::endl;

	out_image = new_img;
	return true;
}
