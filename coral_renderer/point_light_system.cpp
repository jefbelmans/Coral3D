#include "point_light_system.h"
#include "vk_initializers.h"

using namespace coral_3d;

struct PointLightPushConstant
{
    glm::vec4 position{}; // w is radius
    glm::vec4 color{}; // w is intensity
};

point_light_system::point_light_system(coral_device& device, VkRenderPass render_pass,  std::vector<VkDescriptorSetLayout>& desc_set_layouts)
        : device_{device}
{
    create_pipeline_layout(device_, desc_set_layouts);
    create_pipeline(render_pass);
}

point_light_system::~point_light_system()
{
    vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
}

void point_light_system::update(FrameInfo &frame_info, GlobalUBO &ubo)
{
    auto rotation = glm::rotate(glm::mat4(1.f), frame_info.frame_time, {0.f, -1.f, 0.f});
    int light_index{0};
    for(auto& kv: frame_info.gameobjects)
    {
        auto& obj = kv.second;
        if(obj->point_light_ == nullptr) continue;

        obj->transform_.translation = glm::vec3(rotation * glm::vec4(obj->transform_.translation, 1.f));

        // Copy light to UBO
        ubo.point_lights[light_index].position = glm::vec4(obj->transform_.translation, obj->transform_.scale.x);
        ubo.point_lights[light_index].color = obj->point_light_->color;

        light_index++;
    }
    ubo.num_lights = light_index;
}

void point_light_system::render(FrameInfo& frame_info)
{
    pipeline_->bind(frame_info.command_buffer);

    vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout_,
            0, 1,
            &frame_info.global_descriptor_set,
            0, nullptr
    );

    for(auto& kv: frame_info.gameobjects)
    {
        auto &obj = kv.second;
        if (obj->point_light_ == nullptr) continue;

        PointLightPushConstant push_constant{};
        push_constant.position = glm::vec4(obj->transform_.translation, obj->transform_.scale.x);
        push_constant.color = obj->point_light_->color;

        vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout_,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, sizeof (PointLightPushConstant),
                &push_constant
                );

        vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
    }
}

void point_light_system::create_pipeline_layout(coral_device &device, std::vector<VkDescriptorSetLayout>& desc_set_layouts)
{
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PointLightPushConstant);

    VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_constant_range;
    layout_info.setLayoutCount = static_cast<uint32_t>(desc_set_layouts.size());
    layout_info.pSetLayouts = desc_set_layouts.data();

    if (vkCreatePipelineLayout(device.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
        throw std::runtime_error("ERROR! point_light_system::create_pipeline_layout() >> Failed to create pipeline layout!");
}

void point_light_system::create_pipeline(VkRenderPass render_pass)
{
    PipelineConfigInfo config_info{};
    coral_pipeline::default_pipeline_config_info(config_info);
    config_info.binding_descriptions.clear();
    config_info.attribute_descriptions.clear();
    config_info.render_pass = render_pass;
    config_info.pipeline_layout = pipeline_layout_;

    pipeline_ = std::make_unique<coral_pipeline>(
            device_,
            "assets/shaders/point_light.vert.spv",
            "assets/shaders/point_light.frag.spv",
            config_info
            );
}