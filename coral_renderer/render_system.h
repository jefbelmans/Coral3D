#pragma once

#include "coral_camera.h"
#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_gameobject.h"
#include "coral_frame_info.h"
#include "coral_descriptors.h"
#include "coral_texture.h"
#define MAX_MATERIAL_SETS 4096
#define MAX_TEXTURES 1

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class render_system final
	{
	public:
		render_system(coral_device& device, VkRenderPass render_pass, std::vector<VkDescriptorSetLayout> desc_set_layouts);
		~render_system();

		render_system(const render_system&) = delete;
		render_system& operator=(const render_system&) = delete;

		void render_gameobjects(FrameInfo& frame_info);

		VkPipelineLayout pipeline_layout() const { return pipeline_layout_; }
		VkPipeline pipeline() const { return pipeline_->pipeline(); }

	private:
		void create_pipeline_layout(std::vector<VkDescriptorSetLayout> desc_set_layouts);
		void create_pipeline(VkRenderPass render_pass);

		coral_device& device_;

		std::unique_ptr<coral_pipeline> pipeline_;
		VkPipelineLayout pipeline_layout_;
	};
}