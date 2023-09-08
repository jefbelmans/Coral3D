#pragma once

#include <memory>
#include "vulkan/vulkan.h"

class coral_texture;
class coral_material final
{
public:
	coral_material(std::shared_ptr<coral_texture> texture, VkPipelineLayout pipeline_layout, VkPipeline pipeline);
	~coral_material();

	void bind(VkCommandBuffer command_buffer);

	VkPipelineLayout pipeline_layout() const { return pipeline_layout_; }
	VkPipeline pipeline() const { return pipeline_; }
	VkDescriptorSet texture_desc_set() const { return texture_desc_set_; }
private:
	
	std::shared_ptr<coral_texture> texture_;

	VkDescriptorSet texture_desc_set_{ VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_;
	VkPipeline pipeline_;
};