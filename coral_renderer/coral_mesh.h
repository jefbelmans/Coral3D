#pragma once

// STD
#include <vector>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

#include "vk_types.h"
#include "coral_device.h"

namespace coral_3d
{
	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};


	struct Vertex
	{
		glm::vec2 position;

		static VertexInputDescription get_vert_desc();
	};

	struct VertexBig
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static VertexInputDescription get_vert_desc();
	};

	class coral_mesh final
	{
	public:
		coral_mesh(coral_device& device, const std::vector<Vertex>& vertices);
		~coral_mesh();
		
		coral_mesh(const coral_mesh&) = delete;
		coral_mesh& operator=(const coral_mesh&) = delete;

		void bind(VkCommandBuffer command_buffer);
		void draw(VkCommandBuffer command_buffer);

	private:
		void create_vertex_buffers();

		coral_device& device_;
		std::vector<Vertex> vertices_;
		uint32_t vertex_count_;
		AllocatedBuffer vertex_buffer_;

		bool load_from_obj(const char* filename);
	};
}
