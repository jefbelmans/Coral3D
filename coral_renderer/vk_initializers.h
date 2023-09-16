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
    inline VkSpecializationMapEntry specialization_map_entry(uint32_t constant_id, uint32_t offset, size_t size)
    {
        VkSpecializationMapEntry specialization_map_entry{};
        specialization_map_entry.constantID = constant_id;
        specialization_map_entry.offset = offset;
        specialization_map_entry.size = size;
        return specialization_map_entry;
    }
    inline VkSpecializationInfo specialization_info(const std::vector<VkSpecializationMapEntry>& map_entries, size_t data_size, const void* data)
    {
        VkSpecializationInfo specialization_info{};
        specialization_info.mapEntryCount = static_cast<uint32_t>(map_entries.size());
        specialization_info.pMapEntries = map_entries.data();
        specialization_info.dataSize = data_size;
        specialization_info.pData = data;
        return specialization_info;
    }

	// Image initializers
	VkImageCreateInfo image_ci(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo image_view_ci(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkPipelineDepthStencilStateCreateInfo depth_stencil_ci(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
	VkSamplerCreateInfo sampler_ci(VkFilter filters, float max_anisotropy = 1.f, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

	// Descriptor set initializers
	VkDescriptorSetLayoutBinding descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);
	VkWriteDescriptorSet write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);
}