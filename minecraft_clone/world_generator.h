#pragma once

#include <vector>

#include "chunk.h"

class coral_3d::coral_device;
class world_generator final
{
public:
	world_generator(coral_3d::coral_device& device);
	~world_generator() = default;

	void generate_world();
	void update_world(const glm::vec3& position);

	std::vector<Chunk>& get_chunks() { return chunks_; }
private:

	Chunk generate_chunk(const glm::ivec2& coord);
	Block generate_block(const glm::vec3& position);
	void build_chunk(const glm::ivec2& coord, Chunk& chunk);

	bool calculate_block_faces(const glm::vec3& position, Chunk& chunk);
	void build_chunk_mesh(Chunk& chunk);

	Chunk* get_chunk_at_coord(const glm::ivec2& coord);
	Chunk* get_chunk_at_position(const glm::vec3& position);

	Block* get_block_at_position(const glm::vec3& position);

	coral_3d::coral_device& device_;
	std::vector<Chunk> chunks_{};

	const uint32_t world_size_{ 16 };
	const uint16_t render_distance_{ 2w }; // In chunks, in each direction (render_distance * 2 + 1)
	const uint16_t chunk_size_{ 16 };
	const uint16_t world_height_{ 32 };
};
