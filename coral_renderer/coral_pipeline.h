#pragma once

#include "coral_device.h"

// STD
#include <string>
#include <vector>

namespace coral_3d
{
	struct PipelineConfigInfo{};

	class coral_pipeline final
	{
	public:
		coral_pipeline(
			coral_device& device,
			const std::string& vert_file_path,
			const std::string& frag_file_path,
			const PipelineConfigInfo& config_info);

		~coral_pipeline() {};

		coral_pipeline(const coral_pipeline&) = delete;
		coral_pipeline& operator=(const coral_pipeline&) = delete;

		static PipelineConfigInfo default_pipeline_config_info(uint32_t width, uint32_t height);

	private:
		static std::vector<char> read_file(const std::string& file_path);
		
		void create_graphics_pipeline(
			const std::string& vert_file_path,
			const std::string& frag_file_path,
			const PipelineConfigInfo& config_info
		);

		void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

		coral_device& device_;
		VkPipeline graphics_pipeline_;
		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;
	};
}

