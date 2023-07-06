#include "coral_device.h"

// STD
#include <iostream>
#include <set>
#include <unordered_set>

#include <VkBootstrap.h>
#include "vk_initializers.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

using namespace coral_3d;

coral_device::coral_device(coral_window& window) : window_{ window }
{
    create_instance();
    create_command_pool();
    create_sync_structures();
    create_command_buffers();
}

coral_device::~coral_device()
{
    // Wait for the GPU to be done with the last frame
    vkDeviceWaitIdle(device_);

    deletion_queue_.flush();

    vkDestroyDevice(device_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkb::destroy_debug_utils_messenger(instance_, debug_messenger_);
    vkDestroyInstance(instance_, nullptr);
}

uint32_t coral_device::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat coral_device::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device_, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && 
            (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
        
    throw std::runtime_error("failed to find supported format!");
}

AllocatedBuffer coral_device::create_buffer(VkDeviceSize alloc_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
{
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext = nullptr;

    info.size = alloc_size;
    info.usage = usage;

    VmaAllocationCreateInfo vma_alloc_info{};
    vma_alloc_info.usage = memory_usage;

    AllocatedBuffer new_buffer;

    if(vmaCreateBuffer(allocator_, &info, &vma_alloc_info,
        &new_buffer.buffer,
        &new_buffer.allocation,
        nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error(
            "ERROR! coral_device::create_buffer() >> Failed to create buffer!");
    }

    return new_buffer;
}

AllocatedImage coral_device::create_image(const VkImageCreateInfo& image_info, VmaMemoryUsage memory_usage)
{
    AllocatedImage new_image;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = memory_usage;
    alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(allocator_, &image_info, &alloc_info, &new_image.image, &new_image.allocation, nullptr);

    return new_image;
}

void coral_device::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
    VkCommandBuffer cmd{ upload_context_.command_buffer };

    // Begin command buffer recording. We use it exact once before resetting
    VkCommandBufferBeginInfo cmd_begin_info{ vkinit::command_buffer_bi(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) };
    if (vkBeginCommandBuffer(cmd, &cmd_begin_info) != VK_SUCCESS)
        throw std::runtime_error(
            "ERROR! coral_device::immediate_submit() >> Failed to begin recording!");

    // Execute function
    function(cmd);

    if(vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error(
            "ERROR! coral_device::immediate_submit() >> Failed to end recording!");

    VkSubmitInfo submit{ vkinit::submit_info(&cmd) };

    // Submit command buffer to queue and execute it
    // uploadFence will now block until the graphic commands finish execution
    if(vkQueueSubmit(graphics_queue_, 1, &submit, upload_context_.upload_fence)
        != VK_SUCCESS)
        throw std::runtime_error(
            "ERROR! coral_device::immediate_submit() >> Failed to submit commands!");

    vkWaitForFences(device_, 1, &upload_context_.upload_fence, true, UINT64_MAX);
    vkResetFences(device_, 1, &upload_context_.upload_fence);

    // Reset command buffers inside the pool
    vkResetCommandPool(device_, upload_context_.command_pool, 0);
}

void coral_device::copy_buffer(AllocatedBuffer src_buffer, AllocatedBuffer dst_buffer, VkDeviceSize size)
{
    immediate_submit([&](VkCommandBuffer cmd)
        {
            VkBufferCopy copy_region{};
            copy_region.srcOffset = 0;  // Optional
            copy_region.dstOffset = 0;  // Optional
            copy_region.size = size;
            vkCmdCopyBuffer(cmd, src_buffer.buffer, dst_buffer.buffer, 1, &copy_region);
        });
}

void coral_device::copy_buffer_to_image(AllocatedBuffer buffer, AllocatedImage image, uint32_t width, uint32_t height, uint32_t layer_count)
{
    immediate_submit([&](VkCommandBuffer cmd)
        {
            VkBufferImageCopy copy_region{};
            copy_region.bufferOffset = 0;
            copy_region.bufferRowLength = 0;
            copy_region.bufferImageHeight = 0;

            copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_region.imageSubresource.mipLevel = 0;
            copy_region.imageSubresource.baseArrayLayer = 0;
            copy_region.imageSubresource.layerCount = layer_count;

            copy_region.imageExtent = { width, height, 1 };

            // Copy buffer to image
            vkCmdCopyBufferToImage(cmd, buffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        });
}

void coral_device::create_instance()
{
    std::cout << "Creating instance...\n";

    vkb::InstanceBuilder builder;
    auto instance_desc = builder.set_app_name("coral_renderer")
        .request_validation_layers(c_enable_validation_layers_)
        .require_api_version(1, 1, 0)
        .use_default_debug_messenger()
        .build();

    vkb::Instance vkb_instance{instance_desc.value()};
    instance_ = vkb_instance.instance;
    debug_messenger_ = vkb_instance.debug_messenger;

    // Check if this instance has all required extensions
    // Will throw if not found
    has_glfw_required_instance_extensions();

    create_surface();

    VkPhysicalDeviceFeatures feats{};
    feats.multiDrawIndirect = true;

    vkb::PhysicalDeviceSelector selector{vkb_instance};
    selector.set_required_features(feats);

    vkb::PhysicalDevice physical_device = selector
        .set_minimum_version(1, 1)
        .set_surface(surface_)
        .select()
        .value();

    vkb::DeviceBuilder device_builder{physical_device};

    VkPhysicalDeviceShaderDrawParameterFeatures shader_draw_params{};
    shader_draw_params.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES;
    shader_draw_params.pNext = nullptr;
    
    shader_draw_params.shaderDrawParameters = VK_TRUE;

    auto vkb_device = device_builder.add_pNext(&shader_draw_params).build().value();

    graphics_queue_ = vkb_device.get_queue(vkb::QueueType::graphics).value();
    present_queue_ = vkb_device.get_queue(vkb::QueueType::present).value();

    device_ = vkb_device.device;
    physical_device_ = physical_device.physical_device;

    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.physicalDevice = physical_device_;
    allocator_info.device = device_;
    allocator_info.instance = instance_;
    vmaCreateAllocator(&allocator_info, &allocator_);

    deletion_queue_.deletors.emplace_back([=]() {
        vmaDestroyAllocator(allocator_);
        });

    vkGetPhysicalDeviceProperties(physical_device_, &properties);
    std::cout << "physical device: " << properties.deviceName << std::endl;
}

void coral_device::create_surface() 
{ 
    window_.create_window_surface(instance_, &surface_);
}

void coral_device::create_command_pool()
{
    auto indices { find_physical_queue_families() };

    VkCommandPoolCreateInfo graphics_pool_info{
        vkinit::command_pool_ci(indices.graphics_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) };

    // Create general pool
    if (vkCreateCommandPool(device_, &graphics_pool_info, nullptr, &command_pool_) != VK_SUCCESS)
        throw std::runtime_error("ERROR! coral_device::create_command_pool() >> Failed to create command pool!");

    deletion_queue_.deletors.emplace_front([=]() {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
        });

    // Create pool for upload context, used for immediate commands
    if (vkCreateCommandPool(device_, &graphics_pool_info, nullptr, &upload_context_.command_pool)
        != VK_SUCCESS)
    {
        throw std::runtime_error(
            "ERROR! coral_device::create_command_pool() >> Failed to create upload command pool!");
    }

    deletion_queue_.deletors.emplace_back([=]() {
        vkDestroyCommandPool(device_, upload_context_.command_pool, nullptr);
        });
}

void coral_device::create_sync_structures()
{
    // Upload fence
    VkFenceCreateInfo upload_fence_info{ vkinit::fence_ci() };

    if (vkCreateFence(device_, &upload_fence_info, nullptr, &upload_context_.upload_fence) != VK_SUCCESS)
        throw std::runtime_error("ERROR! coral_device::create_sync_structures() >> Failed to create upload fence!");

    deletion_queue_.deletors.emplace_back([=]() {
        vkDestroyFence(device_, upload_context_.upload_fence, nullptr);
        });
}

void coral_device::create_command_buffers()
{
    // Allocate default command buffer that we will use for immediate commands
    VkCommandBufferAllocateInfo alloc_info {
        vkinit::command_buffer_ai(upload_context_.command_pool, 1) };

    if (vkAllocateCommandBuffers(device_, &alloc_info, &upload_context_.command_buffer)
        != VK_SUCCESS)
    {
        throw std::runtime_error(
            "ERROR! coral_device::create_command_buffers() >> Failed to create upload command buffer!");
    }
}

std::vector<const char*> coral_device::get_required_extensions()
{
    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (c_enable_validation_layers_)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool coral_device::check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : c_validation_layers)
    {
        bool layer_found = false;

        for (const auto& layer_properties : available_layers)
        {
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) return false;
    }

    return true;
}

QueueFamilyIndices coral_device::find_queue_families(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueCount > 0 && 
            queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
            indices.graphics_family_has_value = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);

        if (queue_family.queueCount > 0 &&
            presentSupport)
        {
            indices.present_family = i;
            indices.present_family_has_value = true;
        }

        if (indices.is_complete()) break;

        i++;
    }

    return indices;
}

void coral_device::has_glfw_required_instance_extensions()
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    std::cout << "available extensions:" << std::endl;
    std::unordered_set<std::string> available;

    for (const auto& extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << std::endl;
        available.insert(extension.extensionName);
    }

    std::cout << "required extensions:" << std::endl;
    auto required_extensions = get_required_extensions();

    for (const auto& required : required_extensions)
    {
        std::cout << "\t" << required << std::endl;
        if (available.find(required) == available.end())
        {
            throw std::runtime_error(
                "ERROR! coral_device::has_glfw_required_instance_extensions() >> Missing required glfw extension.");
        }
    }
}

bool coral_device::check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extension_count,
        available_extensions.data());

    std::set<std::string> required_extensions(c_device_extensions.begin(), c_device_extensions.end());

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

SwapchainSupportDetails coral_device::query_swapchain_support(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            &present_mode_count,
            details.present_modes.data());
    }

    return details;
}