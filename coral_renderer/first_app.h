#pragma once

#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_swapchain.h"
#include "coral_window.h"
#include "coral_mesh.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class first_app final
	{
	public:
		static constexpr int WIDTH{ 800 };
		static constexpr int HEIGHT{ 600 };

		first_app();
		~first_app();

		first_app(const first_app&) = delete;
		first_app& operator=(const first_app&) = delete;

		void run();

	private:
		void load_meshes();
		void create_pipeline_layout();
		void create_pipeline();
		void create_command_buffers();
		void draw_frame();

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_swapchain swapchain_{ device_, window_.get_extent() };
		std::unique_ptr<coral_pipeline> pipeline_;

		VkPipelineLayout pipeline_layout_;
		std::vector<VkCommandBuffer> command_buffers_;

		std::unique_ptr<coral_mesh> mesh_;
	};
}