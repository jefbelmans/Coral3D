#include "render_system.h"

#include "vk_initializers.h"
#include <stdexcept>

using namespace coral_3d;

struct PushConstant
{
	glm::mat4 transform;
	alignas(16) glm::vec3 color;
};

render_system::render_system(coral_device& device, VkRenderPass render_pass)
	: device_{device}
{
	create_pipeline_layout();
	create_pipeline(render_pass);
}

render_system::~render_system()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void render_system::render_gameobjects(VkCommandBuffer command_buffer, std::vector<coral_gameobject>& gameobjects, const coral_camera& camera)
{
	pipeline_->bind(command_buffer);

	auto view_projection{ camera.get_projection() * camera.get_view() };

	for (auto& obj : gameobjects)
	{
		obj.transform_.rotation.y = glm::mod(obj.transform_.rotation.y + 0.001f, glm::two_pi<float>());
		obj.transform_.rotation.x = glm::mod(obj.transform_.rotation.x + 0.0005f, glm::two_pi<float>());

		PushConstant push{};
		push.color = obj.color_;
		push.transform = view_projection * obj.transform_.mat4();

		vkCmdPushConstants(
			command_buffer,
			pipeline_layout_,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(PushConstant), &push);

		obj.mesh_->bind(command_buffer);
		obj.mesh_->draw(command_buffer);
	}
}

void render_system::create_pipeline_layout()
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
		"shaders/simple_shader.vert.spv",
		"shaders/simple_shader.frag.spv",
		pipeline_config
	);
}
