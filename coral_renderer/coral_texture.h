#pragma once
#include <memory>

#include "vk_types.h"
#include "coral_device.h"

namespace coral_3d
{
	
	class coral_texture final
	{
	public:
		coral_texture(coral_device& device, AllocatedImage image, uint32_t mip_levels, VkFormat format);
		~coral_texture();

		static std::unique_ptr<coral_texture> create_texture_from_file(coral_device& device, const std::string& file_path, VkFormat format);

		AllocatedImage image() const { return image_; }
		VkImageView image_view() const { return image_view_; }
		VkSampler sampler() const { return sampler_; }

		VkDescriptorImageInfo get_descriptor_info() const
		{
			VkDescriptorImageInfo descriptor_info{};

			descriptor_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptor_info.imageView = image_view_;
			descriptor_info.sampler = sampler_;

			return descriptor_info;
		}

	private:
		struct Builder
		{
			AllocatedImage image;
			uint32_t mip_levels;
			bool load_image_from_file(coral_device& device, const std::string& file_name, VkFormat format);
			void generate_mipmaps(coral_device& device, uint32_t width, uint32_t height);
		};

		void create_image_view(VkFormat format);
		void create_texture_sampler();
		
		coral_device& device_;
		AllocatedImage image_;
		VkImageView image_view_;
		VkSampler sampler_;

		// MIP MAPS
		uint32_t mip_levels_;
	};
}
