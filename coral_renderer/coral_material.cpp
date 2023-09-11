#include "coral_material.h"
#include "vk_initializers.h"

// STD
#include <utility>
#include <iostream>

using namespace coral_3d;

void coral_material::load(VkPipelineLayout pipeline_layout,
                          coral_descriptor_set_layout &material_set_layout,
                          coral_descriptor_pool &material_set_pool)
{
    pipeline_layout_ = pipeline_layout;

    // LOAD TEXTURES
    texture_ = coral_texture::create_texture_from_file(
            device_,
            "assets/textures/" + tiny_obj_material_.diffuse_texname,
            VK_FORMAT_R8G8B8A8_SRGB
    );

    // IMAGE SAMPLER
    VkSamplerCreateInfo sampler_create_info{ vkinit::sampler_ci(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR, 16.f)};

    VkSampler sampler;
    vkCreateSampler(device_.device(), &sampler_create_info, nullptr, &sampler);

    // WRITE SAMPLER AND TEXTURES TO DESCRIPTOR
    VkDescriptorImageInfo sampler_info{};
    sampler_info.sampler = sampler;
    auto image_info = texture_->get_descriptor_info();

    coral_descriptor_writer(material_set_layout, material_set_pool)
            .write_sampler(0, &sampler_info)
            .write_image(1, &image_info)
            .build(texture_desc_set_);
}

void coral_material::bind(VkCommandBuffer command_buffer)
{
	// Only bind texture descriptor set if it exists
	if(texture_desc_set_ != VK_NULL_HANDLE)
		vkCmdBindDescriptorSets
        (command_buffer,
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         pipeline_layout_,
         1, 1,
         &texture_desc_set_,
         0, nullptr);
}


