#pragma once

#include "coral_device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

#include "vk_types.h"

namespace coral_3d
{

    struct FrameData
    {
        // Sync objects
        VkSemaphore present_semaphore{ VK_NULL_HANDLE }, render_semaphore{ VK_NULL_HANDLE };
        VkFence render_fence{ VK_NULL_HANDLE };

        // Commands
        VkCommandPool command_pool{ VK_NULL_HANDLE };
        VkCommandBuffer main_command_buffer{ VK_NULL_HANDLE };

        // Buffers
        AllocatedBuffer camera_buffer;
        AllocatedBuffer object_buffer;
        AllocatedBuffer indirect_buffer;

        // Descriptors
        VkDescriptorSet global_descriptor;
        VkDescriptorSet object_descriptor;
    };

    class coral_swapchain final
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        coral_swapchain(coral_device& device, VkExtent2D extent, std::shared_ptr<coral_swapchain> old_swapchain);
        coral_swapchain(coral_device& device, VkExtent2D extent);
        ~coral_swapchain();

        coral_swapchain(const coral_swapchain&) = delete;
        void operator=(const coral_swapchain&) = delete;

        VkFramebuffer get_frame_buffers(int index) { return swapchain_frame_buffers_[index]; }
        VkRenderPass get_render_pass() { return render_pass_; }
        VkImageView get_image_view(int index) { return swapchain_image_views_[index]; }
        size_t image_count() { return swapchain_images_.size(); }
        VkFormat get_swapchain_format() { return swapchain_image_format_; }
        VkExtent2D get_swapchain_extent() { return swapchain_extent_; }
        uint32_t width() { return swapchain_extent_.width; }
        uint32_t height() { return swapchain_extent_.height; }

        float extent_aspect_ratio() {
            return static_cast<float>(swapchain_extent_.width) / static_cast<float>(swapchain_extent_.height);
        }
        VkFormat find_depth_format();

        VkResult aqcuire_next_image(uint32_t* image_index);
        VkResult submit_command_buffer(const VkCommandBuffer* buffers, uint32_t* image_index);

        bool compare_swap_formats(const coral_swapchain& swapchain) const
        {
            return
                swapchain.swapchain_depth_format_ == swapchain_depth_format_ &&
                swapchain.swapchain_image_format_ == swapchain_image_format_;
        }

    private:
        void init();
        void create_swapchain();
        void create_image_views();
        void create_render_pass();
        void create_depth_resources();
        void create_frame_buffers();
        void create_sync_structures();

        // Helper functions
        VkSurfaceFormatKHR choose_swapchain_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
        FrameData& get_current_frame() { return frames_[current_frame_]; }

        // Deletion
        DeletionQueue deletion_queue_;

        VkFormat swapchain_image_format_;
        VkFormat swapchain_depth_format_;
        VkExtent2D swapchain_extent_;

        std::vector<VkFramebuffer> swapchain_frame_buffers_;
        VkRenderPass render_pass_;

        std::vector<AllocatedImage> depth_images_;
        std::vector<VkImageView> depth_image_views_;
        std::vector<VkImage> swapchain_images_;
        std::vector<VkImageView> swapchain_image_views_;

        coral_device& device_;
        VkExtent2D window_extent_;
        VkSwapchainKHR swapchain_;
        std::shared_ptr<coral_swapchain> old_swapchain_;

        FrameData frames_[MAX_FRAMES_IN_FLIGHT];
        size_t current_frame_ = 0;
    };

}  // namespace lve