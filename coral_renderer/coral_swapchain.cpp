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
#include "vk_initializers.h"

using namespace coral_3d;

coral_swapchain::coral_swapchain(coral_device& device, VkExtent2D extent, std::shared_ptr<coral_swapchain> old_swapchain)
    : device_{ device }, window_extent_{ extent }, old_swapchain_{old_swapchain}
{
	init();

	old_swapchain_ = nullptr;
}

coral_swapchain::coral_swapchain(coral_device& device, VkExtent2D extent)
	: device_{ device }, window_extent_{ extent }
{
	init();
}

coral_swapchain::~coral_swapchain()
{
	for (auto image_view : swapchain_image_views_)
	{
		vkDestroyImageView(device_.device(), image_view, nullptr);
	}
	swapchain_image_views_.clear();

	vkDestroySwapchainKHR(device_.device(), swapchain_, nullptr);
	swapchain_ = nullptr;

	deletion_queue_.flush();
}

VkFormat coral_swapchain::find_depth_format()
{
	return device_.find_supported_format(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult coral_swapchain::aqcuire_next_image(uint32_t* image_index)
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

void coral_swapchain::init()
{
	create_swapchain();
	create_image_views();
	create_render_pass();
	create_depth_resources();
	create_frame_buffers();
	create_sync_structures();
}

void coral_swapchain::create_swapchain()
{
	SwapchainSupportDetails swap_chain_support = device_.get_swapchain_support();

	VkSurfaceFormatKHR surface_format = choose_swapchain_format(swap_chain_support.formats);
	VkPresentModeKHR present_mode = choose_present_mode(swap_chain_support.present_modes);
	VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 &&
		image_count > swap_chain_support.capabilities.maxImageCount)
	{
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = device_.surface();

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = device_.find_physical_queue_families();
	uint32_t queueFamilyIndices[] = { indices.graphics_family, indices.present_family };

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;      // Optional
		create_info.pQueueFamilyIndices = nullptr;  // Optional
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	create_info.oldSwapchain = old_swapchain_ == nullptr ? VK_NULL_HANDLE : old_swapchain_->swapchain_;

	if (vkCreateSwapchainKHR(device_.device(), &create_info, nullptr, &swapchain_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_swapchain::create_swapchain() >> Failed to create swapchain!");

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	vkGetSwapchainImagesKHR(device_.device(), swapchain_, &image_count, nullptr);
	swapchain_images_.resize(image_count);
	vkGetSwapchainImagesKHR(device_.device(), swapchain_, &image_count, swapchain_images_.data());

	swapchain_image_format_ = surface_format.format;
	swapchain_extent_ = extent;
}

void coral_swapchain::create_image_views()
{
	swapchain_image_views_.resize(swapchain_images_.size());

	for (size_t i = 0; i < swapchain_images_.size(); i++)
	{
		VkImageViewCreateInfo viewInfo{ vkinit::image_view_ci(swapchain_image_format_, swapchain_images_[i], VK_IMAGE_ASPECT_COLOR_BIT)};

		if (vkCreateImageView(device_.device(), &viewInfo, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_image_views() >> Failed to create image view!");
	}
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
	dependency.dstSubpass = 0;

	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	
	

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

	deletion_queue_.deletors.emplace_back([=, this]() {
		vkDestroyRenderPass(device_.device(), render_pass_, nullptr);
		});
}

void coral_swapchain::create_depth_resources()
{
	VkFormat depth_format{ find_depth_format() };
	swapchain_depth_format_ = depth_format;

	depth_images_.resize(image_count());
	depth_image_views_.resize(image_count());

	VkExtent3D depth_image_extent
	{
		swapchain_extent_.width,
		swapchain_extent_.height,
		1
	};

	VkImageCreateInfo image_info{ vkinit::image_ci(depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent) };
	
	for (size_t i = 0; i < depth_images_.size(); i++)
	{
		depth_images_[i] = device_.create_image(image_info, VMA_MEMORY_USAGE_GPU_ONLY);

		VkImageViewCreateInfo image_view_info{ vkinit::image_view_ci(depth_format, depth_images_[i].image, VK_IMAGE_ASPECT_DEPTH_BIT) };
		if (vkCreateImageView(device_.device(), &image_view_info, nullptr, &depth_image_views_[i]) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_depth_resources() >> Failed to create texture image view!");

		deletion_queue_.deletors.emplace_back([=, this]() {
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

		deletion_queue_.deletors.emplace_back([=, this]() {
			vkDestroyFramebuffer(device_.device(), swapchain_frame_buffers_[i], nullptr);
			// vkDestroyImageView(device_.device(), swapchain_image_views_[i], nullptr);
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

		deletion_queue_.deletors.emplace_back([=, this]() {
			vkDestroyFence(device_.device(), frames_[i].render_fence, nullptr);
			});

		if(vkCreateSemaphore(device_.device(), &semaphoreCreateInfo, nullptr, &frames_[i].present_semaphore) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_sync_structures() >> Failed to create present semaphore!");
		if(vkCreateSemaphore(device_.device(), &semaphoreCreateInfo, nullptr, &frames_[i].render_semaphore) != VK_SUCCESS)
			throw std::runtime_error("ERROR! coral_swapchain::create_sync_structures() >> Failed to create render semaphore!");

		deletion_queue_.deletors.emplace_back([=, this]()
			{
				vkDestroySemaphore(device_.device(), frames_[i].present_semaphore, nullptr);
				vkDestroySemaphore(device_.device(), frames_[i].render_semaphore, nullptr);
			});
	}
}

VkSurfaceFormatKHR coral_swapchain::choose_swapchain_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& available_format : available_formats)
	{
		if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return available_format;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR coral_swapchain::choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			std::cout << "Present mode: Mailbox" << std::endl;
			return available_present_mode;
		}
	}

    return VkPresentModeKHR();
}

VkExtent2D coral_swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	else
	{
		VkExtent2D actualExtent = window_extent_;
		actualExtent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}