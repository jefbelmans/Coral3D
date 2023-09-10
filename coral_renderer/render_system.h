#pragma once

#include "coral_camera.h"
#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_gameobject.h"
#include "coral_frame_info.h"
#include "coral_descriptors.h"
#include "coral_texture.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class render_system final
	{
	public:
		render_system(coral_device& device) : device_{ device } {};
		render_system(coral_device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~render_system();
		render_system(const render_system&) = delete;
		render_system& operator=(const render_system&) = delete;

		void render_gameobjects(FrameInfo& frame_info);

		VkPipelineLayout pipeline_layout() const { return pipeline_layout_; }
		VkPipeline pipeline() const { return pipeline_->pipeline(); }

	private:
		void create_pipeline_layout(VkDescriptorSetLayout global_set_layout);
		void create_pipeline(VkRenderPass render_pass);

		coral_device& device_;

		std::unique_ptr<coral_pipeline> pipeline_;
		VkPipelineLayout pipeline_layout_;

        std::unique_ptr<coral_texture> test_texture_;
        std::unique_ptr<coral_texture> test_texture_2_;

        std::unique_ptr<coral_descriptor_pool> material_descriptor_pool_{};
        std::unique_ptr<coral_descriptor_set_layout> material_set_layout_{};
        VkDescriptorSet material_descriptor_set{};
	};
}