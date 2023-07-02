#include "vk_initializers.h"

VkCommandPoolCreateInfo vkinit::command_pool_ci(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	// Create a command pool for commands submitted to the graphics queue
	VkCommandPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	// The command pool will be one that can submit graphics commands
	info.queueFamilyIndex = queueFamilyIndex;

	// We also want the pool to allow for resetting individual command buffers
	info.flags = flags;

	return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_ai(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level)
{
	// Allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	// Commands will be made from our m_CommandPool
	info.commandPool = pool;

	// Allocate just one command buffer
	info.commandBufferCount = count;

	// The command buffer is a primary buffer (that can submit to the queue)
	info.level = level;

	return info;
}

VkCommandBufferBeginInfo vkinit::command_buffer_bi(VkCommandBufferResetFlags flags)
{
	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags = flags;

	return info;
}

VkSubmitInfo vkinit::submit_info(VkCommandBuffer* cmd)
{
	VkSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;

	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.pWaitDstStageMask = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = cmd;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;

	return info;
}

VkFenceCreateInfo vkinit::fence_ci(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkSemaphoreCreateInfo vkinit::semaphore_ci(VkSemaphoreCreateFlags flags)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkPipelineShaderStageCreateInfo vkinit::pipeline_shader_stage_ci(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
{
	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;

	// Shader stage
	info.stage = stage;
	info.module = shaderModule;

	// Entry point in the shader
	info.pName = "main";

	return info;
}

VkPipelineVertexInputStateCreateInfo vkinit::vertex_input_state_ci()
{
	VkPipelineVertexInputStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;

	// No vertex bindings or attributes
	info.vertexBindingDescriptionCount = 0;
	info.vertexAttributeDescriptionCount = 0;

	return info;
}

VkPipelineInputAssemblyStateCreateInfo vkinit::input_assembly_ci(VkPrimitiveTopology topology)
{
	VkPipelineInputAssemblyStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;

	// Primitive topology
	info.topology = topology;
	info.primitiveRestartEnable = VK_FALSE;

	return info;
}

VkPipelineRasterizationStateCreateInfo vkinit::rasterization_state_ci(VkPolygonMode polygonMode)
{
	VkPipelineRasterizationStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;

	// How to handle polygons
	info.depthClampEnable = VK_FALSE;
	info.rasterizerDiscardEnable = VK_FALSE;

	// How to handle filling points between vertices
	info.polygonMode = polygonMode; // Solid / Wireframe
	info.lineWidth = 1.0f;

	// No backface culling
	info.cullMode = VK_CULL_MODE_NONE;
	info.frontFace = VK_FRONT_FACE_CLOCKWISE;

	// No depth bias
	info.depthBiasEnable = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp = 0.0f;
	info.depthBiasSlopeFactor = 0.0f;

	return info;
}

VkPipelineMultisampleStateCreateInfo vkinit::multisample_state_ci()
{
	VkPipelineMultisampleStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;

	// No multisampling
	info.sampleShadingEnable = VK_FALSE;
	// 1 sample per pixel
	info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	info.minSampleShading = 1.0f;
	info.pSampleMask = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable = VK_FALSE;

	return info;
}

VkPipelineColorBlendAttachmentState vkinit::color_blend_attachment_state()
{
	VkPipelineColorBlendAttachmentState info{};
	info.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	info.blendEnable = VK_FALSE;

	return info;
}

VkPipelineLayoutCreateInfo vkinit::pipeline_layout_ci()
{
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

	// Empty defaults
	info.flags = 0;
	info.setLayoutCount = 0;
	info.pSetLayouts = nullptr;
	info.pushConstantRangeCount = 0;
	info.pPushConstantRanges = nullptr;

	return info;
}

VkImageCreateInfo vkinit::image_ci(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
	VkImageCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usageFlags;

	return info;
}

VkImageViewCreateInfo vkinit::image_view_ci(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

VkPipelineDepthStencilStateCreateInfo vkinit::depth_stencil_ci(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
{
	VkPipelineDepthStencilStateCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f;
	info.maxDepthBounds = 1.0f;
	info.stencilTestEnable = VK_FALSE;

	return info;
}

VkSamplerCreateInfo vkinit::sampler_ci(VkFilter filters, VkSamplerAddressMode samplerAddressMode)
{
	VkSamplerCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = nullptr;

	info.magFilter = filters;
	info.minFilter = filters;
	info.addressModeU = samplerAddressMode;
	info.addressModeV = samplerAddressMode;
	info.addressModeW = samplerAddressMode;

	return info;
}

VkDescriptorSetLayoutBinding vkinit::descriptor_set_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding)
{
	VkDescriptorSetLayoutBinding setBind{};
	setBind.binding = binding;
	setBind.descriptorCount = 1;
	setBind.descriptorType = type;
	setBind.pImmutableSamplers = nullptr;
	setBind.stageFlags = stageFlags;

	return setBind;
}

VkWriteDescriptorSet vkinit::write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstSet = dstSet;
	write.dstBinding = binding;

	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = bufferInfo;

	return write;
}

VkWriteDescriptorSet vkinit::write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;

	write.dstBinding = binding;
	write.dstSet = dstSet;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = imageInfo;

	return write;
}
