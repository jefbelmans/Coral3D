#include "world_generator.h"

// STD
#include <iostream>
#include <algorithm>
#include <chrono>

// Coral
#include "coral_device.h"
#include "atlas_generator.h"

world_generator::world_generator(coral_3d::coral_device& device)
	: device_{ device }
{}

/*======================== World Generation ========================*/
void world_generator::generate_world()
{
	auto start = std::chrono::high_resolution_clock::now();
	
	// First generate all the chunks
	for (int x = -render_distance_; x <= render_distance_; x++)
	{
		for (int z = -render_distance_; z <= render_distance_; z++)
		{
			chunks_.emplace_back(generate_chunk({ x,z }));
		}
	}

	// Then build all the chunks
	uint32_t vertex_count{};
	uint32_t index_count{};
	for (auto& chunk : chunks_)
	{
		build_chunk(chunk.coord, chunk);
		vertex_count += chunk.vertices.size();
		index_count += chunk.indices.size();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "\nGenerated world with:\n\t"
		<< chunks_.size() << " chunks\n\t"
		<< vertex_count << " vertices\n\t"
		<< index_count << " indices\n\t"
		<< index_count / 3 << " triangles\n"
		<< "Took: " << duration << "ms\n\n";
}

void world_generator::update_world(const glm::vec3& position)
{
	// Get the chunk the player is in
	Chunk* player_chunk = get_chunk_at_position(position);

	if (!player_chunk)
		return;

	// Get the chunks around the player
	for (auto& chunk : chunks_)
	{
		chunk.is_active = false;
	}

	uint64_t num_chunks = static_cast<uint64_t>(chunks_.size());
	for (int x = -render_distance_; x <= render_distance_; x++)
	{
		for (int z = -render_distance_; z <= render_distance_; z++)
		{
			glm::ivec2 chunk_coord{ player_chunk->coord.x + x, player_chunk->coord.y + z };
			Chunk* chunk = get_chunk_at_coord(chunk_coord);
			if (chunk)
				chunk->is_active = true;
			else
				chunks_.emplace_back(generate_chunk(chunk_coord));
		}
	}
	// Build the chunks around the player
	for (int i = num_chunks; i < chunks_.size(); i++)
	{
		std::cout << "Building new chunk: " << chunks_[i].coord.x << ", " << chunks_[i].coord.y << "\n";
		build_chunk(chunks_[i].coord, chunks_[i]);
	}
}

Chunk world_generator::generate_chunk(const glm::ivec2& coord)
{
	Chunk chunk{};
	chunk.coord = coord;
	chunk.world_position = { coord.x * chunk_size_, coord.y * chunk_size_ };

	for (int x = 0; x < chunk_size_; x++)
		for (int y = 0; y < world_height_; y++)
			for (int z = 0; z < chunk_size_; z++)
				chunk.block_map[{ x,y,z }] = generate_block({ x,y,z });

	return chunk;
}

Block world_generator::generate_block(const glm::vec3& position)
{
	if (position.y == 0)
		return voxel_data::get_block(BlockType::GRASS_BLOCK);
	if (position.y == world_height_ - 1)
		return voxel_data::get_block(BlockType::BEDROCK);
	if (position.y < 4)
		return voxel_data::get_block(BlockType::DIRT);
	else
		return voxel_data::get_block(BlockType::STONE);
}

void world_generator::build_chunk(const glm::ivec2& coord, Chunk& chunk)
{
	for (int x = 0; x < chunk_size_; x++)
		for (int y = 0; y < world_height_; y++)
			for (int z = 0; z < chunk_size_; z++)
				calculate_block_faces({ x,y,z }, chunk);

	build_chunk_mesh(chunk);

	// After the chunk is built, we can set it active for rendering
	chunk.is_active = true;
}

/// <summary>
/// Calculates the faces of a block in the chunk and adds them to the chunk.
/// </summary>
/// <param name="position">Relative chunk position of the block</param>
/// <param name="chunk">Chunk the block is in</param>
/// <returns>Returns false if the block is not in the chunk, returns true otherwise</returns>
bool world_generator::calculate_block_faces(const glm::vec3& position, Chunk& chunk)
{
	// If we are trying to add a block outside of the chunk, return false
	if(	position.x < 0 || position.x >= chunk_size_ ||
		position.y < 0 || position.y >= world_height_ ||
		position.z < 0 || position.z >= chunk_size_)
		return false;

	Block& block{ chunk.block_map[position] };

	int num_deleted_faces{ 0 };
	auto it = block.faces.begin();
	while (it != block.faces.end())
	{
		// Get block at world position
		Block* adjecent_block = get_block_at_position((position + it->vertices[0].normal) + 
			glm::vec3{chunk.world_position.x, 0.f, chunk.world_position.y});

		// Check if there is a block and if it is solid, if so, don't add this face
		if (adjecent_block && !adjecent_block->is_transparent)
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
			index += chunk.vertices.size() - (num_deleted_faces * 4);
		}
		++it;
	}

	// Calculate the UVs for each face
	atlas_generator::calculate_uvs(block);

	for (auto& face : block.faces)
	{
		chunk.vertices.insert(chunk.vertices.end(), face.vertices.begin(), face.vertices.end());
		chunk.indices.insert(chunk.indices.end(), face.indices.begin(), face.indices.end());
	}

	return true;
}

void world_generator::build_chunk_mesh(Chunk& chunk)
{
	chunk.mesh = coral_3d::coral_mesh::create_mesh_from_vertices(device_, chunk.vertices, chunk.indices);
}

/*======================== Chunk Getters ========================*/
Chunk* world_generator::get_chunk_at_coord(const glm::ivec2& coord)
{
	auto it = std::find_if(chunks_.begin(), chunks_.end(), [&](const Chunk& chunk) {
		return chunk.coord == coord;
		});

	return it != chunks_.end() ? &*it : nullptr;
}

Chunk* world_generator::get_chunk_at_position(const glm::vec3& position)
{
	glm::ivec2 coord
	{
		position.x < 0 ? std::floor(position.x / chunk_size_) : position.x / chunk_size_,
		position.z < 0 ? std::floor(position.z / chunk_size_) : position.z / chunk_size_
	};

	return get_chunk_at_coord(coord);
}

Block* world_generator::get_block_at_position(const glm::vec3& position)
{
	Chunk* chunk{ get_chunk_at_position(position) };

	if (!chunk) return nullptr;
	// Get chunk relative position
	glm::vec3 chunk_relative_position
	{
		position.x - chunk->world_position.x,
		position.y,
		position.z - chunk->world_position.y 
	};

	// Position lies outside of the world if chunk is null
	// or if the chunk does not contain the position
	if (!chunk->block_map.contains(chunk_relative_position)) return nullptr;
		
	return &chunk->block_map[position];
}