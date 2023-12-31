#include "first_app.h"

#include "render_system.h"
#include "point_light_system.h"
#include "skybox_system.h"
#include "coral_camera.h"
#include "coral_buffer.h"
#include "vk_initializers.h"

// STD
#include <chrono>
#include <iostream>

// IMGUI
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

using namespace coral_3d;

first_app::first_app()
 : cubemap_{device_, true}
{
    init_imgui();
    descriptor_pool_ = coral_descriptor_pool::Builder(device_)
            .set_max_sets(MAX_MATERIAL_SETS)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, coral_swapchain::MAX_FRAMES_IN_FLIGHT)
            .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_MATERIAL_SETS)
            .build();

    input.add_callback(GLFW_KEY_B,
                       coral_input::Callback(GLFW_PRESS, [&](){
                           show_cursor_ = !show_cursor_;
                           glfwSetInputMode(window_.get_glfw_window(), GLFW_CURSOR,
                         show_cursor_ ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }));
}

void first_app::run()
{
    // Create buffer that is mapped to the global descriptor
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

    // Set 0: Global descriptor set (Scene data)
    global_set_layout_ = coral_descriptor_set_layout::Builder(device_)
		.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();


    // Set 1: Material descriptor set
    auto material_set_layout = coral_descriptor_set_layout::Builder(device_)
            .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .add_binding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    // Combined descriptor set layouts
    std::vector<VkDescriptorSetLayout> desc_set_layouts
    {
        global_set_layout_->get_descriptor_set_layout(),
        material_set_layout->get_descriptor_set_layout()
    };

    // RENDER SYSTEM
    render_system render_system{device_, desc_set_layouts};
    load_gameobjects(*material_set_layout, render_system.pipeline_layout(), global_ubo);

    // POINT LIGHT SYSTEM
    desc_set_layouts.pop_back();
    point_light_system point_light_system{device_, renderer_.get_swapchain_render_pass(), desc_set_layouts};

    // SKYBOX SYSTEM
    skybox_system skybox_system{device_, renderer_.get_swapchain_render_pass(), desc_set_layouts};

    // CAMERA
    coral_camera camera{ {0.f, 0.f, 3.f} };
    camera.set_view_direction(camera.get_position(), {0.f, 0.f, -1.f});

    auto last_time{ std::chrono::high_resolution_clock::now() };
	while (!window_.should_close())
	{
        // UPDATE INPUT
		glfwPollEvents();

        auto current_time{ std::chrono::high_resolution_clock::now() };
        auto frame_time{ std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count() };
        last_time = current_time;

        // MOVE CAMERA
        if(!show_cursor_)
        {
            camera.update_input(window_.get_glfw_window(), frame_time);
        }

        float aspect{ renderer_.get_aspect_ratio() };
        camera.set_perspective_projection(glm::radians(60.f), aspect, 0.1f, 1000.f);

        // IMGUI FRAME
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

		if (auto command_buffer = renderer_.begin_frame())
		{
            const int frame_index{ renderer_.get_frame_index() };
            auto& obj = gameobjects_.at(0);

            FrameInfo frame_info
            {
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets_[frame_index],
                gameobjects_
            };

            // UPDATE
            GlobalUBO ubo{};
            ubo.view = camera.get_view();
            ubo.view_inverse = glm::inverse(camera.get_view());
            ubo.view_projection = camera.get_projection() * camera.get_view();
            point_light_system.update(frame_info, ubo);
            global_ubo.write_to_index(&ubo, frame_index);
            global_ubo.flush_index(frame_index);

            // RENDER
			renderer_.begin_swapchain_render_pass(command_buffer);
            // IMGUI
            ImGui::Render();
            skybox_system.render(frame_info);
			render_system.render_gameobjects(frame_info);
            point_light_system.render(frame_info);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
			renderer_.end_swapchain_render_pass(command_buffer);
			renderer_.end_frame();
		}
	}

	vkDeviceWaitIdle(device_.device());
}

void first_app::load_gameobjects(coral_descriptor_set_layout& material_set_layout, VkPipelineLayout pipeline_layout,
                                 coral_buffer& global_ubo)
{
    // GAMEOBJECTS
    auto sponza_scene{std::make_shared<coral_gameobject>(coral_gameobject::create_gameobject()) };

    // MESHES
    std::shared_ptr<coral_mesh> sponza_mesh
    {
        coral_mesh::create_mesh_from_file(device_,"assets/meshes/SciFiHelmet/SciFiHelmet.gltf", sponza_scene.get())
    };

    // CREATE MESH MATERIALS AND PIPELINES
    sponza_mesh->load_materials(material_set_layout, *descriptor_pool_);
    sponza_mesh->create_pipelines(
            "assets/shaders/simple_shader.vert.spv",
            "assets/shaders/simple_shader.frag.spv",
            renderer_.get_swapchain_render_pass(),
            pipeline_layout);

    sponza_scene->mesh_ = sponza_mesh;
    sponza_scene->transform_.translation = glm::vec3(0.f, 0.f, 0.f);
    gameobjects_.emplace(sponza_scene->get_id(), sponza_scene);

    // LIGHTS
    auto point_light = std::make_shared<coral_gameobject>(coral_gameobject::create_point_light(1.f, 0.1f, {1.f, 0.2f, 0.2f}));
    point_light->transform_.translation = glm::vec3(-4.f, 0.f, 0.f);
    gameobjects_.emplace(point_light->get_id(), point_light);

    point_light = std::make_shared<coral_gameobject>(coral_gameobject::create_point_light(1.f, 0.1f, {0.2f, 1.f, 0.2f}));
    point_light->transform_.translation = glm::vec3(-2.f, 0.f, 0.f);
    gameobjects_.emplace(point_light->get_id(), point_light);

    point_light = std::make_shared<coral_gameobject>(coral_gameobject::create_point_light(1.f));
    point_light->transform_.translation = glm::vec3(0.f, 0.f, 0.f);
    gameobjects_.emplace(point_light->get_id(), point_light);

    point_light = std::make_shared<coral_gameobject>(coral_gameobject::create_point_light(1.f, 0.1f, {0.2f, 0.2f, 1.f}));
    point_light->transform_.translation = glm::vec3(2.f, 0.f, 0.f);
    gameobjects_.emplace(point_light->get_id(), point_light);

    point_light = std::make_shared<coral_gameobject>(coral_gameobject::create_point_light(1.f, 0.1f, {1.f, 0.2f, 1.f}));
    point_light->transform_.translation = glm::vec3(4.f, 0.f, 0.f);
    gameobjects_.emplace(point_light->get_id(), point_light);

    // SKYBOX
    std::vector<std::string> file_names
    {
        "assets/textures/cubemap/posx.jpg",
        "assets/textures/cubemap/negx.jpg",
        "assets/textures/cubemap/negy.jpg",
        "assets/textures/cubemap/posy.jpg",
        "assets/textures/cubemap/posz.jpg",
        "assets/textures/cubemap/negz.jpg"
    };
    cubemap_.init(file_names,true, true);

    // Write global UBO
    for (size_t i = 0; i < global_descriptor_sets_.size(); i++)
    {
        auto buffer_info = global_ubo.descriptor_info_index(i);
        auto skybox_info =  cubemap_.get_descriptor_image_info();

        coral_descriptor_writer(*global_set_layout_, *descriptor_pool_)
                .write_buffer(0, &buffer_info)
                .write_image(1, &skybox_info)
                .build(global_descriptor_sets_[i]);
    }
}

void first_app::init_imgui()
{
    // Create descriptor pool for IMGUI
    imgui_pool_ = coral_descriptor_pool::Builder(device_)
            .set_max_sets(MAX_MATERIAL_SETS)
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
            .add_pool_size(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
            .build();

    // Initialize ImGui
    ImGui::CreateContext();

    // Initialize ImGui for GLFW
    ImGui_ImplGlfw_InitForVulkan(window_.get_glfw_window(), true);

    // Initialize ImGui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device_.instance();
    init_info.PhysicalDevice = device_.physical_device();
    init_info.Device = device_.device();
    init_info.Queue = device_.graphics_queue();
    init_info.DescriptorPool = imgui_pool_->pool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, renderer_.get_swapchain_render_pass());

    // Execute GPU command to upload font textures
    device_.immediate_submit([&](VkCommandBuffer cmd){
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    // Delete font textures from the CPU
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    device_.deletion_queue().deletors.emplace_back([=](){
        ImGui_ImplVulkan_Shutdown();
    });
}