#pragma once
#include <vector>
#include "vk_types.h"

namespace Coral3D
{
	class PipelineBuilder final
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
		VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
		VkViewport m_Viewport{};
		VkRect2D m_Scissor{};
		VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
		VkPipelineColorBlendAttachmentState m_ColorBlendAttachment{};
		VkPipelineMultisampleStateCreateInfo m_Multisampling{};
		VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
		VkPipelineDepthStencilStateCreateInfo m_DepthStencil;

		VkPipeline build_pipeline(VkDevice device, VkRenderPass renderPass);
	};
}