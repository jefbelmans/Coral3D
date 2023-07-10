#include "chunk.h"

// STD
#include <algorithm>
#include <iostream>

#include "coral_device.h"

using namespace coral_3d;

chunk::chunk(coral_3d::coral_device& device)
{
	load(device);
}

void chunk::load(coral_device& device)
{
	for (int y = 0; y < chunk_height_; y++)
		for (int x = 0; x < chunk_width_; x++)
			for (int z = 0; z < chunk_width_; z++)
				voxel_map_[{ x,y,z }] = true;

	for (int y = 0; y < chunk_height_; y++)
		for (int x = 0; x < chunk_width_; x++)
			for (int z = 0; z < chunk_width_; z++)
				add_block({ x,y,z });

	build_mesh(device);

	std::cout << "Built chunk with:\n\t"
		<< num_faces_ << " faces\n\t"
		<< num_faces_ * 2 << " triangles\n\t"
		<< blocks_.size() << " blocks\n";

	position_ = glm::vec3{ 0.f };
}

void chunk::add_block(glm::vec3 position)
{
	auto block = voxel_data::get_block();
	int num_deleted_faces{ 0 };

	auto it = block.faces.begin();
	while (it != block.faces.end())
	{
		// Check if there is a voxel adjecent to this face
		if (voxel_map_.contains(position + it->vertices[0].normal))
		{
			// If there is, delete this face
			it = block.faces.erase(it);
			num_deleted_faces++;
			continue;
		}

		// Else, add this face to the chunk
		for (auto& vertex : it->vertices)
		{
			vertex.position += position;
		}

		for (auto& index : it->indices)
		{
			index += (num_faces_ * 4) - (num_deleted_faces * 4);
		}
		++it;
	}

	atlas_generator_.calculate_uvs(block);

	num_faces_ += block.faces.size();
	blocks_.emplace_back(block);
}

void chunk::build_mesh(coral_device& device)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& block : blocks_)
	{
		for (const auto& face : block.faces)
		{
			vertices.insert(vertices.end(), face.vertices.begin(), face.vertices.end());
			indices.insert(indices.end(), face.indices.begin(), face.indices.end());
		}
	}

	mesh_ = coral_mesh::create_mesh_from_vertices(device, vertices, indices);
}