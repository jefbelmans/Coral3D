#pragma once

#include "coral_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace coral_3d
{
    class coral_descriptor_set_layout
    {
    public:

        class Builder
        {
        public:
            Builder(coral_device& device) : device_{ device } {}

            Builder& add_binding(
                uint32_t binding,
                VkDescriptorType descriptor_type,
                VkShaderStageFlags stage_flags,
                uint32_t count = 1);
            std::unique_ptr<coral_descriptor_set_layout> build() const;

        private:
            coral_device& device_;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_{};
        };

        coral_descriptor_set_layout(coral_device& device_, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_);
        ~coral_descriptor_set_layout();

        coral_descriptor_set_layout(const coral_descriptor_set_layout&) = delete;
        coral_descriptor_set_layout& operator=(const coral_descriptor_set_layout&) = delete;

        VkDescriptorSetLayout get_descriptor_set_layout() const { return descriptor_set_layout_; }

    private:
        coral_device& device_;
        VkDescriptorSetLayout descriptor_set_layout_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

        friend class coral_descriptor_writer;
    };

    class coral_descriptor_pool
    {
    public:
        class Builder
        {
        public:
            Builder(coral_device& device_) : device_{ device_ } {}

            Builder& add_pool_size(VkDescriptorType descripto_type, uint32_t count);
            Builder& set_pool_flags(VkDescriptorPoolCreateFlags flags);
            Builder& set_max_sets(uint32_t count);
            std::unique_ptr<coral_descriptor_pool> build() const;

        private:
            coral_device& device_;
            std::vector<VkDescriptorPoolSize> pool_sizes_{};
            uint32_t max_sets_ { 1000 };
            VkDescriptorPoolCreateFlags pool_flags_{ 0 };
        };

        coral_descriptor_pool(
            coral_device& device,
            uint32_t max_sets,
            VkDescriptorPoolCreateFlags pool_flags,
            const std::vector<VkDescriptorPoolSize>& pool_sizes);
        ~coral_descriptor_pool();

        coral_descriptor_pool(const coral_descriptor_pool&) = delete;
        coral_descriptor_pool& operator=(const coral_descriptor_pool&) = delete;

        bool allocate_descriptor_set(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const;
        void free_descriptors(std::vector<VkDescriptorSet>& descriptors) const;
        void reset_pool();

    private:
        coral_device& device_;
        VkDescriptorPool descriptor_pool_;

        friend class coral_descriptor_writer;
    };

    class coral_descriptor_writer {
    public:
        coral_descriptor_writer(coral_descriptor_set_layout& set_layout, coral_descriptor_pool& pool);

        coral_descriptor_writer& write_buffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info);
        coral_descriptor_writer& write_image(uint32_t binding, VkDescriptorImageInfo* image_info);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        coral_descriptor_set_layout& set_layout_;
        coral_descriptor_pool& pool_;
        std::vector<VkWriteDescriptorSet> writes_;
    };

}  // namespace lve