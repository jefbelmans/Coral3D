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

    // WRITE TO MATERIAL BUFFER
    MaterialUBO ubo{};

    // DIFFUSE
    ubo.use_diff_map = !tiny_obj_material_.diffuse_texname.empty();
    ubo.diffuse_color.x = tiny_obj_material_.diffuse[0];
    ubo.diffuse_color.y = tiny_obj_material_.diffuse[1];
    ubo.diffuse_color.z = tiny_obj_material_.diffuse[2];

    // SPECULAR
    ubo.use_specular_map = !tiny_obj_material_.specular_texname.empty();
    ubo.specular_color.x = tiny_obj_material_.specular[0];
    ubo.specular_color.y = tiny_obj_material_.specular[1];
    ubo.specular_color.z = tiny_obj_material_.specular[2];
    ubo.shininess = tiny_obj_material_.shininess;

    // BUMP
    ubo.use_bump_map = !tiny_obj_material_.bump_texname.empty();

    // ALPHA BLENDING
    ubo.use_opacity_map = !tiny_obj_material_.alpha_texname.empty();
    ubo.opacity_value = 100.f;

    material_ubo_.map();
    material_ubo_.write_to_buffer(&ubo);
    material_ubo_.flush();
    material_ubo_.unmap();

    // BUILD DESCRIPTOR
    VkDescriptorImageInfo sampler_info{};
    sampler_info.sampler = texture_->sampler();
    auto image_info = texture_->get_descriptor_info();
    auto buffer_info = material_ubo_.descriptor_info();

    coral_descriptor_writer(material_set_layout, material_set_pool)
            .write_sampler(0, &sampler_info)
            .write_image(1, &image_info)
            .write_buffer(2, &buffer_info)
            .build(material_desc_set_);
}

void coral_material::bind(VkCommandBuffer command_buffer)
{
	// Only bind texture descriptor set if it exists
	if(material_desc_set_ != VK_NULL_HANDLE)
		vkCmdBindDescriptorSets
        (command_buffer,
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         pipeline_layout_,
         1, 1,
         &material_desc_set_,
         0, nullptr);
}


