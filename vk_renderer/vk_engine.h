#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <functional>

// GLM
#include <vec4.hpp>
#include <mat4x4.hpp>

#include "vk_mesh.h"
#include "vk_types.h"

class GLFWwindow;
namespace Coral3D
{
	struct DeletionQueue
	{
		std::deque <std::function<void()>> deletors;

		void Flush()
		{
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
				(*it)();

			deletors.clear();
		}
	};

	struct Material
	{
		VkDescriptorSet texture_set{ VK_NULL_HANDLE };
		VkPipeline pipeline;
		VkPipelineLayout pipeline_layout;
	};

	struct Texture
	{
		AllocatedImage image;
		VkImageView image_view;
	};

	struct RenderObject
	{
		Mesh* mesh;
		Material* material;
		glm::mat4 transform;
	};

	struct GPUCameraData
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 view_proj;
		glm::mat4 view_inv;
	};

	struct GPUSceneData
	{
		glm::vec4 fog_color;
		glm::vec4 fog_distance;
		glm::vec4 ambient_color; // w is ambient power
		glm::vec4 light_direction; // w is light power
		glm::vec4 light_color;
	};

	struct GPUObjectData
	{
		glm::mat4 world;
	};

	struct UploadContext
	{
		VkFence upload_fence;
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	};

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

	struct IndirectBatch
	{
		Mesh* mesh;
		Material* material;
		uint32_t first;
		uint32_t count;
	};

	// Number of frames that can be processed concurrently
	static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

	class VulkanEngine final
	{
	public:
		void init();
		void run();
		void cleanup();

		// Allocator
		VmaAllocator m_Allocator{ VK_NULL_HANDLE };
		AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

		void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

		// Deletion
		DeletionQueue m_MainDeletionQueue;

	private:
		void draw();
		std::vector<IndirectBatch> batch_draws(const std::vector<RenderObject>& renderObjects);

		// GLFW window
		GLFWwindow* m_pWindow{ nullptr };
		VkExtent2D m_WindowExtent{ 1280, 720 };
		void init_window();

		// Upload context
		UploadContext m_UploadContext;

		// Engine stats
		bool m_IsInitialized{ false };
		int m_FrameNumber{ 0 };

		// Vulkan Instance
		VkInstance m_Instance{ VK_NULL_HANDLE }; // Vulkan instance handle
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE }; // Vulkan debug output handle
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE }; // Vulkan physical device handle
		VkDevice m_Device{ VK_NULL_HANDLE }; // Vulkan logical device handle
		VkSurfaceKHR m_Surface{ VK_NULL_HANDLE }; // Vulkan surface handle
		void init_vulkan();

		// Swap chain
		VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };
		VkFormat m_SwapChainImageFormat{ VK_FORMAT_UNDEFINED };
		std::vector<VkImage> m_SwapChainImages;
		std::vector<VkImageView> m_SwapChainImageViews;

		void init_swapchain();

		// Queues & buffers
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
		uint32_t m_GraphicsQueueFamily{ UINT32_MAX };
		VkPhysicalDeviceProperties m_DeviceProperties;
		void init_commands();

		// Descriptor sets
		VkDescriptorSetLayout m_GlobalSetLayout{ VK_NULL_HANDLE };
		VkDescriptorSetLayout m_ObjectSetLayout{ VK_NULL_HANDLE };
		VkDescriptorSetLayout m_SingleTextureSetLayout{ VK_NULL_HANDLE };

		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
		void init_descriptors();

		// Render passes
		VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> m_Framebuffers;
		void init_def_renderpass();
		void init_framebuffers();

		// Render loop
		FrameData m_Frames[MAX_FRAMES_IN_FLIGHT];
		FrameData& get_current_frame();
		void init_sync_structures();

		// Shaders
		bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);
		void init_pipelines();

		// Graphics pipeline
		VkPipelineLayout m_MeshPipelineLayout{ VK_NULL_HANDLE };
		VkPipeline m_MeshPipeline{ VK_NULL_HANDLE };

		// Mesh
		Mesh m_TeapotMesh;
		void load_meshes();
		void upload_mesh(Mesh& mesh);

		// Depth buffer
		VkImageView m_DepthImageView{ VK_NULL_HANDLE };
		AllocatedImage m_DepthImage;
		VkFormat m_DepthFormat;

		// Scene management
		std::vector<RenderObject> m_Renderables;

		std::unordered_map<std::string, Material> m_Materials;
		std::unordered_map<std::string, Mesh> m_Meshes;

		GPUSceneData m_GPUSceneData;
		AllocatedBuffer m_SceneDataBuffer;
		size_t pad_uniform_buffer_size(size_t originalSize);

		// Meshes
		Material* create_material(const std::string& name, VkPipeline pipeline, VkPipelineLayout pipelineLayout);
		Material* get_material(const std::string& name);
		Mesh* get_mesh(const std::string& name);

		// Images
		std::unordered_map<std::string, Texture> m_LoadedTextures;
		void load_images();

		void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);
		void init_scene();
	};
}