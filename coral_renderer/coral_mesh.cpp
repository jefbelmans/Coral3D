#include "coral_mesh.h"

// LIBS
#define TINYOBJLOADER_IMPLEMENTATION
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
            coral_3d::utils::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
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

    desc.bindings.emplace_back(main_binding);

    // Position will be stored at Location 0
    VkVertexInputAttributeDescription position_attrib{};
    position_attrib.binding = 0;
    position_attrib.location = 0;
    position_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    position_attrib.offset = offsetof(Vertex, position);

    //// Normal will be stored at Location 1
    VkVertexInputAttributeDescription normal_attrib{};
    normal_attrib.binding = 0;
    normal_attrib.location = 1;
    normal_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attrib.offset = offsetof(Vertex, normal);

    //// UV will be stored at Location 2
    VkVertexInputAttributeDescription texcoord_attrib{};
    texcoord_attrib.binding = 0;
    texcoord_attrib.location = 2;
    texcoord_attrib.format = VK_FORMAT_R32G32_SFLOAT;
    texcoord_attrib.offset = offsetof(Vertex, uv);

    // Color will be stored at Location 3
    VkVertexInputAttributeDescription color_attrib{};
    color_attrib.binding = 0;
    color_attrib.location = 3;
    color_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attrib.offset = offsetof(Vertex, color);


    desc.attributes.emplace_back(position_attrib);
    desc.attributes.emplace_back(normal_attrib);
    desc.attributes.emplace_back(texcoord_attrib);
    desc.attributes.emplace_back(color_attrib);

    return desc;
}

bool Builder::load_from_obj(const std::string& file_path)
{
    // attrib will contain the vertex arrays of the file
    tinyobj::attrib_t attrib;
    // shapes contains the info for each separate object in the file
    std::vector<tinyobj::shape_t> shapes;
    // materials contains the information about the material of each shape, not yet used.
    std::vector<tinyobj::material_t> materials;

    // error and warning output from the load function
    std::string warn;
    std::string err;

    // load the OBJ file
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str());

    //make sure to output the warnings to the console, in case there are issues with the file
    if (!warn.empty())
    {
        std::cout << "WARNING! coral_mesh::load_from_obj() >> " << warn << std::endl;
    }

    //if we have any error, print it to the console, and break the mesh loading.
    //This happens if the file can't be found or is malformed
    if (!err.empty())
    {
        std::cerr << err << std::endl;
        return false;
    }

    std::unordered_map<Vertex, uint32_t> unique_verticies{};

    // Loop over shapes
    for (const auto& shape : shapes)
    {
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

                // If color is provided after position
                auto color_index = 3 * index.vertex_index + 2;
                if (color_index < attrib.colors.size())
                {
                    vertex.color =
                    {
                        attrib.colors[color_index - 2],
                        attrib.colors[color_index - 1],
                        attrib.colors[color_index - 0]
                    };
                }
                else
                    vertex.color = { 1.f, 1.f, 1.f }; // Default color
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
                    attrib.normals[3 * index.texcoord_index + 0],
                    attrib.normals[3 * index.texcoord_index + 1]
                };
            }

            if (unique_verticies.count(vertex) == 0)
            {
                unique_verticies[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.emplace_back(vertex);
            }
            indices.emplace_back(unique_verticies[vertex]);
        }
    }

    return true;
}

coral_mesh::coral_mesh(coral_device& device, const Builder& builder)
    : device_{device}
{
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
}

coral_mesh::~coral_mesh()
{
    vmaDestroyBuffer(device_.allocator(), vertex_buffer_.buffer, vertex_buffer_.allocation);

    if(has_index_buffer())
        vmaDestroyBuffer(device_.allocator(), index_buffer_.buffer, index_buffer_.allocation);
}

std::unique_ptr<coral_mesh> coral_mesh::create_mesh_from_file(coral_device& device, const std::string& file_path)
{
    Builder builder{};
    builder.load_from_obj(file_path);
    std::cout << "[" << file_path << "] vertex count: " << builder.vertices.size() << "\n";
    std::cout << "[" << file_path << "] index count: " << builder.indices.size() << "\n";

    return std::make_unique<coral_mesh>(device, builder);
}

void coral_mesh::bind(VkCommandBuffer command_buffer)
{
    VkBuffer buffers[]{ vertex_buffer_.buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if (has_index_buffer())
    {
        vkCmdBindIndexBuffer(command_buffer, index_buffer_.buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void coral_mesh::draw(VkCommandBuffer command_buffer)
{
    if (has_index_buffer())
        vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
    else
        vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
}

void coral_mesh::create_index_buffers(const std::vector<uint32_t>& indices)
{
    index_count_ = static_cast<uint32_t>(indices.size());

    if (!has_index_buffer()) return;

    VkDeviceSize buffer_size{ sizeof(uint32_t) * index_count_ };
    AllocatedBuffer staging_buffer{};
    staging_buffer = device_.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;
    vmaMapMemory(device_.allocator(), staging_buffer.allocation, &data);
    memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
    vmaUnmapMemory(device_.allocator(), staging_buffer.allocation);

    index_buffer_ = device_.create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    device_.copy_buffer(staging_buffer, index_buffer_, buffer_size);

    vmaDestroyBuffer(device_.allocator(), staging_buffer.buffer, staging_buffer.allocation);
}

void coral_mesh::create_vertex_buffers(const std::vector<Vertex>& vertices)
{
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "ERROR! coral_mesh::create_vertex_buffers() >> Vertex count must be greater than or equal to 3");

    VkDeviceSize buffer_size{ sizeof(Vertex) * vertex_count_ };
    
    AllocatedBuffer staging_buffer{};
    staging_buffer = device_.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;
    vmaMapMemory(device_.allocator(), staging_buffer.allocation, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vmaUnmapMemory(device_.allocator(), staging_buffer.allocation);

    vertex_buffer_ = device_.create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    device_.copy_buffer(staging_buffer, vertex_buffer_, buffer_size);

    vmaDestroyBuffer(device_.allocator(), staging_buffer.buffer, staging_buffer.allocation);

}