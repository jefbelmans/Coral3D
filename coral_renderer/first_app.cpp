#include "first_app.h"

using namespace coral_3d;

void first_app::run()
{
	while (!window_.should_close())
	{
		glfwPollEvents();
	}
}
