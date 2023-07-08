#pragma once

#include "coral_camera.h"
#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_gameobject.h"
#include "coral_frame_info.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class render_system final
	{
	public:
		render_system(coral_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~render_system();

		render_system(const render_system&) = delete;
		render_system& operator=(const render_system&) = delete;

		void render_gameobjects(FrameInfo& frame_info);

	private:
		void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
		void create_pipeline(VkRenderPass render_pass);

		coral_device& device_;

		std::unique_ptr<coral_pipeline> pipeline_;
		VkPipelineLayout pipeline_layout_;

	};
}