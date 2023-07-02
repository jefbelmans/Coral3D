#pragma once

#include <vector>
#include <vec2.hpp>
#include <vec3.hpp>

#include "vk_types.h"

namespace Coral3D
{
	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;

		static VertexInputDescription get_vert_desc();
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		AllocatedBuffer vertexBuffer;

		bool load_from_obj(const char* filename);
	};
}
