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
{
	start_worker_threads();
}

world_generator::~world_generator()
{
	threads_finished_ = true;
	for (size_t i = 0; i < thread_count_; i++)
		worker_threads_[i].join();
}

/*======================== World Generation ========================*/
void world_generator::generate_world()
{
	auto start = std::chrono::high_resolution_clock::now();
	
	// First generate all the chunks
	for (int x = -render_distance_; x <= render_distance_; x++)
		for (int z = -render_distance_; z <= render_distance_; z++)
			chunks_.emplace_back(generate_chunk({ x,z }));

	// Then build all the chunks
	for (auto& chunk : chunks_)
		build_chunk(chunk);

	for (auto chunk : chunks_to_build_)
		build_chunk_mesh(*chunk);

	chunks_to_build_.clear();

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "\nGenerated world with:\n\t"
		<< chunks_.size() << " chunks\n\t"
		<< "Took: " << duration << "ms\n\n";
}

void world_generator::update_world(const glm::vec3& position)
{
	// Get the chunk the player is in
	glm::ivec2 player_chunk_coord = get_coord(position);

	if(old_player_chunk_coord == player_chunk_coord) return;

	old_player_chunk_coord = player_chunk_coord;
	for (auto& chunk : chunks_)
	{
		chunk.is_active = false;
	}

	// Get the chunks around the player
	uint64_t num_chunks = static_cast<uint64_t>(chunks_.size());
	for (int x = -render_distance_; x <= render_distance_; x++)
	{
		for (int z = -render_distance_; z <= render_distance_; z++)
		{
			glm::ivec2 chunk_coord{ old_player_chunk_coord.x + x, old_player_chunk_coord.y + z };
			Chunk* chunk = get_chunk_at_coord(chunk_coord);

			if (chunk && chunk->is_initialized)
				chunk->is_active = true;
			else
			{
				chunks_.emplace_back(generate_chunk(chunk_coord));
				build_chunk(chunks_.back());
				build_chunk_mesh(chunks_.back());
			}
				
		}
	}
}

Chunk& world_generator::add_chunk(const glm::ivec2& chunk_coord)
{
	chunks_.emplace_back(generate_chunk(chunk_coord));
	return chunks_.back();
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

BlockType world_generator::generate_block(const glm::vec3& position)
{
	if (position.y == 0)
		return BlockType::GRASS_BLOCK;
	if (position.y == world_height_ - 1)
		return BlockType::BEDROCK;
	if (position.y < 4)
		return BlockType::DIRT;
	else
		return BlockType::STONE;
}

void world_generator::build_chunk(Chunk& chunk)
{
	for (int x = 0; x < chunk_size_; x++)
		for (int y = 0; y < world_height_; y++)
			for (int z = 0; z < chunk_size_; z++)
				add_block_vertices_to_chunk({ x,y,z }, chunk);

	chunks_to_build_.emplace_back(&chunk);
}

/// <summary>
/// Rebuilds the chunks north, east, south, and west of the given chunk.
/// </summary>
/// <param name="coord">Coord of the center chunk</param>
void world_generator::rebuild_surrounding_chunks(const Chunk& chunk)
{
	// Recalculate bordering faces of chunk north
	Chunk* north_chunk = get_chunk_at_coord({ chunk.coord.x, chunk.coord.y + 1 });
	if (north_chunk && north_chunk->is_active)
		build_chunk(*north_chunk);

	// Recalculate bordering faces of chunk east
	Chunk* east_chunk = get_chunk_at_coord({ chunk.coord.x + 1, chunk.coord.y });
	if (east_chunk && east_chunk->is_active)
		build_chunk(*east_chunk);

	// Recalculate bordering faces of chunk south
	Chunk* south_chunk = get_chunk_at_coord({ chunk.coord.x, chunk.coord.y - 1 });
	if (south_chunk && south_chunk->is_active)
		build_chunk(*south_chunk);

	// Recalculate bordering faces of chunk west
	Chunk* west_chunk = get_chunk_at_coord({ chunk.coord.x - 1, chunk.coord.y });
	if (west_chunk && west_chunk->is_active)
		build_chunk(*west_chunk);
}

/// <summary>
/// Calculates the visible faces of a block in the chunk and adds them to the chunk.
/// </summary>
/// <param name="position">Relative chunk position of the block</param>
/// <param name="chunk">Chunk the block is in</param>
/// <returns>Returns false if the block is not in the chunk, returns true otherwise</returns>
bool world_generator::add_block_vertices_to_chunk(const glm::vec3& position, Chunk& chunk)
{
	// If we are trying to add a block outside of the chunk, return false
	if(	position.x < 0 || position.x >= chunk_size_ ||
		position.y < 0 || position.y >= world_height_ ||
		position.z < 0 || position.z >= chunk_size_)
		return false;

	Block block{ voxel_data::get_block(chunk.block_map[position]) };

	int num_deleted_faces{ 0 };
	auto it = block.faces.begin();
	while (it != block.faces.end())
	{
		BlockType adjecent_block{};
		// Get block at world position
		if(position.x == chunk_size_ - 1 || position.x == 0 || position.z == chunk_size_ - 1 || position.z == 0) 
			adjecent_block = get_block_at_position((position + it->vertices[0].normal) + 
				glm::vec3{chunk.world_position.x, 0.f, chunk.world_position.y});
		else
			adjecent_block = chunk.block_map[position + it->vertices[0].normal];

		// Check if there is a block and if it is solid, if so, don't add this face
		if (voxel_data::is_block_solid(adjecent_block))
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
	chunk.vertices.clear();
	chunk.indices.clear();

	// After the chunk is built, we can set it active for rendering
	chunk.is_active = true;
	chunk.is_initialized = true;
}

/*======================== Chunk Getters ========================*/
#pragma region Chunk Getters
glm::ivec2 world_generator::get_coord(const glm::vec3& position)
{
	return
	{
		position.x < 0 ? std::floor(position.x / chunk_size_) : position.x / chunk_size_,
		position.z < 0 ? std::floor(position.z / chunk_size_) : position.z / chunk_size_
	};
}

Chunk* world_generator::get_chunk_at_coord(const glm::ivec2& coord)
{
	auto it = std::find_if(chunks_.begin(), chunks_.end(), [&](const Chunk& chunk) {
		return chunk.coord == coord;
		});

	return it != chunks_.end() ? &*it : nullptr;
}

Chunk* world_generator::get_chunk_at_position(const glm::vec3& position)
{
	glm::ivec2 coord { get_coord(position) };

	return get_chunk_at_coord(coord);
}

BlockType world_generator::get_block_at_position(const glm::vec3& position)
{
	Chunk* chunk{ get_chunk_at_position(position) };

	// If there is no chunk, return air
	if (!chunk) return BlockType::AIR;

	// Get chunk relative position
	glm::vec3 chunk_relative_position
	{
		position.x - chunk->world_position.x,
		position.y,
		position.z - chunk->world_position.y 
	};

	// Position lies outside of the world if chunk is null
	// or if the chunk does not contain the position
	if (!chunk->block_map.contains(chunk_relative_position)) return BlockType::AIR;
		
	return chunk->block_map[chunk_relative_position];
}
#pragma endregion

/* ======================================================= Jobs ======================================================= */
#pragma region Jobs
void world_generator::start_worker_threads()
{
	thread_count_ = 1;
	worker_threads_.reserve(thread_count_);
	command_pools_.reserve(thread_count_);

	for (size_t i = 0; i < thread_count_; i++)
	{
		command_pools_.emplace_back(device_.create_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
		command_buffers_.emplace_back(device_.create_command_buffer(command_pools_[i], VK_COMMAND_BUFFER_LEVEL_SECONDARY));

		worker_threads_.emplace_back(std::jthread(
			worker_thread(threads_finished_, work_queue_, i, command_buffers_.back())
		));
	}
}

GenerateChunk::GenerateChunk(coral_3d::coral_device& device, world_generator& generator, const glm::ivec2& chunk_coord)
	: device_{ device }, generator_{ generator }, chunk_coord_{ chunk_coord } {}

void GenerateChunk::execute(VkCommandBuffer command_buffer)
{
	auto& chunk = generator_.add_chunk(chunk_coord_);
	generator_.rebuild_surrounding_chunks(chunk);
	generator_.build_chunk(chunk);
}
#pragma endregion