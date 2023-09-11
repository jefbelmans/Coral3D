#pragma once
#include "vk_types.h"

namespace vkinit
{
	// Commands
	VkCommandPoolCreateInfo command_pool_ci(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo command_buffer_ai(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkCommandBufferBeginInfo command_buffer_bi(VkCommandBufferResetFlags flags = 0);
	VkSubmitInfo submit_info(VkCommandBuffer* cmd);

	// Sync structures
	VkFenceCreateInfo fence_ci(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_ci(VkSemaphoreCreateFlags flags = 0);

	// Pipeline initializers
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_ci(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_ci();
	VkPipelineInputAssemblyStateCreateInfo input_assembly_ci(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo rasterization_state_ci(VkPolygonMode polygonMode);
	VkPipelineMultisampleStateCreateInfo multisample_state_ci();
	VkPipelineColorBlendAttachmentState color_blend_attachment_state();
	VkPipelineLayoutCreateInfo pipeline_layout_ci();

	// Image initializers
    VkSamplerCreateInfo sampler_ci(VkFilter min_mag_filter, VkSamplerAddressMode address_mode, VkSamplerMipmapMode mip_map_mode, float max_anisotropic);
	VkImageCreateInfo image_ci(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo image_view_ci(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkPipelineDepthStencilStateCreateInfo depth_stencil_ci(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkSamplerCreateInfo sampler_ci(VkFilter filters, float max_anisotropy = 1.f, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

	// Descriptor set initializers
	VkDescriptorSetLayoutBinding descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);
	VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);
}