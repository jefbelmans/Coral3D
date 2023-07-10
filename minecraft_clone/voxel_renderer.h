#pragma once

// LIBS
#include <vulkan/vulkan.h>

// STD
#include <vector>

// CORAL
#include "coral_device.h"
#include "coral_pipeline.h"

#include "chunk.h"

struct VoxelRenderInfo
{
	int frame_index{};
	float frame_time{};
	VkCommandBuffer command_buffer{};
	VkDescriptorSet global_descriptor_set{};
	std::vector<chunk> chunks{};
};

class voxel_renderer final
{
public:
	voxel_renderer(coral_3d::coral_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
	~voxel_renderer();

	voxel_renderer(const voxel_renderer&) = delete;
	voxel_renderer& operator=(const voxel_renderer&) = delete;

	void render_chunks(VoxelRenderInfo& render_info);
private:
	void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
	void create_pipeline(VkRenderPass render_pass);

	coral_3d::coral_device& device_;

	std::unique_ptr<coral_3d::coral_pipeline> pipeline_;
	VkPipelineLayout pipeline_layout_;
};