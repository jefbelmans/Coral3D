#include "vk_engine.h"

// VMA
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

// GLFW Window
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM
#include <gtx/transform.hpp>

// Bootstrap library
#include <VkBootstrap.h>

// STL includes
#include <iostream>
#include <iomanip>
#include <fstream>

#pragma region macros
// Vulkan error macro
using namespace std;
#define VK_CHECK(x)													\
	do																\
	{																\
		VkResult err = x;											\
		if (err)													\
		{															\
			std::cout << "Detected Vulkan error: " << err << "\n";	\
			abort();												\
		}															\
	} while (0)
#pragma endregion

// Vulkan includes
#include "vk_initializers.h"
#include "vk_pipeline.h"
#include "vk_texture.h"

using namespace Coral3D;

void VulkanEngine::init()
{
	// WINDOW
	init_window();

	// VULKAN
	init_vulkan();
	init_swapchain();
	init_commands();
	init_def_renderpass();
	init_framebuffers();
	init_sync_structures();
	init_descriptors();
	init_pipelines();

	// SCENES
	load_images();
	load_meshes();
	init_scene();

	m_IsInitialized = true;
	std::cout << "VulkanEngine initialized\n";
}

void VulkanEngine::run()
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
		draw();
	}

	glfwTerminate();
}

void VulkanEngine::init_window()
{
	std::cout << "VulkanEngine created\n";

	// Init glfw library
	if (!glfwInit())
		return;

	// Create window without OpenGL context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_pWindow = glfwCreateWindow(1280, 720, "Coral3D", nullptr, nullptr);
	if (!m_pWindow)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}

	if (!glfwInit())
		throw std::runtime_error("Failed to initialize glfw");
}

void VulkanEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
	VkCommandBuffer cmd{ m_UploadContext.command_buffer };
	
	// Begin command buffer recording. We use it exact once before resetting
	VkCommandBufferBeginInfo cmdBeginInfo{ vkinit::command_buffer_bi(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) };
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
	
	// Execute function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit{ vkinit::submit_info(&cmd) };

	// Submit command buffer to queue and execute it
	// uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submit, m_UploadContext.upload_fence));

	vkWaitForFences(m_Device, 1, &m_UploadContext.upload_fence, true, UINT64_MAX);
	vkResetFences(m_Device, 1, &m_UploadContext.upload_fence);

	// Reset command buffers inside the pool
	vkResetCommandPool(m_Device, m_UploadContext.command_pool, 0);
}

void VulkanEngine::draw()
{
	// Wait for the GPU to be done with the last frame
	VK_CHECK(vkWaitForFences(m_Device, 1, &get_current_frame().render_fence, true, UINT64_MAX));
	VK_CHECK(vkResetFences(m_Device, 1, &get_current_frame().render_fence));

	// Request image from the swap chain
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, get_current_frame().present_semaphore, nullptr, &swapchainImageIndex));

	// Commands have finished executing, so reset the buffer
	VK_CHECK(vkResetCommandBuffer(get_current_frame().main_command_buffer, 0));

	// Record commands
	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	// We record each frame to the command buffer, so we need to let Vulkan know that we're only executing it once
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Begin recording
	VK_CHECK(vkBeginCommandBuffer(get_current_frame().main_command_buffer, &cmdBeginInfo));

	// Clear color
	VkClearValue clearValue;
	clearValue.color = { {0.1f, 0.1f, 0.85f, 1.0f} };

	// Depth clear value
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkClearValue clearValues[] = { clearValue, depthClear };

	// Begin the render pass
	VkRenderPassBeginInfo rpInfo{};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	// Use the default render pass on the image with the index that we acquired
	rpInfo.renderPass = m_RenderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = m_WindowExtent;
	rpInfo.framebuffer = m_Framebuffers[swapchainImageIndex];

	// Connect clear values
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = &clearValues[0];

	// Send the begin command to Vulkan
	vkCmdBeginRenderPass(get_current_frame().main_command_buffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	// RENDER STUFF HERE
	// ...

	draw_objects(get_current_frame().main_command_buffer, m_Renderables.data(), m_Renderables.size());

	// End the render pass
	vkCmdEndRenderPass(get_current_frame().main_command_buffer);

	// End recording
	VK_CHECK(vkEndCommandBuffer(get_current_frame().main_command_buffer));

	// Prepare to submit to the queue
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	submit.pWaitDstStageMask = &waitStage;

	// Wait for image to be ready for rendering before rendering to it
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame().present_semaphore;

	// Signal ready when finished rendering
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame().render_semaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &get_current_frame().main_command_buffer;

	// Submit to the graphics queue
	VK_CHECK(vkQueueSubmit(m_GraphicsQueue, 1, &submit, get_current_frame().render_fence));

	// Present the image to the window when the rendering semaphore has signaled that rendering is done
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.swapchainCount = 1;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &get_current_frame().render_semaphore;

	// Which image to present
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(m_GraphicsQueue, &presentInfo));

	m_FrameNumber++;
}

std::vector<IndirectBatch> Coral3D::VulkanEngine::batch_draws(const std::vector<RenderObject>& renderObjects)
{
	std::vector<IndirectBatch> draws;

	int id{};
	std::for_each(begin(renderObjects), end(renderObjects), [&](const RenderObject& ro) {
		bool isMeshBatched { !draws.empty() && ro.mesh == draws.back().mesh };
		bool isMaterialBatched { !draws.empty() && ro.material == draws.back().material };

		// Mesh and material combo is already batched, just increase count
		if (isMeshBatched && isMaterialBatched)
			draws.back().count++;

		// Mesh and material combo is not already batched, add new draw
		else
		{
			IndirectBatch newDraw;
			newDraw.mesh = ro.mesh;
			newDraw.material = ro.material;
			newDraw.first = id;
			newDraw.count = 1;

			draws.emplace_back(newDraw);
		}

		id++;
		});

	return draws;
}

void VulkanEngine::cleanup()
{
	if (m_IsInitialized)
	{
		// Wait for the GPU to be done with the last frame
		vkDeviceWaitIdle(m_Device);

		m_MainDeletionQueue.Flush();

		vkDestroyDevice(m_Device, nullptr);
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
		vkDestroyInstance(m_Instance, nullptr);
	}
}

void VulkanEngine::init_vulkan()
{
	std::cout << "Initializing Vulkan\n";
	vkb::InstanceBuilder builder;

	// Make the Vulkan instance, with basic debug features
	auto instanceDesc = builder.set_app_name("Coral3D")
#ifdef NDEBUG
		.request_validation_layers(false)
#else
		.request_validation_layers(true)
#endif
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkbInstance = instanceDesc.value();

	// Store the instance
	m_Instance = vkbInstance.instance;

	// Store the debug messenger
	m_DebugMessenger = vkbInstance.debug_messenger;

	if (glfwCreateWindowSurface(m_Instance, m_pWindow, nullptr, &m_Surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

	// Get the window size to use for the swap chain
	int width, height;
	glfwGetWindowSize(m_pWindow, &width, &height);
	m_WindowExtent.width = static_cast<uint32_t>(width);
	m_WindowExtent.height = static_cast<uint32_t>(height);

	// use vkbootstrap to select a GPU.
	// We want a GPU that can write to the GLFW surface and supports Vulkan 1.2
	vkb::PhysicalDeviceSelector selector{ vkbInstance };

	VkPhysicalDeviceFeatures features{};
	features.multiDrawIndirect = true;
	features.drawIndirectFirstInstance = true;
	selector.set_required_features(features);

	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(m_Surface)
		.select()
		.value();

	// create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	VkPhysicalDeviceShaderDrawParameterFeatures shaderDrawParams{};
	shaderDrawParams.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES;
	shaderDrawParams.pNext = nullptr;
	
	shaderDrawParams.shaderDrawParameters = VK_TRUE;
	vkb::Device vkbDevice = deviceBuilder.add_pNext(&shaderDrawParams).build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	m_Device = vkbDevice.device;
	m_PhysicalDevice = physicalDevice.physical_device;

	// Get the graphics queue for the rest of the Vulkan application
	m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = m_PhysicalDevice;
	allocatorInfo.device = m_Device;
	allocatorInfo.instance = m_Instance;
	vmaCreateAllocator(&allocatorInfo, &m_Allocator);

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vmaDestroyAllocator(m_Allocator);
		});

	m_DeviceProperties = vkbDevice.physical_device.properties;
	std::cout << vkbDevice.physical_device.features.multiDrawIndirect << std::endl;
	std::cout << "The GPU has a minimum buffer alignment of " << m_DeviceProperties.limits.minUniformBufferOffsetAlignment << "\n";
}

void VulkanEngine::init_swapchain()
{
	std::cout << "Initializing Swap Chain\n";

	vkb::SwapchainBuilder swapChainBuilder{ m_PhysicalDevice, m_Device, m_Surface };

	vkb::Swapchain vkbSwapChain = swapChainBuilder
		.use_default_format_selection()
		// use VSync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
		.set_desired_extent(m_WindowExtent.width, m_WindowExtent.height)
		.build()
		.value();

	// store swap chain and its related images
	m_SwapChain = vkbSwapChain.swapchain;
	m_SwapChainImages = vkbSwapChain.get_images().value();
	m_SwapChainImageViews = vkbSwapChain.get_image_views().value();

	m_SwapChainImageFormat = vkbSwapChain.image_format;

	m_MainDeletionQueue.deletors.emplace_back([=]() {
			vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr); 
		});

	// Depth buffer image matches window extents
	VkExtent3D depthImageExtent 
	{
		m_WindowExtent.width,
		m_WindowExtent.height,
		1
	};

	// Depth format (must be changed to support stencil)
	m_DepthFormat = VK_FORMAT_D32_SFLOAT;

	VkImageCreateInfo depthImageCreateInfo = vkinit::image_ci(m_DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);
	
	// Allocate image on GPU
	VmaAllocationCreateInfo depthImageAllocInfo{};
	depthImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Allocate mem and create image
	vmaCreateImage(m_Allocator, &depthImageCreateInfo, &depthImageAllocInfo, &m_DepthImage.image, &m_DepthImage.allocation, nullptr);

	// Build depth buffer image view
	VkImageViewCreateInfo depthImageViewCreateInfo = vkinit::image_view_ci(m_DepthFormat, m_DepthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(m_Device, &depthImageViewCreateInfo, nullptr, &m_DepthImageView));

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
		vmaDestroyImage(m_Allocator, m_DepthImage.image, m_DepthImage.allocation);
		});
}

void VulkanEngine::init_commands()
{
	std::cout << "Initializing Commands\n";

	// Create pool for upload context
	VkCommandPoolCreateInfo uploadCommandPoolInfo{ vkinit::command_pool_ci(m_GraphicsQueueFamily) };
	VK_CHECK(vkCreateCommandPool(m_Device, &uploadCommandPoolInfo, nullptr, &m_UploadContext.command_pool));

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vkDestroyCommandPool(m_Device, m_UploadContext.command_pool, nullptr);
		});

	// Allocate default command buffer that we will use for immediate commands
	VkCommandBufferAllocateInfo cmdAllocInfo{ vkinit::command_buffer_ai(m_UploadContext.command_pool, 1) };
	
	VkCommandBuffer cmd;
	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &m_UploadContext.command_buffer));

	// Create command pool for commands submitted to the graphics queue
	// We also want the pool to allow for resetting of individual command buffers
	auto commandPoolInfo = vkinit::command_pool_ci(m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frames[i].command_pool));

		// Allocate the default command buffer that we will use for rendering
		auto commandBufferInfo = vkinit::command_buffer_ai(m_Frames[i].command_pool, 1);

		VK_CHECK(vkAllocateCommandBuffers(m_Device, &commandBufferInfo, &m_Frames[i].main_command_buffer));

		m_MainDeletionQueue.deletors.emplace_back([=]() {
			vkDestroyCommandPool(m_Device, m_Frames[i].command_pool, nullptr);
			});
	}
}

AllocatedBuffer VulkanEngine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = nullptr;
	
	info.size = allocSize;
	info.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

	AllocatedBuffer newBuffer{};

	VK_CHECK(vmaCreateBuffer(m_Allocator, &info, &vmaallocInfo,
		&newBuffer.buffer,
		&newBuffer.allocation,
		nullptr));

	return newBuffer;
}

void VulkanEngine::init_descriptors()
{
	// Create descriptor pool, descriptor sets will be allocated from this pool
	std::vector<VkDescriptorPoolSize> sizes
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;

	poolInfo.flags = 0;
	poolInfo.maxSets = 10;
	poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	poolInfo.pPoolSizes = sizes.data();

	vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool);

#pragma region DescriptorSet 1 (Global)
	// DescriptorSetLayout defines how the descriptor data is layed out in the descriptor set

	// Bindig 0: Uniform buffer (Camera data)
	VkDescriptorSetLayoutBinding camBufferBinding{ vkinit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT, 0) };

	// Binding 1: Uniform buffer (Scene data)
	VkDescriptorSetLayoutBinding sceneBufferBinding{ vkinit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1) };

	VkDescriptorSetLayoutBinding bindings[] = { camBufferBinding, sceneBufferBinding };

	VkDescriptorSetLayoutCreateInfo camSetInfo{};
	camSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	camSetInfo.pNext = nullptr;

	// We currently only have one binding, the camera buffer
	camSetInfo.bindingCount = 2;
	camSetInfo.pBindings = bindings;
	camSetInfo.flags = 0;

	vkCreateDescriptorSetLayout(m_Device, &camSetInfo, nullptr, &m_GlobalSetLayout);
#pragma endregion

#pragma region DescriptorSet 2 (Object)
	VkDescriptorSetLayoutBinding objectBind{vkinit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT, 0) };

	VkDescriptorSetLayoutCreateInfo objectSetInfo{};
	objectSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	objectSetInfo.pNext = nullptr;

	objectSetInfo.bindingCount = 1;
	objectSetInfo.pBindings = &objectBind;
	objectSetInfo.flags = 0;

	vkCreateDescriptorSetLayout(m_Device, &objectSetInfo, nullptr, &m_ObjectSetLayout);
#pragma endregion

#pragma region DescriptorSet 3 (Texture)
	VkDescriptorSetLayoutBinding textureBind{vkinit::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)};

	VkDescriptorSetLayoutCreateInfo texSetInfo{};
	texSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texSetInfo.pNext = nullptr;

	texSetInfo.bindingCount = 1;
	texSetInfo.flags = 0;
	texSetInfo.pBindings = &textureBind;

	vkCreateDescriptorSetLayout(m_Device, &texSetInfo, nullptr, &m_SingleTextureSetLayout);
#pragma endregion

	const size_t sceneDataBufferSize = MAX_FRAMES_IN_FLIGHT * pad_uniform_buffer_size(sizeof(GPUSceneData));
	m_SceneDataBuffer = create_buffer(sceneDataBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	const int MAX_OBJECTS = 10'000;
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		// Object data buffer
		m_Frames[i].object_buffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Camera data buffer
		m_Frames[i].camera_buffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Indirect buffer
		m_Frames[i].indirect_buffer = create_buffer(sizeof(VkDrawIndirectCommand) * 10'000,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// Allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;

		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_GlobalSetLayout;

		vkAllocateDescriptorSets(m_Device, &allocInfo, &m_Frames[i].global_descriptor);

		// Allocate one descriptor set that will point to the object buffer for each frame
		VkDescriptorSetAllocateInfo objectAlloc{};
		objectAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		objectAlloc.pNext = nullptr;

		objectAlloc.descriptorPool = m_DescriptorPool;
		objectAlloc.descriptorSetCount = 1;
		objectAlloc.pSetLayouts = &m_ObjectSetLayout;

		vkAllocateDescriptorSets(m_Device, &objectAlloc, &m_Frames[i].object_descriptor);

		// Info about the camera buffer we want to point at in the descriptor
		VkDescriptorBufferInfo cameraInfo{};
		cameraInfo.buffer = m_Frames[i].camera_buffer.buffer;
		// Offset into the buffer
		cameraInfo.offset = 0;
		// Size of data to pass to the shader
		cameraInfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo sceneInfo{};
		sceneInfo.buffer = m_SceneDataBuffer.buffer;
		sceneInfo.offset = 0;
		sceneInfo.range = sizeof(GPUSceneData);

		VkDescriptorBufferInfo objectBufferInfo{};
		objectBufferInfo.buffer = m_Frames[i].object_buffer.buffer;
		objectBufferInfo.offset = 0;
		objectBufferInfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

		VkWriteDescriptorSet cameraWrite{vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			m_Frames[i].global_descriptor, &cameraInfo, 0)};

		VkWriteDescriptorSet sceneWrite{ vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			m_Frames[i].global_descriptor, &sceneInfo, 1) };

		VkWriteDescriptorSet objectWrite{ vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						m_Frames[i].object_descriptor, &objectBufferInfo, 0) };

		VkWriteDescriptorSet setWrites[] = { cameraWrite, sceneWrite, objectWrite };

		vkUpdateDescriptorSets(m_Device, 3, setWrites, 0, nullptr);
	}

	m_MainDeletionQueue.deletors.emplace_back([&]() {
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_GlobalSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_Device, m_ObjectSetLayout, nullptr);

		// Add buffers to deletion queue
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vmaDestroyBuffer(m_Allocator, m_Frames[i].camera_buffer.buffer, m_Frames[i].camera_buffer.allocation);
			vmaDestroyBuffer(m_Allocator, m_Frames[i].object_buffer.buffer, m_Frames[i].object_buffer.allocation);
		}

		vmaDestroyBuffer(m_Allocator, m_SceneDataBuffer.buffer, m_SceneDataBuffer.allocation);
		});
}

void VulkanEngine::init_def_renderpass()
{
	std::cout << "Initializing Default Render Pass\n";

	// Description of the image that we will be writing rendering commands to
	VkAttachmentDescription colorAttachment{};

	// format should be the same as the swap chain images
	colorAttachment.format = m_SwapChainImageFormat;
	// MSAA samples, set to 1 (no MSAA) by default
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// Clear when render pass begins
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Keep the attachment stored when render pass ends
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// Don't care about stencil data
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Image data layout before render pass starts (undefined = don't care)
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Image data layout after render pass (to change to), set to present by default
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	// Attachment number will index into the pAttachments array in the parent render pass itself
	colorAttachmentRef.attachment = 0;
	// Optimal layout for writing to the image
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Description of the depth image
	VkAttachmentDescription depthAttachment{};
	depthAttachment.flags = 0;
	depthAttachment.format = m_DepthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Create one sub pass (minimum one sub pass required)
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	// Connect depth attachment to subpass
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };

	// Connect the color attachment description to the info
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = &attachments[0];

	// Connect the subpass(es) to the info
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// These dependencies tell Vulkan that the attachment cannot be used before the previous renderpasses have finished using it
	VkSubpassDependency colorDependency{};
	colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	colorDependency.dstSubpass = 0;
	
	colorDependency.srcAccessMask = 0;
	colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubpassDependency depthDependency{};
	depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depthDependency.dstSubpass = 0;

	depthDependency.srcAccessMask = 0;
	depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	VkSubpassDependency dependencies[2] = { colorDependency, depthDependency };

	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = &dependencies[0];

	VK_CHECK(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
		});
}

void VulkanEngine::init_framebuffers()
{
	std::cout << "Initializing Framebuffers\n";

	// Create the framebuffers for the swap chain images.
	// This will connect the render pass to the images for rendering
	VkFramebufferCreateInfo fbInfo{};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	// Connect the render pass to the framebuffer
	fbInfo.renderPass = m_RenderPass;
	fbInfo.attachmentCount = 1;
	fbInfo.width = m_WindowExtent.width;
	fbInfo.height = m_WindowExtent.height;
	fbInfo.layers = 1;

	// Get the number of swap chain image views
	const uint32_t swapChainImageViewCount = static_cast<uint32_t>(m_SwapChainImageViews.size());
	m_Framebuffers = std::vector<VkFramebuffer>(swapChainImageViewCount);

	// Create the framebuffers for each image view
	for (uint32_t i = 0; i < swapChainImageViewCount; i++)
	{
		VkImageView attachments[2] = { m_SwapChainImageViews[i], m_DepthImageView };

		fbInfo.attachmentCount = 2;
		fbInfo.pAttachments = attachments;

		VK_CHECK(vkCreateFramebuffer(m_Device, &fbInfo, nullptr, &m_Framebuffers[i]));

		m_MainDeletionQueue.deletors.emplace_back([=]() {
			vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
			vkDestroyImageView(m_Device, m_SwapChainImageViews[i], nullptr);
			});
	}
}

FrameData& VulkanEngine::get_current_frame()
{
	return m_Frames[m_FrameNumber % MAX_FRAMES_IN_FLIGHT];
}

void VulkanEngine::init_sync_structures()
{
	std::cout << "Initializing Sync Structures\n";

	// Upload fence
	VkFenceCreateInfo uploadFenceCreateInfo{ vkinit::fence_ci() };

	VK_CHECK(vkCreateFence(m_Device, &uploadFenceCreateInfo, nullptr, &m_UploadContext.upload_fence));
	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vkDestroyFence(m_Device, m_UploadContext.upload_fence, nullptr);
		});

	// Render fence
	VkFenceCreateInfo fenceCreateInfo{ vkinit::fence_ci(VK_FENCE_CREATE_SIGNALED_BIT) };
	// Sempahore needs no flags
	VkSemaphoreCreateInfo semaphoreCreateInfo{ vkinit::semaphore_ci() };
	
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].render_fence));

		m_MainDeletionQueue.deletors.emplace_back([=]() {
			vkDestroyFence(m_Device, m_Frames[i].render_fence, nullptr);
			});

		VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].present_semaphore));
		VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].render_semaphore));

		m_MainDeletionQueue.deletors.emplace_back([=]()
			{
				vkDestroySemaphore(m_Device, m_Frames[i].present_semaphore, nullptr);
				vkDestroySemaphore(m_Device, m_Frames[i].render_semaphore, nullptr);
			});
	}
}

bool VulkanEngine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{
	// Open the file stream, seek to the end of the file
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	// Get the file size
	size_t fileSize = static_cast<size_t>(file.tellg());

	// SPRIV expects the buffer to be on uint32_t, so make sure to reserve a buffer big enough for that
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// Put cursor at the beginning of the file
	file.seekg(0);

	// Load the entire file into the buffer
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	// Close the file stream
	file.close();

	// Create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	// Code size has to be in bytes, so multiply the int in the buffer by the size of an int
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	// Validate creation
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return false;

	// Set the outShaderModule to the newly created shader module
	*outShaderModule = shaderModule;

	return true;
}

void VulkanEngine::init_pipelines()
{
	std::cout << "Initializing Pipelines\n";

	VkShaderModule fragShader;
	if (!load_shader_module("shaders/PosNormCol.frag.spv", &fragShader))
		std::cout << "Error building the triangle fragment shader module\n";
	else
		std::cout << "Successfully built the fragment shader module\n";

	VkShaderModule vertShader;
	if (!load_shader_module("shaders/PosNormCol.vert.spv", &vertShader))
		std::cout << "Error building the triangle vertex shader module\n";
	else
		std::cout << "Successfully built the vertex shader module\n";

	// Build pipeline layout that controls inputs/outputs of the shader
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{ vkinit::pipeline_layout_ci() };

	VkDescriptorSetLayout setLayouts[] = { m_GlobalSetLayout, m_ObjectSetLayout, m_SingleTextureSetLayout };

	pipelineLayoutInfo.setLayoutCount = 3;
	pipelineLayoutInfo.pSetLayouts = setLayouts;

	VK_CHECK(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_MeshPipelineLayout));

	// Build the stage-create-info for both vertex and fragment stages.
	// This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	pipelineBuilder.m_ShaderStages.push_back(
		vkinit::pipeline_shader_stage_ci(VK_SHADER_STAGE_VERTEX_BIT, vertShader)
	);

	pipelineBuilder.m_ShaderStages.push_back(
		vkinit::pipeline_shader_stage_ci(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader)
	);

	// Vertex input controls how to read vertices from vertex buffers
	pipelineBuilder.m_VertexInputInfo = vkinit::vertex_input_state_ci();

	VertexInputDescription vertexDesc = Vertex::get_vert_desc();

	pipelineBuilder.m_VertexInputInfo.pVertexAttributeDescriptions = vertexDesc.attributes.data();
	pipelineBuilder.m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexDesc.attributes.size());

	pipelineBuilder.m_VertexInputInfo.pVertexBindingDescriptions = vertexDesc.bindings.data();
	pipelineBuilder.m_VertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexDesc.bindings.size());

	// Set input assembly state, which controls primitive topology
	pipelineBuilder.m_InputAssembly = vkinit::input_assembly_ci(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	// Build the viewport and scissor from the swapchain extents
	pipelineBuilder.m_Viewport.x = 0.0f;
	pipelineBuilder.m_Viewport.y = 0.0f;
	pipelineBuilder.m_Viewport.width = static_cast<float>(m_WindowExtent.width);
	pipelineBuilder.m_Viewport.height = static_cast<float>(m_WindowExtent.height);
	pipelineBuilder.m_Viewport.minDepth = 0.0f;
	pipelineBuilder.m_Viewport.maxDepth = 1.0f;

	pipelineBuilder.m_Scissor.offset = { 0, 0 };
	pipelineBuilder.m_Scissor.extent = m_WindowExtent;

	// Build rasterizer
	pipelineBuilder.m_Rasterizer = vkinit::rasterization_state_ci(VK_POLYGON_MODE_FILL);

	// Build multisampling
	pipelineBuilder.m_Multisampling = vkinit::multisample_state_ci();

	// Build color blend attachment with no blending and writing to RGBA
	pipelineBuilder.m_ColorBlendAttachment = vkinit::color_blend_attachment_state();

	// Use the triangle layout
	pipelineBuilder.m_PipelineLayout = m_MeshPipelineLayout;

	// Depth testing
	pipelineBuilder.m_DepthStencil = vkinit::depth_stencil_ci(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	// Build the pipeline
	m_MeshPipeline = pipelineBuilder.build_pipeline(m_Device, m_RenderPass);

	create_material("defaultmesh", m_MeshPipeline, m_MeshPipelineLayout);

	vkDestroyShaderModule(m_Device, vertShader, nullptr);
	vkDestroyShaderModule(m_Device, fragShader, nullptr);

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vkDestroyPipeline(m_Device, m_MeshPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_MeshPipelineLayout, nullptr);
		});
}

void VulkanEngine::load_meshes()
{
	m_TeapotMesh.load_from_obj("../../../../assets/teapot.obj");

	upload_mesh(m_TeapotMesh);
	m_Meshes["Teapot"] = m_TeapotMesh;

	//Mesh lost_empire{};
	//lost_empire.load_from_obj("../../../../assets/lost_empire.obj");

	//upload_mesh(lost_empire);
	//m_Meshes["empire"] = lost_empire;
}

void VulkanEngine::upload_mesh(Mesh& mesh)
{
	const size_t bufferSize{ mesh.vertices.size() * sizeof(Vertex) };

	// Allocate CPU side vertex buffer
	VkBufferCreateInfo stagingBufferInfo{};
	stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferInfo.pNext = nullptr;

	// Total size, in bytes, of the buffer
	// Should hold all vertices
	stagingBufferInfo.size = bufferSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// This data should be on CPU RAM
	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	AllocatedBuffer stagingBuffer;

	VK_CHECK(vmaCreateBuffer(m_Allocator, &stagingBufferInfo, &vmaallocInfo,
		&stagingBuffer.buffer,
		&stagingBuffer.allocation,
		nullptr));

	// Copy vertex data into buffer
	void* data;
	vmaMapMemory(m_Allocator, stagingBuffer.allocation, &data);
	memcpy(data, mesh.vertices.data(), bufferSize);
	vmaUnmapMemory(m_Allocator, stagingBuffer.allocation);

	// Allocate GPU side vertex buffer
	VkBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.pNext = nullptr;

	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VK_CHECK(vmaCreateBuffer(m_Allocator, &vertexBufferInfo, &vmaallocInfo,
		&mesh.vertexBuffer.buffer,
		&mesh.vertexBuffer.allocation,
		nullptr));

	// Copy data from CPU to GPU
	immediate_submit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = bufferSize;
		vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh.vertexBuffer.buffer, 1, &copy);
		});

	m_MainDeletionQueue.deletors.emplace_back([=]() {
		vmaDestroyBuffer(m_Allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
		});

	vmaDestroyBuffer(m_Allocator, stagingBuffer.buffer, stagingBuffer.allocation);
}

size_t VulkanEngine::pad_uniform_buffer_size(size_t originalSize)
{
	size_t minUboAlignment = m_DeviceProperties.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}

Material* VulkanEngine::create_material(const std::string& name, VkPipeline pipeline, VkPipelineLayout layout)
{
	Material mat;
	mat.pipeline = pipeline;
	mat.pipeline_layout = layout;
	m_Materials[name] = mat;
	return &m_Materials[name];
}

Material* VulkanEngine::get_material(const std::string& name)
{
	auto it = m_Materials.find(name);
	if (it == end(m_Materials))
		return nullptr;
	else
		return &(*it).second;
}

Mesh* VulkanEngine::get_mesh(const std::string& name)
{
	auto it = m_Meshes.find(name);
	if (it == end(m_Meshes))
		return nullptr;
	else
		return &(*it).second;
}

void Coral3D::VulkanEngine::load_images()
{
	//Texture lost_empire;

	//vkutil::load_image_from_file(*this, "../../../../assets/lost_empire-RGBA.png", lost_empire.image);

	//VkImageViewCreateInfo img_info = vkinit::image_view_ci(VK_FORMAT_R8G8B8A8_SRGB, lost_empire.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	//vkCreateImageView(m_Device, &img_info, nullptr, &lost_empire.image_view);

	//m_LoadedTextures["empire_diffuse"] = lost_empire;

	Texture uv_checker;

	vkutil::load_image_from_file(*this, "../../../../assets/uv_checker.jpg", uv_checker.image);

	VkImageViewCreateInfo img_info = vkinit::image_view_ci(VK_FORMAT_R8G8B8A8_SRGB, uv_checker.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(m_Device, &img_info, nullptr, &uv_checker.image_view);

	m_LoadedTextures["checker_diffuse"] = uv_checker;
}

void VulkanEngine::draw_objects(VkCommandBuffer cmdBuffer, RenderObject* first, int count)
{
	glm::vec3 camPos{0.f, -30.f, -1310.f};
	// glm::vec3 camPos{0.f, -5.f, 0.f};

	// Camera matrices
	glm::mat4 trans{glm::translate(glm::mat4{1.f}, camPos)};
	glm::mat4 rot{glm::rotate(glm::radians(25.f), glm::vec3{1, 0, 0})};
	glm::mat4 view = rot * trans;
	glm::mat4 proj{glm::perspective(glm::radians(70.f), (float)m_WindowExtent.width / (float)m_WindowExtent.height, 0.1f, 10000.f)};
	proj[1][1] *= -1;

	GPUCameraData camData;
	camData.proj = proj;
	camData.view = view;
	camData.view_proj = proj * view;
	camData.view_inv = glm::inverse(view);

	// Camera data
	void* data;
	vmaMapMemory(m_Allocator, get_current_frame().camera_buffer.allocation, &data);
	memcpy(data, &camData, sizeof(GPUCameraData));
	vmaUnmapMemory(m_Allocator, get_current_frame().camera_buffer.allocation);

	// Scene data
	char* sceneData;
	vmaMapMemory(m_Allocator, m_SceneDataBuffer.allocation, (void**)&sceneData);
	
	int frameIndex = m_FrameNumber % MAX_FRAMES_IN_FLIGHT;
	sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
	
	memcpy(sceneData, &m_GPUSceneData, sizeof(GPUSceneData));
	vmaUnmapMemory(m_Allocator, m_SceneDataBuffer.allocation);

	// Object data
	void* objectData;
	vmaMapMemory(m_Allocator, get_current_frame().object_buffer.allocation, &objectData);
	GPUObjectData* objectSSBO = (GPUObjectData*)objectData;

	for (int i = 0; i < count; i++)
	{
		RenderObject& object = first[i];

		objectSSBO[i].world = object.transform * 
			glm::rotate(glm::mat4{1.f}, glm::radians(m_FrameNumber * 0.4f), glm::vec3(0, 0, 1));
	}

	vmaUnmapMemory(m_Allocator, get_current_frame().object_buffer.allocation);

	auto draws = batch_draws(m_Renderables);

	void* drawData;
	vmaMapMemory(m_Allocator, get_current_frame().indirect_buffer.allocation, &drawData);

	VkDrawIndirectCommand* drawCommands = reinterpret_cast<VkDrawIndirectCommand*>(drawData);

	for (int i = 0; i < count; i++)
	{
		RenderObject& object{ first[i] };
		drawCommands[i].vertexCount = object.mesh->vertices.size();
		drawCommands[i].instanceCount = 1;
		drawCommands[i].firstVertex = 0;
		drawCommands[i].firstInstance = i;
	}

	vmaUnmapMemory(m_Allocator, get_current_frame().indirect_buffer.allocation);

	for (IndirectBatch& draw : draws)
	{
		// Bind pipeline
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline);

#pragma region Bind Descriptors
		uint32_t uniformOffset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

		// Bind global descriptor
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			draw.material->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 1, &uniformOffset);

		// Bind object descriptor
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			draw.material->pipeline_layout, 1, 1, &get_current_frame().object_descriptor, 0, nullptr);

		// Bind texture descriptor (if applicable)
		if (draw.material->texture_set != VK_NULL_HANDLE)
			vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline_layout, 2, 1, &draw.material->texture_set, 0, nullptr);
#pragma endregion

		// Bind mesh
		VkDeviceSize offset{ 0 };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &draw.mesh->vertexBuffer.buffer, &offset);

		uint32_t draw_stride = sizeof(VkDrawIndirectCommand);
		VkDeviceSize indirectOffset = draw.first * draw_stride;
		
		vkCmdDrawIndirect(cmdBuffer, get_current_frame().indirect_buffer.buffer, indirectOffset, draw.count, draw_stride);
	}

	//Mesh* lastMesh = nullptr;
	//Material* lastMaterial = nullptr;
	//for (int i = 0; i < count; i++)
	//{
	//	RenderObject& object = first[i];

	//	// Only bind pipeline if it doesn't match with already bound one
	//	if (object.material != lastMaterial)
	//	{
	//		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
	//		lastMaterial = object.material;

	//		// Offset in scene buffer to access current frame data
	//		uint32_t uniformOffset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
	//		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 1, &uniformOffset);
	//	
	//		// Object data descriptor
	//		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 1, 1, &get_current_frame().object_descriptor, 0, nullptr);
	//	
	//		if (object.material->texture_set != VK_NULL_HANDLE)
	//			vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 2, 1, &object.material->texture_set, 0, nullptr);
	//	}

	//	// Only bind mesh if it's different from last bind
	//	if (object.mesh != lastMesh)
	//	{
	//		// Bind the mesh vertex buffer with offset 0
	//		VkDeviceSize offset{ 0 };
	//		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
	//		lastMesh = object.mesh;
	//	}

	//	vkCmdDraw(cmdBuffer, object.mesh->vertices.size(), 1, 0, i);
	//}
}

void VulkanEngine::init_scene()
{
	Material* defaultMaterial = get_material("defaultmesh");

	VkSamplerCreateInfo samplerInfo{ vkinit::sampler_ci(VK_FILTER_NEAREST) };

	VkSampler nearesetSampler;
	vkCreateSampler(m_Device, &samplerInfo, nullptr, &nearesetSampler);

	// Allocate descriptor set for texture to use on the material
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;

	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_SingleTextureSetLayout;

	vkAllocateDescriptorSets(m_Device, &allocInfo, &defaultMaterial->texture_set);

	// Make descriptor set point to texture
	VkDescriptorImageInfo imageBufferInfo{};
	imageBufferInfo.sampler = nearesetSampler;
	imageBufferInfo.imageView = m_LoadedTextures["checker_diffuse"].image_view;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture{ vkinit::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, defaultMaterial->texture_set, &imageBufferInfo, 0) };
	vkUpdateDescriptorSets(m_Device, 1, &texture, 0, nullptr);

	//RenderObject map;
	//map.mesh = get_mesh("empire");
	//map.material = get_material("defaultmesh");
	//map.transform = glm::translate(glm::vec3{5, -10, 0});

	//std::cout << std::setprecision(10) << "Number of triangles: " << map.mesh->vertices.size() << "\n";

	//m_Renderables.emplace_back(map);

	Mesh* teapotMesh = get_mesh("Teapot");
	const int vertCount = teapotMesh->vertices.size();
	 
	 constexpr int numObjects = 65;
	for (int x = -numObjects * 0.5f; x < numObjects * 0.5f; x++)
	{
		for (int y = -numObjects * 0.5f; y < numObjects * 0.5f; y++)
		{
			RenderObject teapot;
			teapot.mesh = teapotMesh;
			teapot.material = defaultMaterial;
			teapot.transform = glm::translate(glm::mat4{1.0f}, glm::vec3(x * 40.f, 0, y * 40.f)) * glm::rotate(glm::radians(-90.f), glm::vec3{1, 0, 0});

			m_Renderables.push_back(teapot);
		}
	}

	// Scene data
	m_GPUSceneData.ambient_color = { 0.625f, 0.0f, 0.24f, 0.1f };
	m_GPUSceneData.light_direction = { -0.577f, -0.577f, 0.577f, 1.f };

	std::cout << "Number of renderables: " << m_Renderables.size() << "\n";
	std::cout << std::setprecision(0) << "Number of triangles: " << (m_Renderables.size() * vertCount) / 3 << "\n";


}