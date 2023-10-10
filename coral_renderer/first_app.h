#pragma once

#include "coral_device.h"
#include "coral_window.h"
#include "coral_gameobject.h"
#include "coral_renderer.h"
#include "coral_descriptors.h"
#include "coral_texture.h"
#include "coral_cubemap.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
	class first_app final
	{
	public:
		static constexpr int WIDTH{ 1280 };
		static constexpr int HEIGHT{ 720 };

		first_app();

		first_app(const first_app&) = delete;
		first_app& operator=(const first_app&) = delete;

		void run();

	private:
		void load_gameobjects(coral_descriptor_set_layout& material_set_layout, VkPipelineLayout pipeline_layout,
                              coral_buffer& global_ubo);

        void init_imgui();
        std::unique_ptr<coral_descriptor_pool> imgui_pool_{};

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_renderer renderer_{ window_, device_ };

        coral_cubemap cubemap_;

		std::unique_ptr<coral_descriptor_pool> descriptor_pool_{};

        // GLOBAL DESCRIPTOR SETS
        std::vector<VkDescriptorSet> global_descriptor_sets_{coral_swapchain::MAX_FRAMES_IN_FLIGHT};
        std::unique_ptr<coral_descriptor_set_layout> global_set_layout_;

		coral_gameobject::Map gameobjects_;
	};
}