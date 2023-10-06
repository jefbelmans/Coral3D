#include "coral_mesh.h"
#include "coral_pipeline.h"
#include "vk_initializers.h"

// TINY GLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/orthonormalize.hpp>

// STD
#include <iostream>
#include <cassert>
#include <unordered_map>

#include "coral_utils.h"

#include "coral_gameobject.h"

#define CALC_TANGENTS
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

    desc.bindings.emplace_back(main_binding);

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
    tangent_attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
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

bool coral_mesh::Builder::load_from_gltf(coral_device& device, const std::string &file_path)
{
    tinygltf::Model glTF_input;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    if(!loader.LoadASCIIFromFile(&glTF_input, &err, &warn, file_path))
    {
        std::cerr << "ERROR! coral_mesh::load_from_gltf() >> failed to load glTF: " << file_path << std::endl;
        if(!err.empty())
        {
            std::cerr << "ERROR! coral_mesh::load_from_gltf() >> " << err << std::endl;
            return false;
        }
    }

    if(!warn.empty())
    {
        std::cerr << "WARNING! coral_mesh::load_from_gltf() >> " << warn << std::endl;
    }

    load_images(device, glTF_input);
    load_materials(glTF_input);
    load_textures(glTF_input);
    const tinygltf::Scene& scene = glTF_input.scenes[0];
    for(int i : scene.nodes)
    {
        const tinygltf::Node node = glTF_input.nodes[i];
        load_node(device, node, glTF_input, nullptr);
    }

#ifdef CALC_TANGENTS
    // CALCULATE TANGENTS & BITANGENTS
    std::vector<glm::vec3> bitangents{vertices.size()};

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i + 1];
        uint32_t i2 = indices[i + 2];

        const glm::vec3& p0 = vertices[i0].position;
        const glm::vec3& p1 = vertices[i1].position;
        const glm::vec3& p2 = vertices[i2].position;
        const glm::vec2& uv0 = vertices[i0].uv;
        const glm::vec2& uv1 = vertices[i1].uv;
        const glm::vec2& uv2 = vertices[i2].uv;

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        float x1 = uv1.x - uv0.x;
        float x2 = uv2.x - uv0.x;
        float y1 = uv1.y - uv0.y;
        float y2 = uv2.y - uv0.y;

        float r = 1.f / (x1 * y2 - x2 * y1);
        glm::vec3 tangent = (edge1 * y2 - edge2 * y1) * r;
        glm::vec3 bitangent = (edge2 * x1 - edge1 * x2) * r;

        vertices[i0].tangent += glm::vec4(tangent, 0.f);
        vertices[i1].tangent += glm::vec4(tangent, 0.f);
        vertices[i2].tangent += glm::vec4(tangent, 0.f);

        bitangents[i0] += bitangent;
        bitangents[i1] += bitangent;
        bitangents[i2] += bitangent;
    }

    // ORTHONORMALIZE TANGENT AND CALCULATE HANDEDNESS
    for(size_t i = 0; i < vertices.size(); i++)
    {
        const glm::vec3& t = vertices[i].tangent;
        const glm::vec3& b = bitangents[i];
        const glm::vec3& n = vertices[i].normal;
        vertices[i].tangent = glm::vec4(glm::normalize(glm::orthonormalize(t, n)), 0.f);
        vertices[i].tangent.w = (glm::dot(glm::cross(t,b), n) > 0.f) ? 1.f : -1.f;
    }
#endif
    return true;
}

void coral_mesh::Builder::load_images(coral_device& device, tinygltf::Model &input)
{
    images.resize(input.images.size());
    for (size_t i = 0; i < input.images.size(); ++i)
    {
        tinygltf::Image& glTF_image = input.images[i];

        bool is_normal_map{false};
        for(size_t j = 0; j < input.materials.size(); ++j)
        {
            tinygltf::Material glTFMaterial = input.materials[j];
            if(glTFMaterial.additionalValues.find("normalTexture") == glTFMaterial.additionalValues.end()) continue;

            if((size_t)input.textures[glTFMaterial.additionalValues["normalTexture"].TextureIndex()].source == i)
            {
                is_normal_map = true;
                break;
            }
        }

        images[i].texture = coral_texture::create_texture_from_file(
                device,
                "assets/textures/" + glTF_image.uri,
                is_normal_map ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB);
    }
}

void coral_mesh::Builder::load_textures(tinygltf::Model &input)
{
    textures.resize(input.textures.size());
    for (size_t i = 0; i < input.textures.size(); i++)
    {
        textures[i].image_index = input.textures[i].source;
    }
}

void coral_mesh::Builder::load_materials(tinygltf::Model &input)
{
    materials.resize(input.materials.size());
    for (size_t i = 0; i < input.materials.size(); i++)
    {
        tinygltf::Material glTFMaterial = input.materials[i];
        // Get the base color factor
        if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end())
        {
            materials[i].base_color_factor = glm::make_vec4(
                    glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        }
        // Get base color texture index
        if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end())
        {
            materials[i].base_color_texture_index = glTFMaterial.values["baseColorTexture"].TextureIndex();
        }
        // Get the normal map texture index
        if (glTFMaterial.additionalValues.find("normalTexture") != glTFMaterial.additionalValues.end())
        {
            materials[i].normal_texture_index = glTFMaterial.additionalValues["normalTexture"].TextureIndex();
        }

        materials[i].alpha_mode = glTFMaterial.alphaMode;
        materials[i].alpha_cutoff = (float) glTFMaterial.alphaCutoff;
        materials[i].double_sided = glTFMaterial.doubleSided;
    }
}

void coral_mesh::Builder::load_node(coral_device& device, const tinygltf::Node &input_node, const tinygltf::Model &input, coral_3d::Node *parent)
{
    auto* node = new coral_3d::Node{};
    node->name = input_node.name;
    node->parent = parent;

    // NODE TRANSFORM
    node->transform = glm::mat4(1.f);
    if(input_node.translation.size() == 3)
        node->transform = glm::translate(node->transform, glm::vec3(glm::make_vec3(input_node.translation.data())));
    if(input_node.rotation.size() == 4)
    {
        glm::quat q = glm::make_quat(input_node.rotation.data());
        node->transform *= glm::mat4(q);
    }
    if(input_node.scale.size() == 3)
        node->transform = glm::scale(node->transform, glm::vec3(glm::make_vec3(input_node.scale.data())));
    if(input_node.matrix.size() == 16)
        node->transform = glm::make_mat4x4(input_node.matrix.data());

    // LOAD CHILDREN
    if(!input_node.children.empty())
    {
        for (int i : input_node.children)
            load_node(device, input.nodes[i], input, node);
    }

    // LOAD VERTICES AND INDICES
    if(input_node.mesh > -1)
    {
        const tinygltf::Mesh mesh = input.meshes[input_node.mesh];

        // ITERATE PRIMITIVES
        for (size_t i = 0; i < mesh.primitives.size(); ++i)
        {
            const tinygltf::Primitive& glTF_primitive = mesh.primitives[i];
            auto first_index = static_cast<uint32_t>(indices.size());
            auto vertex_start = static_cast<uint32_t>(vertices.size());
            uint32_t index_count = 0;

        #pragma region VERTICES
            {
                const float* position_buffer = nullptr;
                const float* normal_buffer = nullptr;
                const float* tangent_buffer = nullptr;
                const float* tex_coord_buffer = nullptr;
                size_t vertex_count = 0;

                // VERTEX POSITIONS
                if(glTF_primitive.attributes.find("POSITION") != glTF_primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = input.accessors[glTF_primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    position_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertex_count = accessor.count;
                }

                // VERTEX NORMALS
                if(glTF_primitive.attributes.find("NORMAL") != glTF_primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = input.accessors[glTF_primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    normal_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // VERTEX TANGENTS
                if(glTF_primitive.attributes.find("TANGENT") != glTF_primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = input.accessors[glTF_primitive.attributes.find("TANGENT")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    tangent_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                // VERTEX TEX COORDS
                if(glTF_primitive.attributes.find("TEXCOORD_0") != glTF_primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = input.accessors[glTF_primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    tex_coord_buffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                for (size_t v = 0; v < vertex_count; v++)
                {
                    Vertex vertex{};
                    vertex.position = glm::make_vec3(&position_buffer[v * 3]);
                    vertex.normal = glm::normalize(normal_buffer ? glm::make_vec3(&normal_buffer[v * 3]) : glm::vec3(0.f));
                    vertex.tangent = tangent_buffer ? glm::make_vec4(&tangent_buffer[v * 4]) : glm::vec4(0.f);
                    vertex.uv = tex_coord_buffer ? glm::make_vec2(&tex_coord_buffer[v * 2]) : glm::vec2(0.f);
                    vertices.emplace_back(vertex);
                }
            }
#pragma endregion

#pragma region INDICES
            {
                const tinygltf::Accessor& accessor = input.accessors[glTF_primitive.indices];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[view.buffer];

                index_count += static_cast<uint32_t>(accessor.count);

                switch(accessor.componentType)
                {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    {
                        const auto* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            indices.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    {
                        const auto* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            indices.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                    {
                        const auto* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                        for (size_t index = 0; index < accessor.count; index++)
                        {
                            indices.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    default:
                    {
                        std::cerr << "ERROR! coral_mesh::Builder::load_node() >> Index component type [" << accessor.componentType << "] not supported!" << std::endl;
                        return;
                    }
                }
                Primitive primitive{};
                primitive.first_index = first_index;
                primitive.index_count = index_count;
                primitive.material_index = glTF_primitive.material;
                node->mesh.primitives.emplace_back(primitive);
            }
#pragma endregion
        }
    }

    if(parent)
        parent->children.emplace_back(node);
    else
        nodes.emplace_back(node);
}

coral_mesh::coral_mesh(coral_device& device, Builder& builder, coral_gameobject* parent)
    : device_{device}
    , ptr_parent_{parent}
{
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
    images_ = std::move(builder.images);
    textures_ = std::move(builder.textures);
    materials_ = std::move(builder.materials);
    nodes_ = std::move(builder.nodes);
    std::cout << "INFO! coral_mesh::create_mesh_from_file() >> Finished loading mesh!" << std::endl;
}

coral_mesh::~coral_mesh()
{
    for(const Material& material : materials_)
        vkDestroyPipeline(device_.device(), material.pipeline, nullptr);
}

std::unique_ptr<coral_mesh> coral_mesh::create_mesh_from_file(coral_device& device, const std::string& file_path, coral_gameobject* parent)
{
    std::cout<< "INFO! coral_mesh::load_from_gltf() >> Loading mesh..." << std::endl;
    Builder builder{};
    builder.load_from_gltf(device, file_path);
    std::cout << "[" << file_path << "] vertex count: " << builder.vertices.size() << "\n";
    std::cout << "[" << file_path << "] index count: " << builder.indices.size() << "\n";

    return std::make_unique<coral_mesh>(device, builder, parent);
}

void coral_mesh::load_materials(coral_descriptor_set_layout& material_set_layout, coral_descriptor_pool& material_set_pool)
{
    for(auto& material : materials_)
    {
        auto color_desc = get_texture_descriptor(material.base_color_texture_index);
        auto normal_desc = get_texture_descriptor(material.normal_texture_index);

        coral_descriptor_writer(material_set_layout, material_set_pool)
                .write_image(0, &color_desc)
                .write_image(1, &normal_desc)
                .build(material.descriptor_set);
    }
}

void coral_mesh::draw(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout)
{
    // All vertices and indices are stored in single buffers, so we only need to bind once
    VkBuffer buffers[]{ vertex_buffer_->get_buffer().buffer };
    VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(command_buffer, index_buffer_->get_buffer().buffer, 0, VK_INDEX_TYPE_UINT32);

    // Render all nodes at top-level
    for (auto& node : nodes_)
        draw_node(command_buffer, pipeline_layout, node);
}

void coral_mesh::draw_node(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, coral_3d::Node *node)
{
    if (!node->visible) return;

    if (!node->mesh.primitives.empty())
    {
        // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
        glm::mat4 node_transform = node->transform;
        coral_3d::Node* current_parent = node->parent;
        while (current_parent)
        {
            node_transform = current_parent->transform * node_transform;
            current_parent = current_parent->parent;
        }

        PushConstant push{};
        push.node_transform = ptr_parent_->transform_.mat4() * node_transform;

        // Pass the final matrix to the vertex shader using push constants
        vkCmdPushConstants(
                command_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstant),
                &push);

        for (coral_3d::Primitive& primitive : node->mesh.primitives)
        {
            if (primitive.index_count > 0)
            {
                coral_3d::Material& material = materials_[primitive.material_index];

                // Bind the pipeline for the node's material
                vkCmdBindPipeline(
                        command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        material.pipeline);

                vkCmdBindDescriptorSets(
                        command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        pipeline_layout,
                        1, 1,
                        &material.descriptor_set,
                        0, nullptr);

                vkCmdDrawIndexed(
                        command_buffer,
                        primitive.index_count,
                        1,
                        primitive.first_index,
                        0, 0);
            }
        }
    }

    for (auto& child : node->children)
        draw_node(command_buffer, pipeline_layout, child);
}

void coral_mesh::create_vertex_buffers(const std::vector<Vertex>& vertices)
{
    uint32_t vertex_count = static_cast<uint32_t>(vertices.size());
    assert(vertex_count >= 3 && "ERROR! coral_mesh::create_vertex_buffers() >> Vertex count must be greater than or equal to 3");

    uint32_t vertex_size{ sizeof(vertices[0]) };
    VkDeviceSize buffer_size{ vertex_size * vertex_count};

    coral_buffer staging_buffer
    {
        device_,
		vertex_size,
        vertex_count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    };

    vertex_buffer_ = std::make_unique<coral_buffer>(
        device_,
        vertex_size,
        vertex_count,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)vertices.data());

    device_.copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);
}

void coral_mesh::create_index_buffers(const std::vector<uint32_t>& indices)
{
    uint32_t index_count = static_cast<uint32_t>(indices.size());

    if (index_count <= 0) return;

    uint32_t index_size{ sizeof(indices[0]) };
    VkDeviceSize buffer_size{ index_size * index_count };

    coral_buffer staging_buffer
    {
        device_,
		index_size,
        index_count,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    };

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)indices.data());

    index_buffer_ = std::make_unique<coral_buffer>(
		device_,
		index_size,
        index_count,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

    device_.copy_buffer(staging_buffer.get_buffer(), index_buffer_->get_buffer(), buffer_size);
}

VkDescriptorImageInfo coral_mesh::get_texture_descriptor(const size_t index)
{
    return images_[index].texture->get_descriptor_info();
}

void coral_mesh::create_pipelines(
        const std::string& vert_file_path,
        const std::string& frag_file_path,
        VkRenderPass render_pass,
        VkPipelineLayout pipeline_layout)
{
    assert(pipeline_layout != nullptr &&
           "ERROR! render_system::create_pipeline() >> Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo config_info{};
    coral_pipeline::default_pipeline_config_info(config_info);

    config_info.render_pass = render_pass;
    config_info.pipeline_layout = pipeline_layout;

    assert(config_info.pipeline_layout != VK_NULL_HANDLE &&
           "ERROR! coral_pipeline::create_graphics_pipeline() >> no pipeline layout provided in config_info ");

    assert(config_info.render_pass != VK_NULL_HANDLE &&
           "ERROR! coral_pipeline::create_graphics_pipeline() >> no render pass provided in config_info ");

    auto vert_code = coral_pipeline::read_file(vert_file_path);
    auto frag_code =  coral_pipeline::read_file(frag_file_path);

    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;
    coral_pipeline::create_shader_module(device_, vert_code, &vert_shader_module);
    coral_pipeline::create_shader_module(device_, frag_code, &frag_shader_module);

    VkPipelineShaderStageCreateInfo shader_stages[2];

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].pNext = nullptr;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_shader_module;
    shader_stages[0].pName = "main";
    shader_stages[0].flags = 0;
    shader_stages[0].pSpecializationInfo = nullptr;

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].pNext = nullptr;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_shader_module;
    shader_stages[1].pName = "main";
    shader_stages[1].flags = 0;
    shader_stages[1].pSpecializationInfo = nullptr;

    // VERTEX INPUT INFO
    VkPipelineVertexInputStateCreateInfo vertex_input_info{ vkinit::vertex_input_state_ci() };

    auto& binding_descriptions{config_info.binding_descriptions};
    auto& attribute_descriptions{config_info.attribute_descriptions};

    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
    pipeline_info.pViewportState = &config_info.viewport_info;
    pipeline_info.pRasterizationState = &config_info.rasterization_info;
    pipeline_info.pMultisampleState = &config_info.multisample_info;
    pipeline_info.pColorBlendState = &config_info.color_blend_info;
    pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
    pipeline_info.pDynamicState = &config_info.dynamic_state_info;

    pipeline_info.layout = config_info.pipeline_layout;
    pipeline_info.renderPass = config_info.render_pass;
    pipeline_info.subpass = config_info.subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    for(auto& material : materials_)
    {
        struct MaterialSpecializationData
        {
            VkBool32 use_alpha_mask;
            float alpha_cutoff;
        } material_specialization_data;

        std::vector<VkSpecializationMapEntry> specialization_map_entries
                {
                        vkinit::specialization_map_entry(0, offsetof(MaterialSpecializationData, use_alpha_mask), sizeof(MaterialSpecializationData::use_alpha_mask)),
                        vkinit::specialization_map_entry(1, offsetof(MaterialSpecializationData, alpha_cutoff), sizeof(MaterialSpecializationData::alpha_cutoff))
                };
        VkSpecializationInfo specialization_info{vkinit::specialization_info(specialization_map_entries, sizeof(material_specialization_data), &material_specialization_data)};
        shader_stages[1].pSpecializationInfo = &specialization_info;
        config_info.rasterization_info.cullMode = material.double_sided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

        material_specialization_data.use_alpha_mask = material.alpha_mode == "MASK";
        material_specialization_data.alpha_cutoff = material.alpha_cutoff;

        if (vkCreateGraphicsPipelines(device_.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &material.pipeline)
            != VK_SUCCESS)
            throw std::runtime_error("ERROR! coral_pipeline::create_graphics_pipeline() >> Failed to create graphics pipeline!");
    }
}
