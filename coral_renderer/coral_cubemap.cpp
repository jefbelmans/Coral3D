#include "coral_cubemap.h"
#include "coral_buffer.h"
#include "vk_initializers.h"

#include "stb_image.h"

// STD
#include <iostream>

namespace coral_3d
{
    coral_cubemap::coral_cubemap(coral_device &device, bool nearest_filter)
    : device_(device), file_names_({""}), width_(0), height_(0)
    , bytes_per_pixel_(0), mip_levels_(1), nearest_filter_(nearest_filter), sRGB_(false)
    {}

    coral_cubemap::~coral_cubemap()
    {
        vkDestroyImageView(device_.device(), image_view_, nullptr);
        vkDestroySampler(device_.device(), sampler_, nullptr);
        vmaDestroyImage(device_.allocator(), cubemap_image_.image, cubemap_image_.allocation);
    }

    bool coral_cubemap::init(const std::vector<std::string> &file_names, bool sRGB, bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);
        file_names_ = file_names;
        sRGB_ = sRGB;
        return create();
    }

    bool coral_cubemap::create()
    {
        stbi_uc* pixels;

        // Load first face to get size
        pixels = stbi_load(file_names_[0].c_str(), &width_, &height_, &bytes_per_pixel_, STBI_rgb_alpha);
        if (!pixels)
        {
            std::cout << "Failed to load texture file " << file_names_[0] << std::endl;
            return false;
        }

        VkDeviceSize layer_size = width_ * height_ * 4;
        VkDeviceSize image_size = layer_size * NUMBER_OF_CUBEMAP_IMAGES;

        coral_buffer staging_buffer
        {
            device_,image_size,1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        };
        staging_buffer.map();
        staging_buffer.write_to_buffer(pixels, layer_size);

        stbi_image_free(pixels);

        // Load other 5 faces
        for (int i = 1; i < NUMBER_OF_CUBEMAP_IMAGES; ++i)
        {
            pixels = stbi_load(file_names_[i].c_str(), &width_, &height_, &bytes_per_pixel_, STBI_rgb_alpha);
            if (!pixels)
            {
                std::cout << "Failed to load texture file " << file_names_[i] << std::endl;
                return false;
            }
            staging_buffer.write_to_buffer(pixels, layer_size, layer_size * i);
            stbi_image_free(pixels);
        }

        // CREATE IMAGE
        image_format_ = sRGB_ ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        create_image(image_format_);

        // Transition and copy image to buffer
        device_.transition_image_layout(cubemap_image_.image, image_format_, VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, NUMBER_OF_CUBEMAP_IMAGES, mip_levels_);
        device_.copy_buffer_to_image(staging_buffer.get_buffer(), cubemap_image_, width_, height_,
                                     NUMBER_OF_CUBEMAP_IMAGES);
        device_.transition_image_layout(cubemap_image_.image, image_format_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, NUMBER_OF_CUBEMAP_IMAGES, mip_levels_);
        image_layout_ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        create_image_sampler();
        create_image_view();

        descriptor_image_info_.sampler = sampler_;
        descriptor_image_info_.imageLayout = image_layout_;
        descriptor_image_info_.imageView = image_view_;

        return true;
    }

    bool coral_cubemap::create_image(VkFormat format)
    {
        VkExtent3D img_extent;
        img_extent.width = static_cast<uint32_t>(width_);
        img_extent.height = static_cast<uint32_t>(height_);
        img_extent.depth = 1;

        VkImageCreateInfo create_info{vkinit::image_ci(format, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT, img_extent)};
        create_info.mipLevels = mip_levels_;
        create_info.arrayLayers = NUMBER_OF_CUBEMAP_IMAGES;
        create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        VmaAllocationCreateInfo alloc_create_info{};
        alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(device_.allocator(), &create_info, &alloc_create_info,
                       &cubemap_image_.image, &cubemap_image_.allocation, nullptr);

        return true;
    }

    bool coral_cubemap::create_image_view()
    {
        VkImageViewCreateInfo image_info{ vkinit::image_view_ci(image_format_, cubemap_image_.image, VK_IMAGE_ASPECT_COLOR_BIT) };
        image_info.subresourceRange.levelCount = mip_levels_;
        image_info.subresourceRange.layerCount = NUMBER_OF_CUBEMAP_IMAGES;
        image_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

        if (vkCreateImageView(device_.device(), &image_info, nullptr, &image_view_) != VK_SUCCESS)
            throw std::runtime_error("ERROR! coral_cubemap::create_inage_view() >> Failed to create image view!");

        return true;
    }

    bool coral_cubemap::create_image_sampler()
    {
        VkFilter filter = nearest_filter_ ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
        VkSamplerCreateInfo sampler_info{ vkinit::sampler_ci(filter, device_.properties.limits.maxSamplerAnisotropy) };
        sampler_info.maxLod = static_cast<float>(mip_levels_);

        if (vkCreateSampler(device_.device(), &sampler_info, nullptr, &sampler_) != VK_SUCCESS)
            throw std::runtime_error("ERROR!  coral_cubemap::create_image_sampler() >> Failed to create texture sampler!");

        return true;
    }
} // coral_3d