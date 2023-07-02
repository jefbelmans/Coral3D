#include "vk_engine.h"

int main(int argc, char* argv[])
{
	Coral3D::VulkanEngine engine;

	engine.init();

	engine.run();

	engine.cleanup();

	return 0;
}