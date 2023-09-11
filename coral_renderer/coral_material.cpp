#include "coral_material.h"

#include <utility>
#include <iostream>

using namespace coral_3d;

void coral_material::load(VkPipelineLayout pipeline_layout,
                          coral_descriptor_set_layout &material_set_layout,
                          coral_descriptor_pool &material_set_pool)
{
    pipeline_layout_ = pipeline_layout;

    std::cout << "Material path:" << tiny_obj_material_.diffuse_texname << std::endl;

    // LOAD TEXTURES
    texture_ = coral_texture::create_texture_from_file(
            device_,
            "assets/textures/sponza_floor_a_diff.png",
            VK_FORMAT_R8G8B8A8_SRGB
    );

    // WRITE TEXTURES TO DESCRIPTOR
    auto image_info = texture_->get_descriptor_info();
    coral_descriptor_writer(material_set_layout, material_set_pool)
            .write_image(0, &image_info)
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


