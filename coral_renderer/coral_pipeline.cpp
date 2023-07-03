#include "coral_pipeline.h"

#include <iostream>
#include <fstream>

using namespace coral_3d;

coral_pipeline::coral_pipeline(
	coral_device& device,
	const std::string& vert_file_path,
	const std::string& frag_file_path,
	const PipelineConfigInfo& config_info) : device_{device}
{
	create_graphics_pipeline(vert_file_path, frag_file_path, config_info);
}

PipelineConfigInfo coral_3d::coral_pipeline::default_pipeline_config_info(uint32_t width, uint32_t height)
{
	PipelineConfigInfo config_info{};
	return config_info;
}

std::vector<char> coral_pipeline::read_file(const std::string& file_path)
{
	std::ifstream file{file_path, std::ios::ate | std::ios::binary };

	if (!file.is_open())
		throw std::runtime_error("ERROR! coral_pipeline::read_file() >> Failed to open file: " + file_path);

	// Create buffer sized to the file
	size_t file_size{ static_cast<size_t>(file.tellg()) };
	std::vector<char> buffer(file_size);

	// Read file into buffer
	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();
	return buffer;
}

void coral_pipeline::create_graphics_pipeline(
	const std::string& vert_file_path,
	const std::string& frag_file_path,
	const PipelineConfigInfo& config_info)
{
	auto vertShader = read_file(vert_file_path);
	auto fragShader = read_file(frag_file_path);

	std::cout << "Vertex shader code size: " << vertShader.size() << "\n";
	std::cout << "Frag shader code size: " << fragShader.size() << "\n";
}

void coral_3d::coral_pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = nullptr;

	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if(vkCreateShaderModule(device_.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_pipeline::create_shader_module() >> Failed to create shader module!");
}