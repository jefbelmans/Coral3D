#pragma once

#include "coral_device.h"
#include "coral_mesh.h"

// STD
#include <string>
#include <vector>

namespace coral_3d
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewport_info;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		std::vector<VkDynamicState> dynamic_state_enables;
		VkPipelineDynamicStateCreateInfo dynamic_state_info;
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t subpass = 0;
	};

	class coral_pipeline final
	{
	public:
		coral_pipeline(
			coral_device& device,
			const std::string& vert_file_path,
			const std::string& frag_file_path,
			const PipelineConfigInfo& config_info);

		~coral_pipeline();

		coral_pipeline(const coral_pipeline&) = delete;
		coral_pipeline& operator=(const coral_pipeline&) = delete;

		void bind(VkCommandBuffer command_buffer);

		VkPipeline pipeline() const { return graphics_pipeline_; }

		static void default_pipeline_config_info(PipelineConfigInfo& config_info);
        static std::vector<char> read_file(const std::string& file_path);
        static void create_shader_module(  coral_device& device, const std::vector<char>& code, VkShaderModule* shader_module);

    private:
		void create_graphics_pipeline(
			const std::string& vert_file_path,
			const std::string& frag_file_path,
			const PipelineConfigInfo& config_info
		);


		coral_device& device_;
		VkPipeline graphics_pipeline_;
		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;
	};
}

