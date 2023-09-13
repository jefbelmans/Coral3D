#include "coral_mesh.h"

// LIBS
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT // Ensures robust triangulation
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

// STD
#include <iostream>
#include <cassert>
#include <unordered_map>

#include "coral_utils.h"

using namespace coral_3d;

namespace std
{
    template<>
    struct hash<coral_3d::Vertex>
    {
        size_t operator()(coral_3d::Vertex const &vertex) const
        {
            size_t seed = 0;
            coral_3d::utils::hash_combine(seed, vertex.position, vertex.tangent, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

VertexInputDescription Vertex::get_vert_desc()
{
    VertexInputDescription desc;

    // We only have one vertex buffer binding, with a per-vertex rate
    VkVertexInputBindingDescription main_binding{};
    main_binding.binding = 0;
    main_binding.stride = sizeof(Vertex);
    main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    desc.bindings_.emplace_back(main_binding);

    // Position will be stored at Location 0
    VkVertexInputAttributeDescription position_attrib{};
    position_attrib.binding = 0;
    position_attrib.location = 0;
    position_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    position_attrib.offset = offsetof(Vertex, position);

    // Normal will be stored at Location 1
    VkVertexInputAttributeDescription normal_attrib{};
    normal_attrib.binding = 0;
    normal_attrib.location = 1;
    normal_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attrib.offset = offsetof(Vertex, normal);

    // Tangent will be stored at Location 2
    VkVertexInputAttributeDescription tangent_attrib{};
    tangent_attrib.binding = 0;
    tangent_attrib.location = 2;
    tangent_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    tangent_attrib.offset = offsetof(Vertex, tangent);

    // UV will be stored at Location 3
    VkVertexInputAttributeDescription texcoord_attrib{};
    texcoord_attrib.binding = 0;
    texcoord_attrib.location = 3;
    texcoord_attrib.format = VK_FORMAT_R32G32_SFLOAT;
    texcoord_attrib.offset = offsetof(Vertex, uv);

    desc.attributes.emplace_back(position_attrib);
    desc.attributes.emplace_back(normal_attrib);
    desc.attributes.emplace_back(tangent_attrib);
    desc.attributes.emplace_back(texcoord_attrib);

    return desc;
}

bool coral_mesh::Builder::load_from_obj(coral_device& device, const std::string& file_path)
{
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "assets/materials";

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(file_path, reader_config))
    {
        // If there is an error loading the mesh, output it to the console and return false
        // This most likely happens when the file can't be found or is corrupted / malformed
        if (!reader.Error().empty())
        {
            std::cerr << "ERROR! coral_mesh::load_from_obj() >> " << reader.Error() << std::endl;
            return false;
        }
    }

    // Output any warnings to the console
    if (!reader.Warning().empty())
    {
        std::cout << "WARNING! coral_mesh::load_from_obj() >> " << reader.Warning() << std::endl;
    }

    // attrib will contain the vertex arrays of the file
    auto& attrib = reader.GetAttrib();
    // shapes contains the info for each separate object in the file
    auto& shapes = reader.GetShapes();
    // materials contains the information about the material of each shape, not yet used.
    auto& materials = reader.GetMaterials();

    std::cout << "[" << file_path << "] shape count: " << shapes.size() << std::endl;
    std::cout << "[" << file_path << "] material count: " << materials.size() << std::endl;

    sub_meshes.reserve(shapes.size());

    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    // Loop over shapes
    for (const auto& shape : shapes)
    {
        const int material_id {shape.mesh.material_ids[0]};
        std::cout << "[" << file_path << "] >> shape [" << shape.name << "] has material [" << material_id << ": \"" << materials[material_id].name << "\"]" << std::endl;
        uint32_t index_count{};

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            if (index.vertex_index >= 0)
            {
                vertex.position = 
                { 
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
            }                                                                      

            if (index.normal_index >= 0)
            {
                vertex.normal =
                {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if (index.texcoord_index >= 0)
            {
                vertex.uv =
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (unique_vertices.count(vertex) == 0)
            {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.emplace_back(vertex);
            }
            indices.emplace_back(unique_vertices[vertex]);
            index_count++;
        }

        sub_meshes.emplace_back(
                index_count, indices.size() - index_count,
                std::make_shared<coral_material>(device, materials[material_id]));

        std::cout << "[" << file_path << "] Created sub mesh with " << sub_meshes.back().index_count  << " indices and a first index of " << sub_meshes.back().first_index << std::endl;
    }

    // Tangent calculation
    for (uint32_t i = 0; i < indices.size(); i += 3)
    {
        uint32_t index0 = indices[i];
        uint32_t index1 = indices[size_t(i) + 1];
        uint32_t index2 = indices[size_t(i) + 2];

        const glm::vec3& p0 = vertices[index0].position;
        const glm::vec3& p1 = vertices[index1].position;
        const glm::vec3& p2 = vertices[index2].position;
        const glm::vec2& uv0 = vertices[index0].uv;
        const glm::vec2& uv1 = vertices[index1].uv;
        const glm::vec2& uv2 = vertices[index2].uv;

        const glm::vec3 edge0 = p1 - p0;
        const glm::vec3 edge1 = p2 - p0;
        const glm::vec2 deltaUV0 = uv1 - uv0;
        const glm::vec2 deltaUV1 = uv2 - uv0;

        float r = 1.f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

        glm::vec3 tangent
        {
            r * (deltaUV1.y * edge0.x - deltaUV0.y * edge1.x),
            r * (deltaUV1.y * edge0.y - deltaUV0.y * edge1.y),
            r * (deltaUV1.y * edge0.z - deltaUV0.y * edge1.z),
        };
        vertices[index0].tangent = tangent;
        vertices[index1].tangent = tangent;
        vertices[index2].tangent = tangent;
    }

    //Fix the tangents per vertex now because we accumulated


    return true;
}

coral_mesh::coral_mesh(coral_device& device, const Builder& builder)
    : device_{device}
{
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
    sub_meshes_ = builder.sub_meshes;
}

std::unique_ptr<coral_mesh> coral_mesh::create_mesh_from_file(coral_device& device, const std::string& file_path)
{
    Builder builder{};
    builder.load_from_obj(device, file_path);
    std::cout << "[" << file_path << "] vertex count: " << builder.vertices.size() << "\n";
    std::cout << "[" << file_path << "] index count: " << builder.indices.size() << "\n";

    return std::make_unique<coral_mesh>(device, builder);
}

void coral_mesh::load_materials(VkPipelineLayout pipeline_layout,
                                coral_descriptor_set_layout& material_set_layout,
                                coral_descriptor_pool& material_set_pool)
{
    for(auto& sub_mesh : sub_meshes_)
        sub_mesh.material->load(pipeline_layout, material_set_layout, material_set_pool);
}

void coral_mesh::bind(VkCommandBuffer command_buffer)
{
    VkBuffer buffers[]{ vertex_buffer_->get_buffer().buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if (has_index_buffer())
    {
        vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void coral_mesh::draw(VkCommandBuffer command_buffer)
{
    if (has_index_buffer())
    {
        for(auto& sub_mesh : sub_meshes_)
        {
            sub_mesh.material->bind(command_buffer);
            vkCmdDrawIndexed(command_buffer, sub_mesh.index_count, 1, sub_mesh.first_index, 0, 0);
        }
    }
    else
        vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
}

void coral_mesh::create_vertex_buffers(const std::vector<Vertex>& vertices)
{
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "ERROR! coral_mesh::create_vertex_buffers() >> Vertex count must be greater than or equal to 3");

    uint32_t vertex_size{ sizeof(vertices[0]) };
    VkDeviceSize buffer_size{ vertex_size * vertex_count_};

    coral_buffer staging_buffer
    {
        device_,
		vertex_size,
        vertex_count_,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    };

    vertex_buffer_ = std::make_unique<coral_buffer>(
        device_,
        vertex_size,
        vertex_count_,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)vertices.data());

    device_.copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);

}

void coral_mesh::create_index_buffers(const std::vector<uint32_t>& indices)
{
    index_count_ = static_cast<uint32_t>(indices.size());

    if (!has_index_buffer()) return;

    uint32_t index_size{ sizeof(indices[0]) };
    VkDeviceSize buffer_size{ index_size * index_count_ };


    coral_buffer staging_buffer
    {
        device_,
		index_size,
		index_count_,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    };

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)indices.data());

    index_buffer_ = std::make_unique<coral_buffer>(
		device_,
		index_size,
		index_count_,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

    device_.copy_buffer(staging_buffer.get_buffer(), index_buffer_->get_buffer(), buffer_size);
}