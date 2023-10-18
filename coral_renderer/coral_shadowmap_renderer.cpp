#include "coral_shadowmap_renderer.h"

#define DEPTH_FORMAT VK_FORMAT_D16_UNORM
#define SHADOWMAP_DIM 2048
#define DEFAULT_SHADOWMAP_FILTER VK_FILTER_LINEAR

#include "vk_initializers.h"

namespace coral_3d
{
    coral_shadowmap_renderer::coral_shadowmap_renderer(coral_device& device)
    : device_{device}
    {
        create_framebuffer();
    }

    coral_shadowmap_renderer::~coral_shadowmap_renderer()
    {
        vkDestroyRenderPass(device_.device(), render_pass_, nullptr);
        vmaDestroyImage(device_.allocator(), frame_buffer_attachment_.image.image, frame_buffer_attachment_.image
        .allocation);

        vkDestroyFramebuffer(device_.device(), frame_buffer_, nullptr);
    }

    void coral_shadowmap_renderer::create_render_pass()
    {
        // ATTACHMENT DESCRIPTION
        VkAttachmentDescription attachment_description{};
        attachment_description.format = DEPTH_FORMAT;
        attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // We will read from the depth, so it's important to store it
        attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // Attachment will be transitioned to shader read at render pass end
        attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        // DEPTH REFERENCE
        VkAttachmentReference depth_ref{};
        depth_ref.attachment = 0;
        depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // SUB PASSES
        VkSubpassDescription sub_pass{};
        sub_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub_pass.colorAttachmentCount = 0;
        sub_pass.pDepthStencilAttachment = &depth_ref;

        // Dependencies
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.pNext = nullptr;

        create_info.attachmentCount = 1;
        create_info.pAttachments = &attachment_description;
        create_info.subpassCount = 1;
        create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
        create_info.pDependencies = dependencies.data();

        vkCreateRenderPass(device_.device(), &create_info, nullptr, &render_pass_);
    }

    void coral_shadowmap_renderer::create_framebuffer()
    {
        width_ = SHADOWMAP_DIM;
        height_ = SHADOWMAP_DIM;

        // For shadow mapping we only need a depth attachment
        VkImageCreateInfo image_ci = vkinit::image_ci(DEPTH_FORMAT,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT, {width_, height_, 1});

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(device_.allocator(), &image_ci, &alloc_info, &frame_buffer_attachment_.image.image,
                       &frame_buffer_attachment_.image.allocation, nullptr);

        VkImageViewCreateInfo depth_stencil_view = vkinit::image_view_ci(DEPTH_FORMAT, frame_buffer_attachment_.image
        .image, VK_IMAGE_ASPECT_DEPTH_BIT);
        vkCreateImageView(device_.device(), &depth_stencil_view, nullptr, &frame_buffer_attachment_
        .image_view);

        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        VkFilter shadowmap_filter = DEFAULT_SHADOWMAP_FILTER;
        VkSamplerCreateInfo sampler = vkinit::sampler_ci(shadowmap_filter, 1.0f, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        vkCreateSampler(device_.device(), &sampler, nullptr, &depth_sampler_);

        create_render_pass();

        // Create frame buffer
        VkFramebufferCreateInfo frame_buffer_ci{};
        frame_buffer_ci.renderPass = render_pass_;
        frame_buffer_ci.attachmentCount = 1;
        frame_buffer_ci.pAttachments = &frame_buffer_attachment_.image_view;
        frame_buffer_ci.width = width_;
        frame_buffer_ci.height = height_;
        frame_buffer_ci.layers = 1;

        vkCreateFramebuffer(device_.device(), &frame_buffer_ci, nullptr, &frame_buffer_);

    }
} // coral_3d