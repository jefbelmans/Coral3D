#include "coral_material.h"

coral_material::coral_material(std::shared_ptr<coral_texture> texture, VkPipelineLayout pipeline_layout, VkPipeline pipeline)
	: texture_(texture)
    , pipeline_(pipeline)
	, pipeline_layout_(pipeline_layout)
{}

coral_material::~coral_material()
{
}

void coral_material::bind(VkCommandBuffer command_buffer)
{
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

	// Only bind texture descriptor set if it exists
	if(texture_desc_set_ != VK_NULL_HANDLE)
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 1, 1, &texture_desc_set_, 0, nullptr);
}
