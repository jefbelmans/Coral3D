#include "skybox_system.h"
#include "vk_initializers.h"

namespace coral_3d
{

    skybox_system::skybox_system(coral_device& device, VkRenderPass render_pass,  std::vector<VkDescriptorSetLayout>& desc_set_layouts)
            : device_{device}
    {
        create_pipeline_layout(device_, desc_set_layouts);
        create_pipeline(render_pass);
    }

    skybox_system::~skybox_system()
    {
        vkDestroyPipelineLayout(device_.device(), pipeline_layout_, nullptr);
    }


    void skybox_system::render(FrameInfo& frame_info)
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

            vkCmdDraw(frame_info.command_buffer, 36, 1, 0, 0);
        }
    }

    void skybox_system::create_pipeline_layout(coral_device &device, std::vector<VkDescriptorSetLayout>& desc_set_layouts)
    {
        VkPipelineLayoutCreateInfo layout_info{ vkinit::pipeline_layout_ci() };
        layout_info.pushConstantRangeCount = 0;
        layout_info.pPushConstantRanges = nullptr;
        layout_info.setLayoutCount = static_cast<uint32_t>(desc_set_layouts.size());
        layout_info.pSetLayouts = desc_set_layouts.data();

        if (vkCreatePipelineLayout(device.device(), &layout_info, nullptr, &pipeline_layout_) != VK_SUCCESS)
            throw std::runtime_error("ERROR! skybox_system::create_pipeline_layout() >> Failed to create pipeline layout!");
    }

    void skybox_system::create_pipeline(VkRenderPass render_pass)
    {
        PipelineConfigInfo config_info{};
        coral_pipeline::default_pipeline_config_info(config_info);
        config_info.binding_descriptions.clear();
        config_info.attribute_descriptions.clear();
        config_info.depth_stencil_info.depthWriteEnable = VK_FALSE;
        config_info.depth_stencil_info.depthTestEnable = VK_FALSE;
        config_info.render_pass = render_pass;
        config_info.pipeline_layout = pipeline_layout_;

        pipeline_ = std::make_unique<coral_pipeline>(
                device_,
                "assets/shaders/skybox.vert.spv",
                "assets/shaders/skybox.frag.spv",
                config_info
        );
    }
} // coral_3d