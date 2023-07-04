#pragma once

#include "coral_device.h"
#include "coral_swapchain.h"
#include "coral_window.h"

// STD
#include <memory>
#include <vector>
#include <cassert>

namespace coral_3d
{
	class coral_renderer final
	{
	public:
		coral_renderer(coral_window& window, coral_device& device);
		~coral_renderer();

		coral_renderer(const coral_renderer&) = delete;
		coral_renderer& operator=(const coral_renderer&) = delete;

		VkRenderPass get_swapchain_render_pass() const { return swapchain_->get_render_pass(); }
		float get_aspect_ratio() const { return swapchain_->extent_aspect_ratio(); }
		bool is_frame_in_progress() const { return is_frame_started_; }
		
		VkCommandBuffer get_current_command_buffer() const
		{
			assert(is_frame_started_ && "ERROR! coral_renderer::get_current_command_buffer() >> Cannot get buffer when frame not in progress");
			return command_buffers_[current_frame_index_];
		}

		int get_frame_index() const
		{
			assert(is_frame_started_ && "ERROR! coral_renderer::get_frame_index() >> Cannot get frame index when frame not in progress");
			return current_frame_index_;
		}

		VkCommandBuffer begin_frame();
		void end_frame();
		void begin_swapchain_render_pass(VkCommandBuffer command_buffer);
		void end_swapchain_render_pass(VkCommandBuffer command_buffer);

	private:
		void create_command_buffers();
		void free_command_buffers();
		void recreate_swapchain();

		coral_window& window_;
		coral_device& device_;
		std::unique_ptr<coral_swapchain> swapchain_;
		std::vector<VkCommandBuffer> command_buffers_;

		uint32_t current_image_index_;
		int current_frame_index_{ 0 };
		bool is_frame_started_{ false };
	};
}