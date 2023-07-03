#pragma once

#include "coral_pipeline.h"
#include "coral_window.h"
#include "coral_device.h"

namespace coral_3d
{
	class first_app final
	{
	public:
		static constexpr int WIDTH{ 800 };
		static constexpr int HEIGHT{ 600 };

		void run();

	private:
		coral_window window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_device device_{ window_ };
		coral_pipeline coral_pipeline_{
			device_,
			"shaders/PosNormCol.vert.spv",
			"shaders/PosNormCol.frag.spv",
			coral_pipeline::default_pipeline_config_info(WIDTH, HEIGHT)};
	};
}