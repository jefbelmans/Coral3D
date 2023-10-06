#pragma once

// CORAL
#include "coral_camera.h"
#include "coral_device.h"
#include "coral_pipeline.h"
#include "coral_gameobject.h"
#include "coral_frame_info.h"
#include "coral_descriptors.h"

// STD
#include <memory>
#include <vector>

namespace coral_3d
{
    class point_light_system final
    {
    public:
        point_light_system(coral_device& device, VkRenderPass render_pass, std::vector<VkDescriptorSetLayout>& desc_set_layouts);
        ~point_light_system();

        point_light_system(const point_light_system&) = delete;
        point_light_system& operator=(const point_light_system&) = delete;

        void update(FrameInfo& frame_info, GlobalUBO& ubo);
        void render(FrameInfo& frame_info);

        VkPipelineLayout pipeline_layout() const { return pipeline_layout_; }
        VkPipeline pipeline() const { return pipeline_->pipeline(); }

    private:
        void create_pipeline_layout(coral_device &device, std::vector<VkDescriptorSetLayout>& desc_set_layouts);
        void create_pipeline(VkRenderPass render_pass);

        coral_device& device_;

        std::unique_ptr<coral_pipeline> pipeline_;
        VkPipelineLayout pipeline_layout_;
    };
}