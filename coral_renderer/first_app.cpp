#include "first_app.h"

// STD
#include <stdexcept>
#include <array>

#include "vk_initializers.h"

using namespace coral_3d;

first_app::first_app()
{
	load_meshes();
	create_pipeline_layout();
	create_pipeline();
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

void first_app::load_meshes()
{
	std::vector<Vertex> vertices
	{
		{{0.0f, -0.5f}},
		{{0.5f,  0.5f}},
		{{-0.5f, 0.5f}}
	};

	mesh_ = std::make_unique<coral_mesh>(device_, vertices);
}

void first_app::create_pipeline_layout()
{
	VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
	if (vkCreatePipelineLayout(device_.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_pipeline_layout() >> Failed to create pipeline layout!");
}

void first_app::create_pipeline()
{
	auto pipeline_config{ coral_pipeline::default_pipeline_config_info(swapchain_.width(), swapchain_.height()) };
	pipeline_config.render_pass = swapchain_.get_render_pass();
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
	command_buffers_.resize(swapchain_.image_count());

	VkCommandBufferAllocateInfo alloc_info{ vkinit::command_buffer_ai(device_.get_command_pool(), static_cast<uint32_t>(command_buffers_.size())) };
	if (vkAllocateCommandBuffers(device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to allocate command buffers!");

	for (int i = 0; i < command_buffers_.size(); i++)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS)
			throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to begin recording command buffer!");

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = swapchain_.get_render_pass();
		render_pass_info.framebuffer = swapchain_.get_frame_buffers(i);

		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = swapchain_.get_swapchain_extent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		pipeline_->bind(command_buffers_[i]);
		mesh_->bind(command_buffers_[i]);
		mesh_->draw(command_buffers_[i]);

		vkCmdEndRenderPass(command_buffers_[i]);

		if (vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS)
			throw std::runtime_error("ERROR! first_app::create_command_buffers() >> Failed to record command buffer!");
	}
}

void first_app::draw_frame()
{
	uint32_t image_index;
	auto result = swapchain_.aqcuire_next_image(&image_index);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("ERROR! first_app::draw_frame() >> Failed to aquire swapchain image!");

	result = swapchain_.submit_command_buffer(&command_buffers_[image_index], &image_index);
	if(result != VK_SUCCESS)
		throw std::runtime_error("ERROR! first_app::draw_frame() >> Failed to present swapchain image!");
}
