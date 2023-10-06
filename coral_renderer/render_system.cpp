#include "render_system.h"
#include "vk_initializers.h"

using namespace coral_3d;

render_system::render_system(coral_device& device, std::vector<VkDescriptorSetLayout>& desc_set_layouts)
	: device_{device}
    , pipeline_layout_{VK_NULL_HANDLE}
{
    create_pipeline_layout(device_, desc_set_layouts);
}

render_system::~render_system()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void render_system::render_gameobjects(FrameInfo& frame_info)
{
	vkCmdBindDescriptorSets(
		frame_info.command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_,
		0, 1,
        &frame_info.global_descriptor_set,
		0, nullptr
	);

	for (auto& kv : frame_info.gameobjects)
	{
		auto& obj{ kv.second };
		if (obj->mesh_ == nullptr) continue;

		obj->mesh_->draw(frame_info.command_buffer, pipeline_layout_);
	}
}

void render_system::create_pipeline_layout(coral_device &device, std::vector<VkDescriptorSetLayout>& desc_set_layouts)
{
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstant);

    VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_constant_range;
    layout_info.setLayoutCount = static_cast<uint32_t>(desc_set_layouts.size());
    layout_info.pSetLayouts = desc_set_layouts.data();

    if (vkCreatePipelineLayout(device.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        throw std::runtime_error("ERROR! coral_pipeline::create_pipeline_layout() >> Failed to create pipeline layout!");
}