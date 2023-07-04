#include "coral_renderer.h"

// STD
#include <array>
#include <stdexcept>

#include "vk_initializers.h"

using namespace coral_3d;

coral_renderer::coral_renderer(coral_window& window, coral_device& device)
    : window_{window}
    , device_{device}
{
    recreate_swapchain();
    create_command_buffers();
}

coral_renderer::~coral_renderer()
{
    free_command_buffers();
}

VkCommandBuffer coral_renderer::begin_frame()
{
	assert(!is_frame_started_ && "ERROR! coral_renderer::begin_frame() >> Can't call begin frame while already in progress!");
    
	auto result = swapchain_->aqcuire_next_image(&current_image_index_);

	// Window has been resized
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain();
		return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("ERROR! coral_renderer::begin_frame() >> Failed to aquire swapchain image!");

	is_frame_started_ = true;

	auto command_buffer{ get_current_command_buffer() };

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_renderer::begin_frame() >> Failed to begin recording command buffer!");

	return command_buffer;
}

void coral_renderer::end_frame()
{
	assert(is_frame_started_ && "ERROR! coral_renderer::end_frame() >> Can't call end_frame() if frame is not in progress!");

	auto command_buffer{ get_current_command_buffer() };
	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_renderer::end_frame() >> Failed to record command buffer!");

	auto result = swapchain_->submit_command_buffer(&command_buffer, &current_image_index_);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.was_window_resized())
	{
		window_.reset_window_resized();
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_renderer::end_frame() >> Failed to present swapchain image!");

	is_frame_started_ = false;
	current_frame_index_ = (current_frame_index_ + 1) % coral_swapchain::MAX_FRAMES_IN_FLIGHT;
}

void coral_renderer::begin_swapchain_render_pass(VkCommandBuffer command_buffer)
{
	assert(is_frame_started_ && "ERROR! first_app::begin_swapchain_render_pass() >> Can't call begin_swapchain_render_pass() if frame is not in progress!");
	assert(command_buffer == get_current_command_buffer() &&
		"ERROR! first_app::begin_swapchain_render_pass() >> Can't begin render pass on a buffer from a different frame");

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = swapchain_->get_render_pass();
	render_pass_info.framebuffer = swapchain_->get_frame_buffers(current_image_index_);

	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = swapchain_->get_swapchain_extent();

	std::array<VkClearValue, 2> clear_values{};
	clear_values[0].color = { 0.02f, 0.02f, 0.02f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain_->get_swapchain_extent().width);
	viewport.height = static_cast<float>(swapchain_->get_swapchain_extent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, swapchain_->get_swapchain_extent() };
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

}

void coral_renderer::end_swapchain_render_pass(VkCommandBuffer command_buffer)
{
	assert(is_frame_started_ && "ERROR! first_app::end_swapchain_render_pass() >> Can't call end_swapchain_render_pass() if frame is not in progress!");
	assert(command_buffer == get_current_command_buffer() &&
		"ERROR! first_app::end_swapchain_render_pass() >> Can't end render pass on a buffer from a different frame");

	vkCmdEndRenderPass(command_buffer);
}

void coral_renderer::create_command_buffers()
{
	command_buffers_.resize(coral_swapchain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo alloc_info{ vkinit::command_buffer_ai(device_.get_command_pool(), static_cast<uint32_t>(command_buffers_.size())) };
	if (vkAllocateCommandBuffers(device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to allocate command buffers!");
}

void coral_renderer::free_command_buffers()
{
	vkFreeCommandBuffers(
		device_.device(),
		device_.get_command_pool(),
		static_cast<uint32_t>(command_buffers_.size()),
		command_buffers_.data());

	command_buffers_.clear();
}

void coral_renderer::recreate_swapchain()
{
	auto extent{ window_.get_extent() };

	// Idle when minimized
	while (extent.width == 0 || extent.height == 0)
	{
		extent = window_.get_extent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device_.device());

	if (swapchain_ == nullptr)
		swapchain_ = std::make_unique<coral_swapchain>(device_, extent);
	else
	{
		std::shared_ptr<coral_swapchain> old_swapchain { std::move(swapchain_) };
		swapchain_ = std::make_unique<coral_swapchain>(device_, extent, old_swapchain);

		if (!old_swapchain->compare_swap_formats(*swapchain_.get()))
			throw std::runtime_error("ERROR! coral_renderer::recreate_swapchain() >> Swapchain image or depth format has changed!");
	}
}