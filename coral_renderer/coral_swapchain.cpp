#include "coral_swapchain.h"

// STD
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

// VULKAN
#include <VkBootstrap.h>
#include "vk_initializers.h"

using namespace coral_3d;

coral_swapchain::coral_swapchain(coral_device& device, VkExtent2D extent)
    :device_{ device }, window_extent_{ extent }
{
    create_swapchain();
    create_render_pass();
    create_depth_resources();
    create_frame_buffers();
    create_sync_structures();
}

coral_swapchain::~coral_swapchain()
{
	deletion_queue_.flush();
}

VkFormat coral_swapchain::find_depth_format()
{
	return device_.find_supported_format(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult coral_3d::coral_swapchain::aqcuire_next_image(uint32_t* image_index)
{
	// NOTE: as of 04/07, vkDeviceWaitIdle is about 3% faster when rendering a single triangle.
	vkDeviceWaitIdle(device_.device());
	// vkWaitForFences(device_.device(), 1, &get_current_frame().render_fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device_.device(), 1, &get_current_frame().render_fence);

	VkResult result = vkAcquireNextImageKHR(
		device_.device(),
		swapchain_,
		UINT64_MAX,
		get_current_frame().present_semaphore,  // must be a not signaled semaphore
		VK_NULL_HANDLE,
		image_index);

	return result;
}

VkResult coral_swapchain::submit_command_buffer(const VkCommandBuffer* buffers, uint32_t* image_index)
{
	// Prepare to submit to the queue
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	submit.pWaitDstStageMask = &waitStage;

	// Wait for image to be ready for rendering before rendering to it
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame().present_semaphore;

	// Signal ready when finished rendering
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame().render_semaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = buffers;

	// Submit to the graphics queue
	if (vkQueueSubmit(device_.graphics_queue(), 1, &submit, get_current_frame().render_fence) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_swapchain::submit_command_buffer() >> Failed to submit draw command buffer!");

	// Present the image to the window when the rendering semaphore has signaled that rendering is done
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &swapchain_;
	presentInfo.swapchainCount = 1;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &get_current_frame().render_semaphore;

	// Which image to present
	presentInfo.pImageIndices = image_index;

	auto result = vkQueuePresentKHR(device_.present_queue(), &presentInfo);

	current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	return result;
}

void coral_swapchain::create_swapchain()
{
	vkb::SwapchainBuilder swapchain_builder{ device_.physical_device(), device_.device(), device_.surface() };

	vkb::Swapchain vkb_swapchain = swapchain_builder
		.use_default_format_selection()
		// use VSync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
		.set_desired_extent(window_extent_.width, window_extent_.height)
		.build()
		.value();

	// store swap chain and its related data
	swapchain_ = vkb_swapchain.swapchain;
	swapchain_images_ = vkb_swapchain.get_images().value();
	swapchain_image_views_ = vkb_swapchain.get_image_views().value();
	swapchain_image_format_ = vkb_swapchain.image_format;

	swapchain_extent_ = vkb_swapchain.extent;

	deletion_queue_.deletors.emplace_back([=]() {
		vkDestroySwapchainKHR(device_.device(), swapchain_, nullptr);
		});
}

void coral_swapchain::create_render_pass()
{
	// Description of the image that we will be writing rendering commands to
	VkAttachmentDescription color_attachment{};

	// format should be the same as the swap chain images
	color_attachment.format = swapchain_image_format_;
	// MSAA samples, set to 1 (no MSAA) by default
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// Clear when render pass begins
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Keep the attachment stored when render pass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// Don't care about stencil data
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// Image data layout before render pass starts (undefined = don't care)
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Image data layout after render pass (to change to), set to present by default
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	// Attachment number will index into the pAttachments array in the parent render pass itself
	color_attachment_ref.attachment = 0;
	// Optimal layout for writing to the image
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Description of the depth image
	VkAttachmentDescription depth_attachment{};
	depth_attachment.flags = 0;
	depth_attachment.format = find_depth_format();
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref{};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Create one sub pass (minimum one sub pass required)
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	// These dependencies tell Vulkan that the attachment cannot be used before the previous renderpasses have finished using it
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	std::array<VkAttachmentDescription, 2> attachments { color_attachment, depth_attachment };

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.pNext = nullptr;

	// Connect the color attachment description to the info
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_info.pAttachments = attachments.data();

	// Connect the subpass(es) to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	// Connect the dependency
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(device_.device(), &render_pass_info, nullptr, &render_pass_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_swapchain::create_render_pass() >> Failed to create render pass!");

	deletion_queue_.deletors.emplace_back([=]() {
		vkDestroyRenderPass(device_.device(), render_pass_, nullptr);
		});
}

void coral_swapchain::create_depth_resources()
{
	VkFormat depth_format{ find_depth_format() };

	depth_images_.resize(image_count());
	depth_image_views_.resize(image_count());

	VkExtent3D depth_image_extent
	{
		swapchain_extent_.width,
		swapchain_extent_.height,
		1.f
	};

	VkImageCreateInfo image_info{ vkinit::image_ci(depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent) };
	
	for (int i = 0; i < depth_images_.size(); i++)
	{
		depth_images_[i] = device_.create_image(image_info, VMA_MEMORY_USAGE_GPU_ONLY);

		VkImageViewCreateInfo image_view_info{ vkinit::image_view_ci(depth_format, depth_images_[i].image, VK_IMAGE_ASPECT_DEPTH_BIT) };
		if (vkCreateImageView(device_.device(), &image_view_info, nullptr, &depth_image_views_[i]) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_depth_resources() >> Failed to create texture image view!");

		deletion_queue_.deletors.emplace_back([=]() {
			vkDestroyImageView(device_.device(), depth_image_views_[i], nullptr);
			vmaDestroyImage(device_.allocator(), depth_images_[i].image, depth_images_[i].allocation);
			});
	}
}

void coral_swapchain::create_frame_buffers()
{
	swapchain_frame_buffers_.resize(image_count());

	VkFramebufferCreateInfo fb_info{};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.renderPass = render_pass_;
	fb_info.width = swapchain_extent_.width;
	fb_info.height = swapchain_extent_.height;
	fb_info.layers = 1;

	for (size_t i = 0; i < swapchain_frame_buffers_.size(); i++)
	{
		std::array<VkImageView, 2> attachments{swapchain_image_views_[i], depth_image_views_[i]};

		fb_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		fb_info.pAttachments = attachments.data();

		if (vkCreateFramebuffer(device_.device(), &fb_info, nullptr, &swapchain_frame_buffers_[i]) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_frame_buffers() >> Failed to create framebuffer!");

		deletion_queue_.deletors.emplace_back([=]() {
			vkDestroyFramebuffer(device_.device(), swapchain_frame_buffers_[i], nullptr);
			vkDestroyImageView(device_.device(), swapchain_image_views_[i], nullptr);
			});
	}
}

void coral_swapchain::create_sync_structures()
{
	// Render fence
	VkFenceCreateInfo fenceCreateInfo{ vkinit::fence_ci(VK_FENCE_CREATE_SIGNALED_BIT) };
	// Sempahore needs no flags
	VkSemaphoreCreateInfo semaphoreCreateInfo{ vkinit::semaphore_ci() };

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateFence(device_.device(), &fenceCreateInfo, nullptr, &frames_[i].render_fence) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_sync_structures() >> Failed to create render fence!");

		deletion_queue_.deletors.emplace_back([=]() {
			vkDestroyFence(device_.device(), frames_[i].render_fence, nullptr);
			});

		if(vkCreateSemaphore(device_.device(), &semaphoreCreateInfo, nullptr, &frames_[i].present_semaphore) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_sync_structures() >> Failed to create present semaphore!");
		if(vkCreateSemaphore(device_.device(), &semaphoreCreateInfo, nullptr, &frames_[i].render_semaphore) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_sync_structures() >> Failed to create render semaphore!");

		deletion_queue_.deletors.emplace_back([=]()
			{
				vkDestroySemaphore(device_.device(), frames_[i].present_semaphore, nullptr);
				vkDestroySemaphore(device_.device(), frames_[i].render_semaphore, nullptr);
			});
	}
}