#include "voxel_renderer.h"

// STD
#include <stdexcept>

// CORAL
#include "vk_initializers.h"

using namespace coral_3d;

struct PushConstant
{
	glm::vec2 position;
};

voxel_renderer::voxel_renderer(coral_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
	: device_{ device }
{
	create_pipeline_layout(global_set_layout);
	create_pipeline(render_pass);
}

voxel_renderer::~voxel_renderer()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void voxel_renderer::render_chunks(VoxelRenderInfo& render_info)
{
	// Bind pipeline
	pipeline_->bind(render_info.command_buffer);

	// Bind global descriptor set
	vkCmdBindDescriptorSets(
		render_info.command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_,
		0, 1,
		&render_info.global_descriptor_set,
		0, nullptr
	);

	for (auto& chunk : render_info.chunks)
	{
		if (!chunk.is_active) continue;

		PushConstant push{ chunk.world_position };
		vkCmdPushConstants(
			render_info.command_buffer,
			pipeline_layout_,
			VK_SHADER_STAGE_VERTEX_BIT,
			0, sizeof(PushConstant), &push);

		chunk.mesh->bind(render_info.command_buffer);
		chunk.mesh->draw(render_info.command_buffer);
	}
}

void voxel_renderer::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstant);

	// Add set layouts here
	std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

	VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = &push_constant_range;
	layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
	layout_info.pSetLayouts = descriptor_set_layouts.data();

	if (vkCreatePipelineLayout(device_.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
		throw std::runtime_error("ERROR! voxel_renderer::create_pipeline_layout() >> Failed to create pipeline layout!");
}

void voxel_renderer::create_pipeline(VkRenderPass render_pass)
{
	assert(pipeline_layout_ != nullptr &&
		"ERROR! voxel_renderer::create_pipeline() >> Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipeline_config{};
	coral_pipeline::default_pipeline_config_info(pipeline_config);

	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout_;

	pipeline_ = std::make_unique<coral_pipeline>(
		device_,
		"assets/shaders/voxel_shader.vert.spv",
		"assets/shaders/voxel_shader.frag.spv",
		pipeline_config
	);
}