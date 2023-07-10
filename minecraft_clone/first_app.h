#pragma once

#include "coral_device.h"
#include "coral_window.h"
#include "coral_gameobject.h"
#include "coral_renderer.h"
#include "coral_descriptors.h"
#include "coral_texture.h"

#include "chunk.h"

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
		void load_chunks();

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_renderer renderer_{ window_, device_ };

		std::unique_ptr<coral_texture> atlas_texture_;
		std::unique_ptr<coral_descriptor_pool> global_descriptor_pool_{};

		std::vector<chunk> chunks_;
	};
}