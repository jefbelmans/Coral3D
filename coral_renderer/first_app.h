#pragma once

#include "coral_device.h"
#include "coral_window.h"
#include "coral_gameobject.h"
#include "coral_renderer.h"

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
		void load_gameobjects();

		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_renderer renderer_{ window_, device_ };

		std::vector<coral_gameobject> gameobjects_;
	};
}