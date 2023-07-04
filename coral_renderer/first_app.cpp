#include "first_app.h"

// STD
#include <stdexcept>
#include <array>

#include "vk_initializers.h"

using namespace coral_3d;

first_app::first_app()
{
	load_gameobjects();
	create_pipeline_layout();
	recreate_swapchain();
	create_command_buffers();
}

first_app::~first_app()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void first_app::run()
{
	while (!window_.should_close())
	{
		glfwPollEvents();
		draw_frame();
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::load_gameobjects()
{
	std::vector<Vertex> vertices
	{
		{{0.0f, -0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f }},
		{{-0.5f, 0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f }},
		{{0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }},
	};

	auto mesh{ std::make_shared<coral_mesh>(device_, vertices) };
	auto triangle = coral_gameobject::create_gameobject();
	triangle.mesh_ = mesh;
	triangle.color_ = { 0.1f, 0.8f, 0.1f };
	triangle.transform_.translation.x = -.2f;

	gameobjects_.emplace_back(std::move(triangle));
}

void first_app::create_pipeline_layout()
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstant);

	VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = &push_constant_range;

	if (vkCreatePipelineLayout(device_.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_pipeline_layout() >> Failed to create pipeline layout!");
}

void first_app::create_pipeline()
{
	auto pipeline_config{ coral_pipeline::default_pipeline_config_info(swapchain_->width(), swapchain_->height()) };
	pipeline_config.render_pass = swapchain_->get_render_pass();
	pipeline_config.pipeline_layout = pipeline_layout_;

	pipeline_ = std::make_unique<coral_pipeline>(
		device_,
		// "shaders/PosNormCol.vert.spv",
		// "shaders/PosNormCol.frag.spv",
		"shaders/simple_shader.vert.spv",
		"shaders/simple_shader.frag.spv",
		pipeline_config
	);
}

void first_app::create_command_buffers()
{
	command_buffers_.resize(swapchain_->image_count());

	VkCommandBufferAllocateInfo alloc_info{ vkinit::command_buffer_ai(device_.get_command_pool(), static_cast<uint32_t>(command_buffers_.size())) };
	if (vkAllocateCommandBuffers(device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to allocate command buffers!");
}

void first_app::free_command_buffers()
{
	vkFreeCommandBuffers(
		device_.device(),
		device_.get_command_pool(),
		static_cast<uint32_t>(command_buffers_.size()),
		command_buffers_.data());

	command_buffers_.clear();
}

void first_app::draw_frame()
{
	uint32_t image_index;
	auto result = swapchain_->aqcuire_next_image(&image_index);

	// Window has been resized
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreate_swapchain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("ERROR! first_app::draw_frame() >> Failed to aquire swapchain image!");

	record_command_buffer(image_index);
	result = swapchain_->submit_command_buffer(&command_buffers_[image_index], &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.was_window_resized())
	{
		window_.reset_window_resized();
		recreate_swapchain();
		return;
	}

	if(result != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::draw_frame() >> Failed to present swapchain image!");
}

void first_app::recreate_swapchain()
{
	auto extent{ window_.get_extent() };

	// Idle when minimized
	while (extent.width == 0 || extent.height == 0)
	{
		extent = window_.get_extent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device_.device());

	if(swapchain_ == nullptr)
		swapchain_ = std::make_unique<coral_swapchain>(device_, extent);
	else
	{
		swapchain_ = std::make_unique<coral_swapchain>(device_, extent, std::move(swapchain_));
		if (swapchain_->image_count() != command_buffers_.size())
		{
			free_command_buffers();
			create_command_buffers();
		}
	}
		
	create_pipeline();
}

void first_app::record_command_buffer(int image_index)
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffers_[image_index], &begin_info) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to begin recording command buffer!");

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = swapchain_->get_render_pass();
	render_pass_info.framebuffer = swapchain_->get_frame_buffers(image_index);

	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = swapchain_->get_swapchain_extent();

	std::array<VkClearValue, 2> clear_values{};
	clear_values[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
	clear_values[1].depthStencil = { 1.0f, 0 };
	render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
	render_pass_info.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(command_buffers_[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	render_gameobjects(command_buffers_[image_index]);

	vkCmdEndRenderPass(command_buffers_[image_index]);

	if (vkEndCommandBuffer(command_buffers_[image_index]) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to record command buffer!");
}

void coral_3d::first_app::render_gameobjects(VkCommandBuffer command_buffer)
{
	pipeline_->bind(command_buffer);

	for (auto& obj : gameobjects_)
	{
		PushConstant push{};
		push.offset = obj.transform_.translation;
		push.color = obj.color_;
		push.transform = obj.transform_.mat3();

		vkCmdPushConstants(
			command_buffer,
			pipeline_layout_,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(PushConstant), &push);

		obj.mesh_->bind(command_buffer);
		obj.mesh_->draw(command_buffer);
	}
}