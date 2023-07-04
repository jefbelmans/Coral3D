#include "coral_mesh.h"

// LIBS
#include <tiny_obj_loader.h>

// STD
#include <iostream>
#include <cassert>

using namespace coral_3d;

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
    //VkVertexInputAttributeDescription normal_attrib{};
    //normal_attrib.binding = 0;
    //normal_attrib.location = 1;
    //normal_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    //normal_attrib.offset = offsetof(Vertex, normal);

    // Color will be stored at Location 1
    VkVertexInputAttributeDescription color_attrib{};
    color_attrib.binding = 0;
    color_attrib.location = 1;
    color_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attrib.offset = offsetof(Vertex, color);

    //// UV will be stored at Location 3
    //VkVertexInputAttributeDescription texcoord_attrib{};
    //texcoord_attrib.binding = 0;
    //texcoord_attrib.location = 3;
    //texcoord_attrib.format = VK_FORMAT_R32G32_SFLOAT;
    //texcoord_attrib.offset = offsetof(Vertex, uv);

    desc.attributes.emplace_back(position_attrib);
    // desc.attributes.emplace_back(normal_attrib);
    desc.attributes.emplace_back(color_attrib);
    // desc.attributes.emplace_back(texcoord_attrib);

    return desc;
}

bool coral_mesh::load_from_obj(const char* filename)
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
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);

    //make sure to output the warnings to the console, in case there are issues with the file
    if (!warn.empty())
    {
        std::cout << "WARNING! Mesh::load_from_obj() >> " << warn << std::endl;
    }

    //if we have any error, print it to the console, and break the mesh loading.
    //This happens if the file can't be found or is malformed
    if (!err.empty())
    {
        std::cerr << err << std::endl;
        return false;
    }

    // Loop over shapes
    //for (size_t s = 0; s < shapes.size(); s++)
    //{

    //    // Loop over faces(polygon)
    //    size_t index_offset = 0;
    //    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
    //    {
    //        //hardcode loading to triangles
    //        int fv = 3;

    //        // Loop over vertices in the face.
    //        for (size_t v = 0; v < fv; v++)
    //        {
    //            // access to vertex
    //            tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

    //            //vertex position
    //            tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
    //            tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
    //            tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
    //            //vertex normal
    //            tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
    //            tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
    //            tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

    //            //copy it into our vertex
    //            Vertex new_vert;
    //            new_vert.position.x = vx;
    //            new_vert.position.y = vy;
    //            new_vert.position.z = vz;

    //            new_vert.normal.x = nx;
    //            new_vert.normal.y = ny;
    //            new_vert.normal.z = nz;

    //            //we are setting the vertex color as the vertex normal. This is just for display purposes
    //            new_vert.color = new_vert.normal;

    //            tinyobj::real_t ux{attrib.texcoords[2 * idx.texcoord_index + 0]};
    //            tinyobj::real_t uy{attrib.texcoords[2 * idx.texcoord_index + 1]};

    //            new_vert.uv = { ux, 1 - uy };

    //            vertices_.push_back(new_vert);
    //        }
    //        index_offset += fv;
    //    }
    //}

    return true;
}

coral_mesh::coral_mesh(coral_device& device, const std::vector<Vertex>& vertices)
    : device_{device}
    , vertices_{vertices}
{
    create_vertex_buffers();
}

coral_mesh::~coral_mesh()
{
    vmaDestroyBuffer(device_.allocator(), vertex_buffer_.buffer, vertex_buffer_.allocation);
}

void coral_mesh::bind(VkCommandBuffer command_buffer)
{
    VkBuffer buffers[]{ vertex_buffer_.buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

void coral_mesh::draw(VkCommandBuffer command_buffer)
{
    vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
}

void coral_mesh::create_vertex_buffers()
{
    vertex_count_ = static_cast<uint32_t>(vertices_.size());
    assert(vertex_count_ >= 3 && "ERROR! coral_mesh::create_vertex_buffers() >> Vertex count must be greater than or equal to 3");

    VkDeviceSize buffer_size{ sizeof(Vertex) * vertex_count_ };
    vertex_buffer_ = device_.create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;
    vmaMapMemory(device_.allocator(), vertex_buffer_.allocation, &data);
    memcpy(data, vertices_.data(), static_cast<size_t>(buffer_size));
    vmaUnmapMemory(device_.allocator(), vertex_buffer_.allocation);
}
