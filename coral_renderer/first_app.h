#pragma once

#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_swapchain.h"
#include "coral_window.h"
#include "coral_mesh.h"
#include "coral_gameobject.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	struct PushConstant
	{
		alignas(16) glm::mat3 transform;
		alignas(16) glm::vec3 offset;
		alignas(16) glm::vec3 color;
	};

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
		void load_gameobjects();
		void create_pipeline_layout();
		void create_pipeline();
		void create_command_buffers();
		void free_command_buffers();
		void draw_frame();
		void recreate_swapchain();
		void record_command_buffer(int image_index);
		void render_gameobjects(VkCommandBuffer command_buffer);

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		std::unique_ptr<coral_swapchain> swapchain_;
		std::unique_ptr<coral_pipeline> pipeline_;
		VkPipelineLayout pipeline_layout_;
		std::vector<VkCommandBuffer> command_buffers_;
		std::vector<coral_gameobject> gameobjects_;
	};
}