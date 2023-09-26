#pragma once

// STD
#include <vector>
#include <memory>

#include "tiny_gltf.h"

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

#include "vk_types.h"
#include "coral_device.h"
#include "coral_buffer.h"
#include "coral_texture.h"
#include "coral_descriptors.h"

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
        glm::vec4 tangent;
		glm::vec2 uv;

		static VertexInputDescription get_vert_desc();

		bool operator==(const Vertex& other) const
		{
			return
				position == other.position &&
				normal == other.normal &&
                tangent == other.tangent &&
				uv == other.uv;
		}
	};

    struct Primitive
    {
        uint32_t first_index;
        uint32_t index_count;
        int32_t material_index;
    };

    struct Mesh
    {
        std::vector<Primitive> primitives;
    };

    struct Node
    {
        Node* parent;
        std::vector<Node*> children;
        Mesh mesh;
        glm::mat4 transform;
        std::string name;
        bool visible = true;
        ~Node()
        {
            for(auto& child : children)
                delete child;
        }
    };

    struct Material
    {
        glm::vec4 base_color_factor = glm::vec4{1.f};
        uint32_t base_color_texture_index;
        uint32_t normal_texture_index;

        std::string alpha_mode = "OPAQUE";
        float alpha_cutoff;
        bool double_sided = false;

        VkDescriptorSet descriptor_set;
        VkPipeline pipeline;
    };

    struct Image
    {
        std::unique_ptr<coral_texture> texture;
    };

    struct Texture
    {
        int32_t image_index;
    };

	class coral_mesh final
	{
	public:
		struct Builder
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
            std::vector<Image> images;
            std::vector<Texture> textures;
            std::vector<Material> materials;
            std::vector<Node*> nodes;

            bool load_from_gltf(coral_device& device, const std::string& file_path);
            void load_images(coral_device& device, tinygltf::Model& input);
            void load_textures(tinygltf::Model& input);
            void load_materials(tinygltf::Model& input);
            void load_node(coral_device& device, const tinygltf::Node& input_node, const tinygltf::Model& input, coral_3d::Node* parent);
		};

		coral_mesh(coral_device& device, Builder& builder);
        ~coral_mesh();
		coral_mesh(const coral_mesh&) = delete;
		coral_mesh& operator=(const coral_mesh&) = delete;

		static std::unique_ptr<coral_mesh> create_mesh_from_file(coral_device& device, const std::string& file_path);

        void load_materials(coral_descriptor_set_layout& material_set_layout, coral_descriptor_pool& material_set_pool);
        void create_pipelines(
                const std::string &vert_file_path,
                const std::string &frag_file_path,
                VkRenderPass render_pass,
                VkPipelineLayout pipeline_layout);

		void draw(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout);

	private:
        VkDescriptorImageInfo get_texture_descriptor(size_t index);

        void draw_node(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, coral_3d::Node* node);

        void create_vertex_buffers(const std::vector<Vertex>& vertices);
		void create_index_buffers(const std::vector<uint32_t>& indices);

		coral_device& device_;

		std::unique_ptr<coral_buffer> vertex_buffer_;
		std::unique_ptr<coral_buffer> index_buffer_;

        std::vector<Image> images_;
        std::vector<Texture> textures_;
        std::vector<Material> materials_;
        std::vector<Node*> nodes_;
	};
}
