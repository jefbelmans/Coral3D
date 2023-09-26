#pragma once

#include "coral_device.h"
#include "coral_window.h"
#include "coral_gameobject.h"
#include "coral_renderer.h"
#include "coral_descriptors.h"
#include "coral_texture.h"

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
		void load_gameobjects(coral_descriptor_set_layout& material_set_layout);

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_renderer renderer_{ window_, device_ };

        VkPipelineLayout pipeline_layout_{};

		std::unique_ptr<coral_descriptor_pool> descriptor_pool_{};
		coral_gameobject::Map gameobjects_;
	};
}