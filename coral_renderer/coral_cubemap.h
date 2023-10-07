#ifndef CORAL_CUBEMAP_H
#define CORAL_CUBEMAP_H

// STD
#include <vector>
#include <string>

#include "coral_device.h"

namespace coral_3d
{
    class coral_cubemap final
    {
    public:
        explicit coral_cubemap(coral_device &device, bool nearest_filter);
        ~coral_cubemap();

        bool init(const std::vector<std::string>& file_names, bool sRGB, bool flip = false);

        // GETTERS
        int get_width() const {return width_;};
        int get_height() const { return height_; }
        const VkDescriptorImageInfo get_descriptor_image_info() const { return descriptor_image_info_; }

    private:
        bool create();
        bool create_image(VkFormat format);
        bool create_image_view();
        bool create_image_sampler();

        static constexpr int NUMBER_OF_CUBEMAP_IMAGES = 6;

        coral_device& device_;

        std::vector<std::string> file_names_;
        int width_, height_, bytes_per_pixel_;
        int mip_levels_;

        bool nearest_filter_, sRGB_;

        VkFormat image_format_;
        AllocatedImage cubemap_image_;
        VkImageLayout image_layout_;
        VkImageView image_view_;
        VkSampler sampler_;

        VkDescriptorImageInfo descriptor_image_info_;

    };
} // coral_3d

#endif //CORAL_CUBEMAP_H
