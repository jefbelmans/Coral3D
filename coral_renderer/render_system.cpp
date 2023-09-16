#include "render_system.h"

#include "vk_initializers.h"
#include <stdexcept>
#include <utility>

using namespace coral_3d;

render_system::render_system(coral_device& device, VkPipelineLayout pipeline_layout)
	: device_{device}
    , pipeline_layout_ {pipeline_layout}
{}

render_system::~render_system()
{
	vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void render_system::render_gameobjects(FrameInfo& frame_info)
{
	vkCmdBindDescriptorSets(
		frame_info.command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_layout_,
		0, 1,
        &frame_info.global_descriptor_set,
		0, nullptr
	);

	for (auto& kv : frame_info.gameobjects)
	{
		auto& obj{ kv.second };
		if (obj.mesh_ == nullptr) continue;

		obj.mesh_->draw(frame_info.command_buffer, pipeline_layout_);
	}
}