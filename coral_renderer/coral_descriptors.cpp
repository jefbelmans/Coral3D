#include "coral_descriptors.h"

// STD
#include <cassert>
#include <stdexcept>

using namespace coral_3d;

coral_descriptor_set_layout::Builder& coral_descriptor_set_layout::Builder::add_binding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
    assert(bindings_.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorType = descriptorType;
    layout_binding.descriptorCount = count;
    layout_binding.stageFlags = stageFlags;
    bindings_[binding] = layout_binding;
    return *this;
}

std::unique_ptr<coral_descriptor_set_layout> coral_descriptor_set_layout::Builder::build() const
{
    return std::make_unique<coral_descriptor_set_layout>(device_, bindings_);
}

// *************** Descriptor Set Layout *********************

coral_descriptor_set_layout::coral_descriptor_set_layout(coral_device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) 
    : device_{ device }
    , bindings_{ bindings }
    , descriptor_set_layout_{ VK_NULL_HANDLE }
{
    std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
    for (auto& kv : bindings)
    {
        set_layout_bindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
    descriptor_set_layout_info.pBindings = set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(
        device_.device(),
        &descriptor_set_layout_info,
        nullptr,
        &descriptor_set_layout_) != VK_SUCCESS)
    {
        throw std::runtime_error("ERROR! coral_descriptor_set_layout::coral_descriptor_set_layout() >> Failed to create descriptor set layout!");
    }
}

coral_descriptor_set_layout::~coral_descriptor_set_layout()
{
    vkDestroyDescriptorSetLayout(device_.device(), descriptor_set_layout_, nullptr);
}

// *************** Descriptor Pool Builder *********************

coral_descriptor_pool::Builder& coral_descriptor_pool::Builder::add_pool_size(VkDescriptorType descriptor_type, uint32_t count)
{
    pool_sizes_.push_back({ descriptor_type, count });
    return *this;
}

coral_descriptor_pool::Builder& coral_descriptor_pool::Builder::set_pool_flags(VkDescriptorPoolCreateFlags flags)
{
    pool_flags_ = flags;
    return *this;
}

coral_descriptor_pool::Builder& coral_descriptor_pool::Builder::set_max_sets(uint32_t count)
{
    max_sets_ = count;
    return *this;
}

std::unique_ptr<coral_descriptor_pool> coral_descriptor_pool::Builder::build() const
{
    return std::make_unique<coral_descriptor_pool>(device_, max_sets_, pool_flags_, pool_sizes_);
}

// *************** Descriptor Pool *********************

coral_descriptor_pool::coral_descriptor_pool(
    coral_device& device,
    uint32_t max_sets,
    VkDescriptorPoolCreateFlags pool_flags,
    const std::vector<VkDescriptorPoolSize>& pool_sizes) : device_{ device }
{
    VkDescriptorPoolCreateInfo descriptor_pool_info{};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_info.pPoolSizes = pool_sizes.data();
    descriptor_pool_info.maxSets = max_sets;
    descriptor_pool_info.flags = pool_flags;

    if (vkCreateDescriptorPool(device_.device(), &descriptor_pool_info, nullptr, &descriptor_pool_) != VK_SUCCESS)
        throw std::runtime_error("ERROR! coral_descriptor_pool::coral_descriptor_pool() >> Failed to create descriptor pool!");
}

coral_descriptor_pool::~coral_descriptor_pool()
{
    vkDestroyDescriptorPool(device_.device(), descriptor_pool_, nullptr);
}

bool coral_descriptor_pool::allocate_descriptor_set(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool_;
    allocInfo.pSetLayouts = &descriptor_set_layout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(device_.device(), &allocInfo, &descriptor) != VK_SUCCESS)
        return false;

    return true;
}

void coral_descriptor_pool::free_descriptors(std::vector<VkDescriptorSet>& descriptors) const
{
    vkFreeDescriptorSets(
        device_.device(),
        descriptor_pool_,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void coral_descriptor_pool::reset_pool()
{
    vkResetDescriptorPool(device_.device(), descriptor_pool_, 0);
}

// *************** Descriptor Writer *********************

coral_descriptor_writer::coral_descriptor_writer(coral_descriptor_set_layout& set_layout, coral_descriptor_pool& pool)
    : set_layout_{ set_layout }, pool_{ pool } {}

coral_descriptor_writer& coral_descriptor_writer::write_buffer(uint32_t binding, VkDescriptorBufferInfo* buffer_info)
{
    assert(set_layout_.bindings_.count(binding) == 1 && "ERROR! coral_descriptor_writer::write_buffer() >> Layout does not contain specified binding.");

    auto& binding_description = set_layout_.bindings_[binding];

    assert(
        binding_description.descriptorCount == 1 &&
        "ERROR! coral_descriptor_writer::write_buffer() >> Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = binding_description.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = buffer_info;
    write.descriptorCount = 1;

    writes_.push_back(write);
    return *this;
}

coral_descriptor_writer& coral_descriptor_writer::write_image(uint32_t binding, VkDescriptorImageInfo* image_info)
{
    assert(set_layout_.bindings_.count(binding) == 1 && "ERROR! coral_descriptor_writer::write_image() >> Layout does not contain specified binding");

    auto& binding_description = set_layout_.bindings_[binding];

    assert(
        binding_description.descriptorCount == 1 &&
        "ERROR! coral_descriptor_writer::write_image() >> Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = binding_description.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = image_info;
    write.descriptorCount = 1;

    writes_.push_back(write);
    return *this;
}

bool coral_descriptor_writer::build(VkDescriptorSet& set)
{
    if (!pool_.allocate_descriptor_set(set_layout_.get_descriptor_set_layout(), set))
        return false;

    overwrite(set);
    return true;
}

void coral_descriptor_writer::overwrite(VkDescriptorSet& set)
{
    for (auto& write : writes_)
        write.dstSet = set;

    vkUpdateDescriptorSets(pool_.device_.device(), writes_.size(), writes_.data(), 0, nullptr);
}