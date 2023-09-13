#include "first_app.h"

#include "render_system.h"
#include "coral_camera.h"
#include "coral_buffer.h"

// STD
#include <array>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "vk_initializers.h"
#include "coral_texture.h"

using namespace coral_3d;

first_app::first_app()
{
    global_descriptor_pool_ = coral_descriptor_pool::Builder(device_)
        .set_max_sets(coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
        .build();

    material_descriptor_pool_ = coral_descriptor_pool::Builder(device_)
            .set_max_sets(MAX_MATERIAL_SETS)
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLER, 1)
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_MATERIAL_SETS)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
            .build();
}

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

    // Set 0: Global descriptor sets
    auto global_set_layout = coral_descriptor_set_layout::Builder(device_)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    std::vector<VkDescriptorSet> global_descriptor_sets{coral_swapchain::MAX_FRAMES_IN_FLIGHT};
    for (size_t i = 0; i < global_descriptor_sets.size(); i++)
    {
        auto buffer_info = global_ubo.descriptor_info_index(i);

        coral_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
            .write_buffer(0, &buffer_info)
            .build(global_descriptor_sets[i]);
    }

    // Set 1: Material descriptor set
    auto material_set_layout = coral_descriptor_set_layout::Builder(device_)
            .add_binding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_binding(1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURES)
            .add_binding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    // Combined descriptor set layouts
    std::vector<VkDescriptorSetLayout> desc_set_layouts
    {
        global_set_layout->get_descriptor_set_layout(),
        material_set_layout->get_descriptor_set_layout()
    };

    render_system render_system{ device_, renderer_.get_swapchain_render_pass(), desc_set_layouts};
    coral_camera camera{ {0.f, 2.5f, 0.f} };

    load_gameobjects(render_system.pipeline_layout(), *material_set_layout);

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

            FrameInfo frame_info
            {
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index],
                gameobjects_
            };

            // UPDATE GLOBAL UBO
            GlobalUBO ubo{};
            ubo.view = camera.get_view();
            ubo.view_inverse = glm::inverse(camera.get_view());
            ubo.view_projection = camera.get_projection() * camera.get_view();
            global_ubo.write_to_index(&ubo, frame_index);
            global_ubo.flush_index(frame_index);

            // RENDER
			renderer_.begin_swapchain_render_pass(command_buffer);
			render_system.render_gameobjects(frame_info);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::load_gameobjects(VkPipelineLayout pipeline_layout, coral_descriptor_set_layout& material_set_layout)
{
#pragma region Sponza
    std::shared_ptr<coral_mesh> sponza_mesh{ coral_mesh::create_mesh_from_file(device_, "assets/meshes/sponza.obj") };
    sponza_mesh->load_materials(pipeline_layout, material_set_layout, *material_descriptor_pool_);

    auto sponza_scene{ coral_gameobject::create_gameobject() };
    sponza_scene.mesh_ = sponza_mesh;
    sponza_scene.transform_.translation = { 0.f, 0.f, 0.f };
    gameobjects_.emplace(sponza_scene.get_id(), std::move(sponza_scene));

#pragma endregion
}