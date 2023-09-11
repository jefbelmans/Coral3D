#pragma once

// CORAL
#include "coral_descriptors.h"
#include "coral_texture.h"
#include "tiny_obj_loader.h"

// STD
#include <memory>
#include <utility>
#include "vulkan/vulkan.h"

namespace coral_3d
{
    class coral_material final
    {
    public:
        coral_material(coral_device& device, tinyobj::material_t tiny_obj_material) : device_{device}, tiny_obj_material_{std::move(tiny_obj_material)}
        {};

        coral_material(const coral_material&) = delete;
        coral_material& operator=(const coral_material&) = delete;

        void load(
                VkPipelineLayout pipeline_layout,
                coral_descriptor_set_layout& material_set_layout,
                coral_descriptor_pool& material_set_pool);

        void bind(VkCommandBuffer command_buffer);

        VkPipelineLayout pipeline_layout() const { return pipeline_layout_; }
        VkDescriptorSet texture_desc_set() const { return texture_desc_set_; }

    private:
        coral_device& device_;
        VkPipelineLayout pipeline_layout_;

        std::shared_ptr<coral_texture> texture_;
        VkDescriptorSet texture_desc_set_;

        tinyobj::material_t tiny_obj_material_;
    };
}
