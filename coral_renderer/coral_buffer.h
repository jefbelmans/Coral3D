#pragma once

#include "coral_device.h"

namespace coral_3d
{
	class coral_buffer final
	{
	public:
		coral_buffer(
			coral_device& device,
			VkDeviceSize instance_size,
			uint32_t instance_count,
			VkBufferUsageFlags usage_flags,
			VmaMemoryUsage usage,
			VmaAllocationCreateFlags flags = 0,
			VkDeviceSize min_offset_alignment = 1
		);

		~coral_buffer();

		coral_buffer(const coral_buffer&) = delete;
		coral_buffer& operator=(const coral_buffer&) = delete;

		VkResult map();
		void unmap();

		void write_to_buffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void write_to_index(void* data, uint32_t index);
		VkResult flush_index(uint32_t index);
		VkDescriptorBufferInfo descriptor_info_index(uint32_t index) const;
		VkResult invalidate_index(uint32_t index);

		AllocatedBuffer get_buffer() const { return buffer_; }
		void* get_mapped_data() const { return mapped_data_; }
		uint32_t get_instance_count() const { return instance_count_; }
		VkDeviceSize get_instance_size() const { return instance_size_; }
		VkDeviceSize get_alignment_size() const { return alignment_size_; }
		VmaMemoryUsage get_usage() const { return usage_; }
		VkDeviceSize get_buffer_size() const { return buffer_size_; }

	private:
		static VkDeviceSize get_allignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

		coral_device& device_;
		void* mapped_data_{nullptr};
		AllocatedBuffer buffer_{};

		VkDeviceSize buffer_size_{0};
        VkDeviceSize instance_size_{0};
		uint32_t instance_count_{ 0 };
		VkDeviceSize alignment_size_{ 0 };
		VmaMemoryUsage usage_{VMA_MEMORY_USAGE_AUTO};
		VkBufferUsageFlags usage_flags_{0};

	};
}

