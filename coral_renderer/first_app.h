#pragma once

#include "coral_window.h"
#include "coral_pipeline.h"

namespace coral_3d
{
	class first_app final
	{
	public:
		static constexpr int WIDTH{ 800 };
		static constexpr int HEIGHT{ 600 };

		void run();

	private:
		coral_window coral_window_{ WIDTH, HEIGHT, "Coral Renderer" };
		coral_pipeline coral_pipeline_{"shaders/PosNormCol.vert.spv", "shaders/PosNormCol.frag.spv"};

	};
}