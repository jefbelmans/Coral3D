#pragma once

// STD
#include <vector>
#include <memory>

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

// TINY GLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "vk_types.h"
#include "coral_device.h"
#include "coral_buffer.h"
#include "coral_material.h"

namespace coral_3d
{
	struct VertexInputDescription
	{
		std::vector<VkVertexInputBindingDescription> bindings_;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
        glm::vec3 tangent;
		glm::vec2 uv;

		static VertexInputDescription get_vert_desc();

		bool operator==(const Vertex& other) const
		{
			return
				position == other.position &&
				normal == other.normal &&
				uv == other.uv &&
				tangent == other.tangent;
		}
	};

    struct SubMesh
    {
        uint32_t index_count;
        uint32_t first_index;

        std::shared_ptr<coral_material> material;
    };

	class coral_mesh final
	{
	public:
		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
            std::vector<SubMesh> sub_meshes{};

			bool load_from_obj(coral_device& device, const std::string& file_path);
            bool load_from_gltf(coral_device& device, const std::string& file_path);
		};

		coral_mesh(coral_device& device, const Builder& builder);
		
		coral_mesh(const coral_mesh&) = delete;
		coral_mesh& operator=(const coral_mesh&) = delete;

		static std::unique_ptr<coral_mesh> create_mesh_from_file(coral_device& device, const std::string& file_path);

        void load_materials(VkPipelineLayout pipeline_layout,
                            coral_descriptor_set_layout& material_set_layout,
                            coral_descriptor_pool& material_set_pool);
		void bind(VkCommandBuffer command_buffer);
		void draw(VkCommandBuffer command_buffer);

		uint32_t get_index_count() const { return index_count_; }
		uint32_t get_vertex_count() const { return vertex_count_; }

	private:
		void create_vertex_buffers(const std::vector<Vertex>& vertices);
		void create_index_buffers(const std::vector<uint32_t>& indices);

		bool has_index_buffer() const { return index_count_ > 0; }

		coral_device& device_;

		uint32_t vertex_count_;
		std::unique_ptr<coral_buffer> vertex_buffer_;

		uint32_t index_count_;
		std::unique_ptr<coral_buffer> index_buffer_;

        std::vector<SubMesh> sub_meshes_;
	};
}
