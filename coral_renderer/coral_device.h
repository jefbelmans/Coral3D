#pragma once

#include "coral_window.h"

#include <vk_mem_alloc.h>

#include <string>
#include <vector>
#include <deque>
#include <functional>

#include "vk_types.h"

namespace coral_3d
{
	struct DeletionQueue
	{
		std::deque <std::function<void()>> deletors;

		void flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
				(*it)();

			deletors.clear();
		}
	};

	struct UploadContext
	{
		VkFence upload_fence;
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics_family;
		uint32_t present_family;
		bool graphics_family_has_value { false };
		bool present_family_has_value { false };
		bool is_complete() { return graphics_family_has_value && present_family_has_value; }
	};

	class coral_device final
	{
	public:

#ifdef NDEBUG
		const bool c_enable_validation_layers_{false};
#else
		const bool c_enable_validation_layers_{ true };
#endif

		coral_device(coral_window& window);
		~coral_device();

		// Non-copyable & non-movable
		coral_device(const coral_device&) = delete;
		void operator=(const coral_device&) = delete;
		coral_device(coral_device&&) = delete;
		coral_device& operator=(coral_device&&) = delete;

		VkCommandPool get_command_pool() { return command_pool_; }
		VkDevice device() { return device_; }
		VkSurfaceKHR surface() { return surface_; }
		VkQueue graphics_queue() { return graphics_queue_; }
		VkQueue present_queue() { return present_queue_; }

		SwapchainSupportDetails get_swapchain_support() {};
		uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
		QueueFamilyIndices find_physical_queue_families() { return find_queue_families(physical_device_); }
		VkFormat find_supported_format(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer helpers
		AllocatedBuffer create_buffer(size_t alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
		void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
		void copy_buffer(AllocatedBuffer src_buffer, AllocatedBuffer dst_buffer, VkDeviceSize size);
		void copy_buffer_to_image(AllocatedBuffer buffer, AllocatedImage image, uint32_t width, uint32_t height, uint32_t layer_count);

		VkPhysicalDeviceProperties properties;

	private:
		void create_instance();
		void create_surface();
		void create_command_pool();
		void create_command_buffers();

		// Helper methods
		std::vector<const char*> get_required_extensions();
		bool check_validation_layer_support();
		QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
		void has_glfw_required_instance_extensions();
		bool check_device_extension_support(VkPhysicalDevice device);
		SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device);

		DeletionQueue deletion_queue_;

		// For immediate commands
		UploadContext upload_context_;

		VkInstance instance_;
		VkDebugUtilsMessengerEXT debug_messenger_;
		VkPhysicalDevice physical_device_ { VK_NULL_HANDLE };
		coral_window& window_;
		VkCommandPool command_pool_;

		VmaAllocator allocator_;
		VkDevice device_;
		VkSurfaceKHR surface_;
		VkQueue graphics_queue_;
		VkQueue present_queue_;

		const std::vector<const char*> c_validation_layers { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> c_device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}