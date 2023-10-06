#include "coral_texture.h"

// STD
#include <iostream>

// LIBS
#include <stb_image.h>

#include "vk_initializers.h"
#include "coral_device.h"
#include "coral_buffer.h"

using namespace coral_3d;

bool coral_texture::Builder::load_image_from_file(coral_device& device, const std::string& file_name, VkFormat format)
{
	int tex_width, tex_height, tex_channels;

	stbi_uc* img{ stbi_load(file_name.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha)};

	if (!img)
	{
		std::cout << "Failed to load texture file " << file_name << std::endl;
		return false;
	}

	mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

	void* pPixels{ img };
	VkDeviceSize img_size{ static_cast<uint64_t>(tex_width * tex_height * 4) };

	// This format must match with the format loaded from stb_image
	VkFormat img_format{ format };

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

	VkImageCreateInfo image_ci{ vkinit::image_ci(img_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, img_extent) };
	image_ci.mipLevels = mip_levels;

	VmaAllocationCreateInfo image_ai{};
	image_ai.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaCreateImage(device.allocator(), &image_ci, &image_ai,
		&image.image, &image.allocation, nullptr);

	// Transition layout using barries and copy buffer to image
	device.transition_image_layout(image.image, img_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, mip_levels);
	device.copy_buffer_to_image(staging_buffer.get_buffer(), image, img_extent.width, img_extent.height, 1);
	generate_mipmaps(device, tex_width, tex_height);

	return true;
}

void coral_texture::Builder::generate_mipmaps(coral_device& device, uint32_t width, uint32_t height)
{
	device.immediate_submit([&](VkCommandBuffer cmd) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image.image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < mip_levels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(cmd,
				image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mip_levels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		});
}

coral_texture::coral_texture(coral_device& device, AllocatedImage image, uint32_t mip_levels, VkFormat format)
	: device_{ device }
	, image_{ image }
	, mip_levels_{ mip_levels }
{
	create_image_view(format);
	create_texture_sampler();
}

coral_texture::~coral_texture()
{
	vkDestroySampler(device_.device(), sampler_, nullptr);
	vkDestroyImageView(device_.device(), image_view_, nullptr);
	vmaDestroyImage(device_.allocator(), image_.image, image_.allocation);
}

std::unique_ptr<coral_texture> coral_texture::create_texture_from_file(coral_device& device, const std::string& file_path, VkFormat format)
{
	Builder builder{};
	builder.load_image_from_file(device, file_path, format);
	return std::make_unique<coral_texture>(device, builder.image, builder.mip_levels, format);
}

void coral_texture::create_image_view(VkFormat format)
{
	VkImageViewCreateInfo image_info{ vkinit::image_view_ci(format, image_.image, VK_IMAGE_ASPECT_COLOR_BIT) };
	image_info.subresourceRange.levelCount = mip_levels_;

	if (vkCreateImageView(device_.device(), &image_info, nullptr, &image_view_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_texture::create_image_view() >> Failed to create image view!");
}

void coral_texture::create_texture_sampler()
{
	VkSamplerCreateInfo sampler_info{ vkinit::sampler_ci(VK_FILTER_LINEAR, device_.properties.limits.maxSamplerAnisotropy) };
	sampler_info.maxLod = static_cast<float>(mip_levels_);

	if (vkCreateSampler(device_.device(), &sampler_info, nullptr, &sampler_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_texture::create_texture_sampler() >> Failed to create texture sampler!");
}
