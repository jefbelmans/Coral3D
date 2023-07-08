#include "coral_pipeline.h"

// STD
#include <iostream>
#include <fstream>
#include <cassert>

#include "coral_mesh.h"
#include "vk_initializers.h"

using namespace coral_3d;

coral_pipeline::coral_pipeline(
	coral_device& device,
	const std::string& vert_file_path,
	const std::string& frag_file_path,
	const PipelineConfigInfo& config_info) : device_{device}
{
	create_graphics_pipeline(vert_file_path, frag_file_path, config_info);
}

coral_pipeline::~coral_pipeline()
{
	vkDestroyShaderModule(device_.device(), vert_shader_module_, nullptr);
	vkDestroyShaderModule(device_.device(), frag_shader_module_, nullptr);
	vkDestroyPipeline(device_.device(), graphics_pipeline_, nullptr);
}

void coral_pipeline::bind(VkCommandBuffer command_buffer)
{
	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);
}

void coral_pipeline::default_pipeline_config_info(PipelineConfigInfo& config_info)
{
	// IA
	config_info.input_assembly_info = vkinit::input_assembly_ci(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	// VIEWPORT INFO
	config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	config_info.viewport_info.pNext = nullptr;
	config_info.viewport_info.viewportCount = 1;
	config_info.viewport_info.pViewports = nullptr;
	config_info.viewport_info.scissorCount = 1;
	config_info.viewport_info.pScissors = nullptr;

	// RASTERIZER
	config_info.rasterization_info = vkinit::rasterization_state_ci(VK_POLYGON_MODE_FILL);

	// MULTISAMPLER
	config_info.multisample_info = vkinit::multisample_state_ci();

	// BLEND ATTACHMENT
	config_info.color_blend_attachment = vkinit::color_blend_attachment_state();

	// BLEND INFO
	config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	config_info.color_blend_info.pNext = nullptr;
	config_info.color_blend_info.logicOpEnable = VK_FALSE;
	config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	config_info.color_blend_info.attachmentCount = 1;
	config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
	config_info.color_blend_info.blendConstants[0] = 0.f;
	config_info.color_blend_info.blendConstants[1] = 0.f;
	config_info.color_blend_info.blendConstants[2] = 0.f;
	config_info.color_blend_info.blendConstants[3] = 0.f;

	// DEPTH INFO
	config_info.depth_stencil_info = vkinit::depth_stencil_ci(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	// DYNAMIC STATE
	config_info.dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	config_info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	config_info.dynamic_state_info.pDynamicStates = config_info.dynamic_state_enables.data();
	config_info.dynamic_state_info.dynamicStateCount =
		static_cast<uint32_t>(config_info.dynamic_state_enables.size());
	config_info.dynamic_state_info.flags = 0;
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
	assert(config_info.pipeline_layout != VK_NULL_HANDLE &&
		"ERROR! coral_pipeline::create_graphics_pipeline() >> no pipeline layout provided in config_info ");

	assert(config_info.render_pass != VK_NULL_HANDLE &&
		"ERROR! coral_pipeline::create_graphics_pipeline() >> no render pass provided in config_info ");

	auto vert_code = read_file(vert_file_path);
	auto frag_code = read_file(frag_file_path);

	create_shader_module(vert_code, &vert_shader_module_);
	create_shader_module(frag_code, &frag_shader_module_);

	VkPipelineShaderStageCreateInfo shader_stages[2];

	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].pNext = nullptr;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = vert_shader_module_;
	shader_stages[0].pName = "main";
	shader_stages[0].flags = 0;
	shader_stages[0].pSpecializationInfo = nullptr;

	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].pNext = nullptr;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = frag_shader_module_;
	shader_stages[1].pName = "main";
	shader_stages[1].flags = 0;
	shader_stages[1].pSpecializationInfo = nullptr;

	// VERTEX INPUT INFO
	VkPipelineVertexInputStateCreateInfo vertex_input_info{ vkinit::vertex_input_state_ci() };
	
	VertexInputDescription vertex_desc{ Vertex::get_vert_desc() };
	
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_desc.attributes.size());
	vertex_input_info.pVertexAttributeDescriptions = vertex_desc.attributes.data();

	vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_desc.bindings_.size());
	vertex_input_info.pVertexBindingDescriptions = vertex_desc.bindings_.data();

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
	pipeline_info.pViewportState = &config_info.viewport_info;
	pipeline_info.pRasterizationState = &config_info.rasterization_info;
	pipeline_info.pMultisampleState = &config_info.multisample_info;
	pipeline_info.pColorBlendState = &config_info.color_blend_info;
	pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
	pipeline_info.pDynamicState = &config_info.dynamic_state_info;

	pipeline_info.layout = config_info.pipeline_layout;
	pipeline_info.renderPass = config_info.render_pass;
	pipeline_info.subpass = config_info.subpass;

	pipeline_info.basePipelineIndex = -1;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_)
		!= VK_SUCCESS)
		throw std::runtime_error("ERROR! coral_pipeline::create_graphics_pipeline() >> Failed to create graphics pipeline!");
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