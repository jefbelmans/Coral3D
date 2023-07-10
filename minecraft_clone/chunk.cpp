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
				add_voxel({ x,y,z });

	std::cout << "Built chunk with:\n\t"
		<< faces_.size() << " faces\n\t"
		<< chunk_width_ * chunk_width_ * chunk_height_ << " voxels\n\t"
		<< faces_.size() * 2 << " triangles\n";

	build_mesh(device);
	position_ = glm::vec3{ 0.f };
}

void chunk::add_voxel(glm::vec3 position)
{
	auto faces = voxel_data::get_faces();

	int num_deleted_faces{ 0 };

	std::vector<voxel_data::VoxelFace>::iterator it = faces.begin();
	while (it != faces.end())
	{
		if (voxel_map_.contains(position + it->vertices[0].normal))
		{
			it = faces.erase(it);
			num_deleted_faces++;
			continue;
		}

		for (auto& vertex : it->vertices)
		{
			vertex.position += position;
		}

		for (auto& index : it->indices)
		{
			index += (faces_.size() * 4) - (num_deleted_faces * 4);
		}
		++it;
	}

	faces_.insert(faces_.end(), faces.begin(), faces.end());
}

void chunk::build_mesh(coral_device& device)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& face : faces_)
	{
		vertices.insert(vertices.end(), face.vertices.begin(), face.vertices.end());
		indices.insert(indices.end(), face.indices.begin(), face.indices.end());
	}

	mesh_ = coral_mesh::create_mesh_from_vertices(device, vertices, indices);
}