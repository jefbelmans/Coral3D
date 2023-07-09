#include "render_system.h"

#include "vk_initializers.h"
#include <stdexcept>

using namespace coral_3d;

struct PushConstant
{
	glm::mat4 world_matrix;
	glm::mat4 normal_matrix;
};

render_system::render_system(coral_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
	: device_{device}
{
	create_pipeline_layout(global_set_layout);
	create_pipeline(render_pass);
}

render_system::~render_system()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void render_system::render_gameobjects(FrameInfo& frame_info)
{
	pipeline_->bind(frame_info.command_buffer);

	vkCmdBindDescriptorSets(
		frame_info.command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_,
		0, 1,
		&frame_info.global_descriptor_set,
		0, nullptr
	);

	coral_mesh* last_mesh{nullptr};
	for (auto& kv : frame_info.gameobjects)
	{
		auto& obj{ kv.second };

		if (obj.mesh_ == nullptr) continue;

		PushConstant push{};
		push.world_matrix =  obj.transform_.mat4();
		push.normal_matrix = obj.transform_.noraml_matrix();

		vkCmdPushConstants(
			frame_info.command_buffer,
			pipeline_layout_,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(PushConstant), &push);

		if(obj.mesh_.get() != last_mesh)
			obj.mesh_->bind(frame_info.command_buffer);

		// obj.mesh_->bind(frame_info.command_buffer);
		obj.mesh_->draw(frame_info.command_buffer);

		last_mesh = obj.mesh_.get();
	}
}

void render_system::create_pipeline_layout(VkDescriptorSetLayout global_set_layout)
{
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
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
		throw std::runtime_error("ERROR! first_app::create_pipeline_layout() >> Failed to create pipeline layout!");
}

void render_system::create_pipeline(VkRenderPass render_pass)
{

	assert(pipeline_layout_ != nullptr &&
		"ERROR! first_app::create_pipeline() >> Cannot create pipeline before pipeline layout!");

	PipelineConfigInfo pipeline_config{};
	coral_pipeline::default_pipeline_config_info(pipeline_config);

	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout_;

	pipeline_ = std::make_unique<coral_pipeline>(
		device_,
		// "shaders/PosNormCol.vert.spv",
		// "shaders/PosNormCol.frag.spv",
		"assets/shaders/simple_shader.vert.spv",
		"assets/shaders/simple_shader.frag.spv",
		pipeline_config
	);
}
