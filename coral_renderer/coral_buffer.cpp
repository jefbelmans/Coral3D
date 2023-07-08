#include "coral_buffer.h"

#include <cassert>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

using namespace coral_3d;

coral_buffer::coral_buffer(
	coral_device& device,
	VkDeviceSize instance_size,
	uint32_t instance_count,
	VkBufferUsageFlags usage_flags,
	VmaMemoryUsage usage,
	VmaAllocationCreateFlags flags,
	VkDeviceSize min_offset_alignment)
	: device_{device}
	, instance_size_{instance_size}
	, instance_count_{instance_count}
	, usage_{usage}
	, usage_flags_{usage_flags}
{
	alignment_size_ = get_allignment(instance_size_, min_offset_alignment);
	buffer_size_ = alignment_size_ * instance_count_;
	buffer_ = device_.create_buffer(buffer_size_, usage_flags_, usage_, flags);
}

coral_buffer::~coral_buffer()
{
	unmap();
	vmaDestroyBuffer(device_.allocator(), buffer_.buffer, buffer_.allocation);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult coral_buffer::map()
{
	assert(buffer_.buffer && buffer_.allocation && "ERROR! coral_buffer::map() >> Called map on an unitialized buffer");
	return vmaMapMemory(device_.allocator(), buffer_.allocation, &mapped_data_);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void coral_buffer::unmap()
{
	if (mapped_data_)
	{
		vmaUnmapMemory(device_.allocator(), buffer_.allocation);
		mapped_data_ = nullptr;
	}
}

/**
 * Copies the specified data to the mapped buffer. Default value writes_ whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void coral_buffer::write_to_buffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
	assert(mapped_data_ && "ERROR! coral_buffer::write_to_buffer() >> Called write_to_buffer on an unmapped buffer");


	if(size == VK_WHOLE_SIZE)
		memcpy(mapped_data_, data, buffer_size_);
	else
	{
		char* mem_offset{ reinterpret_cast<char*>(mapped_data_) };
		mem_offset += offset;
		memcpy(mem_offset, data, size);
	}
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult coral_buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
	return vmaFlushAllocation(device_.allocator(), buffer_.allocation, offset, size);
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo coral_buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) const
{
	return VkDescriptorBufferInfo
	{
		buffer_.buffer,
		offset,
		size
	};
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult coral_buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	return vmaInvalidateAllocation(device_.allocator(), buffer_.allocation, offset, size);
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignment_size_
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void coral_buffer::write_to_index(void* data, uint32_t index)
{
	write_to_buffer(data, instance_size_, index * alignment_size_);
}

/**
 *  Flush the memory range at index * alignment_size_ of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
VkResult coral_buffer::flush_index(uint32_t index)
{
	return flush(alignment_size_, index * alignment_size_);
}

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignment_size_
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
VkDescriptorBufferInfo coral_buffer::descriptor_info_index(uint32_t index) const
{
	return descriptor_info(alignment_size_, index * alignment_size_);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignment_size_
 *
 * @return VkResult of the invalidate call
 */
VkResult coral_buffer::invalidate_index(uint32_t index)
{
	return invalidate(alignment_size_, index * alignment_size_);
}


VkDeviceSize coral_buffer::get_allignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment)
{
	return min_offset_alignment > 0 ? (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1) : instance_size;
}