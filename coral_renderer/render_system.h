#pragma once

#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_gameobject.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class render_system final
	{
	public:
		render_system(coral_device& device, VkRenderPass render_pass);
		~render_system();

		render_system(const render_system&) = delete;
		render_system& operator=(const render_system&) = delete;

		void render_gameobjects(VkCommandBuffer command_buffer, std::vector<coral_gameobject>& gameobjects);

	private:
		void create_pipeline_layout();
		void create_pipeline(VkRenderPass render_pass);

		coral_device& device_;

		std::unique_ptr<coral_pipeline> pipeline_;
		VkPipelineLayout pipeline_layout_;

	};
}