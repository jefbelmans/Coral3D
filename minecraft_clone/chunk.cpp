#include "chunk.h"

// STD
#include <algorithm>
#include <iostream>

#include "coral_device.h"

using namespace coral_3d;

chunk::chunk(coral_3d::coral_device& device, ChunkPosition position)
	: position_{ position }
{
	load(device);
}

void chunk::load(coral_device& device)
{
	for (int y = 0; y < chunk_height_; y++)
		for (int x = 0; x < chunk_width_; x++)
			for (int z = 0; z < chunk_width_; z++)
				voxel_map_[{ x,y,z }] = BlockType::STONE;

	for (int y = 0; y < chunk_height_; y++)
		for (int x = 0; x < chunk_width_; x++)
			for (int z = 0; z < chunk_width_; z++)
				add_block({ x,y,z });

	build_mesh(device);

	//std::cout << "Built chunk with:\n\t"
	//	<< num_faces_ << " faces\n\t"
	//	<< num_faces_ * 2 << " triangles\n\t"
	//	<< blocks_.size() << " blocks\n";
}

void chunk::add_block(glm::vec3 position)
{
	Block block{};

	if (position.y == 0)
		block = voxel_data::get_block(BlockType::GRASS_BLOCK);
	else if (position.y < 4)
		block = voxel_data::get_block(BlockType::DIRT);
	else
		block = voxel_data::get_block(BlockType::STONE);

	int num_deleted_faces{ 0 };
	auto it = block.faces.begin();
	while (it != block.faces.end())
	{
		// Check if there is a voxel adjecent to this face
		if (voxel_map_.contains(position + it->vertices[0].normal) && voxel_map_[position + it->vertices[0].normal] != BlockType::AIR)
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
			index += vertices_.size() - (num_deleted_faces * 4);
		}
		++it;
	}

	atlas_generator::calculate_uvs(block);
	for (auto& face : block.faces)
	{
		vertices_.insert(vertices_.end(), face.vertices.begin(), face.vertices.end());
		indices_.insert(indices_.end(), face.indices.begin(), face.indices.end());
	}
}

void chunk::build_mesh(coral_device& device)
{
	// mesh_ = coral_mesh::create_mesh_from_file(device, "assets/meshes/room.obj");
	mesh_ = coral_mesh::create_mesh_from_vertices(device, vertices_, indices_);
}