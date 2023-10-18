#ifndef CORAL_SHADOWMAP_RENDERER_H
#define CORAL_SHADOWMAP_RENDERER_H

#include "vk_types.h"
#include "coral_device.h"

namespace coral_3d
{
    class coral_shadowmap_renderer final
    {
    public:
        coral_shadowmap_renderer(coral_device& device);
        ~coral_shadowmap_renderer();

    private:
        void create_render_pass();
        void create_framebuffer();

        coral_device& device_;

        unsigned int width_;
        unsigned int height_;

        VkFramebuffer frame_buffer_;
        struct FrameBufferAttachment
        {
            AllocatedImage image;
            VkImageView image_view;
        } frame_buffer_attachment_;
        VkRenderPass render_pass_;
        VkSampler depth_sampler_;
        VkDescriptorImageInfo descriptor;
    };

} // coral_3d

#endif // CORAL_SHADOWMAP_RENDERER_H
