#include "coral_texture.h"

// STD
#include <iostream>

// LIBS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vk_initializers.h"
#include "coral_device.h"

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
	AllocatedBuffer staging_buffer{ device.create_buffer(img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY) };

	// Copy data to buffer
	void* data;
	vmaMapMemory(device.allocator(), staging_buffer.allocation, &data);
	memcpy(data, pPixels, static_cast<size_t>(img_size));
	vmaUnmapMemory(device.allocator(), staging_buffer.allocation);

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

	device.immediate_submit([&](VkCommandBuffer cmd)
		{
			VkImageSubresourceRange range{};
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier transfer_img_barrier{};
			transfer_img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			transfer_img_barrier.pNext = nullptr;

			// Defines the pipeline layout before and after this barrier
			transfer_img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			transfer_img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			transfer_img_barrier.image = new_img.image;
			transfer_img_barrier.subresourceRange = range;

			transfer_img_barrier.srcAccessMask = 0;
			transfer_img_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// Barrier the image into the transfer layout
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &transfer_img_barrier);

			// Image is ready to receive pixel data at this point
			VkBufferImageCopy copy_region{};
			copy_region.bufferOffset = 0;
			copy_region.bufferRowLength = 0;
			copy_region.bufferImageHeight = 0;

			copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.imageSubresource.mipLevel = 0;
			copy_region.imageSubresource.baseArrayLayer = 0;
			copy_region.imageSubresource.layerCount = 1;
			copy_region.imageExtent = img_extent;

			// Copy buffer to image
			vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, new_img.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

			// Turn image into shader readable layout
			VkImageMemoryBarrier readable_img_barrier{ transfer_img_barrier };

			readable_img_barrier.oldLayout = transfer_img_barrier.newLayout;
			readable_img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			readable_img_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			readable_img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Barrier the image into the readable layout
			// TODO: OPTIMIZE GPU BUBBLE
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &readable_img_barrier);
		});

	device.deletion_queue().deletors.emplace_back([&]()
		{
			vmaDestroyImage(device.allocator(), new_img.image, new_img.allocation);
		});

	vmaDestroyBuffer(device.allocator(), staging_buffer.buffer, staging_buffer.allocation);

	std::cout << "Texture loaded successfully " << file_name << std::endl;

	out_image = new_img;
	return true;
}
