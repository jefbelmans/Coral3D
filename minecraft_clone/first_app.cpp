#include "first_app.h"

// CORAL
#include "coral_camera.h"
#include "coral_buffer.h"
#include "coral_texture.h"

#include "voxel_renderer.h"

// STD
#include <stdexcept>
#include <array>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace coral_3d;

struct GlobalUBO
{
    glm::mat4 view_projeciton{1.f};

    // GLOBAL LIGHT
    glm::vec4 global_light_direction{ glm::normalize(glm::vec4{ 0.577f, 0.377f, -0.577f, 0.f})}; // w is ignored
};

first_app::first_app()
{
    global_descriptor_pool_ = coral_descriptor_pool::Builder(device_)
        .set_max_sets(coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .build();

	generate_world();
}

first_app::~first_app()
{}

void first_app::run()
{
    coral_buffer global_ubo
    {
        device_,
		sizeof(GlobalUBO),
		coral_swapchain::MAX_FRAMES_IN_FLIGHT,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY,
        0,
        device_.properties.limits.minUniformBufferOffsetAlignment
	};
    global_ubo.map();

    auto global_set_layout = coral_descriptor_set_layout::Builder(device_)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(); 

    std::vector<VkDescriptorSet> global_descriptor_sets{coral_swapchain::MAX_FRAMES_IN_FLIGHT};
    for (int i = 0; i < global_descriptor_sets.size(); i++)
    {
        auto buffer_info = global_ubo.descriptor_info_index(i);
        auto image_info = atlas_texture_->get_descriptor_info();

        coral_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
            .write_buffer(0, &buffer_info)
            .write_image(1, &image_info)
            .build(global_descriptor_sets[i]);
    }

	voxel_renderer render_system{ device_, renderer_.get_swapchain_render_pass(), global_set_layout->get_descriptor_set_layout() };
    coral_camera camera{ {0.f, -2.f, 0.f} };

    auto last_time{ std::chrono::high_resolution_clock::now() };

	while (!window_.should_close())
	{
		glfwPollEvents();

        auto current_time{ std::chrono::high_resolution_clock::now() };
        auto frame_time{ std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count() };
        last_time = current_time;

        // MOVE CAMERA
        camera.update_input(window_.get_glfw_window(), frame_time);

        float aspect{ renderer_.get_aspect_ratio() };
        camera.set_perspective_projection(glm::radians(60.f), aspect, 0.1f, 1000.f);

		if (auto command_buffer = renderer_.begin_frame())
		{
            const int frame_index{ renderer_.get_frame_index() };

            VoxelRenderInfo render_info
            {
                frame_index,
                frame_time,
                command_buffer,
                global_descriptor_sets[frame_index],
                world_generator_.get_chunks()
			};

            // UPDATE GLOBAL UBO
            GlobalUBO ubo{};
            ubo.view_projeciton = camera.get_projection() * camera.get_view();
            global_ubo.write_to_index(&ubo, frame_index);
            global_ubo.flush_index(frame_index);

            // UPDATE CHUNKS
           world_generator_.update_world(camera.get_position());

            // RENDER
			renderer_.begin_swapchain_render_pass(command_buffer);
			render_system.render_chunks(render_info);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::generate_world()
{
    world_generator_.generate_world();

    atlas_texture_ = coral_texture::create_texture_from_file(
        device_,
        "assets/textures/atlas_texture.png",
        VK_FORMAT_R8G8B8A8_SRGB
    ); 
}