#pragma once

#include <vector>
#include <condition_variable>
#include <thread>

#include "chunk.h"


class coral_3d::coral_device;
class world_generator final
{
public:
	world_generator(coral_3d::coral_device& device);
	~world_generator();

	void generate_world();
	void update_world(const glm::vec3& position);

	std::vector<Chunk>& get_chunks() { return chunks_; }
private:

	Chunk generate_chunk(const glm::ivec2& coord);
	BlockType generate_block(const glm::vec3& position);

	void build_chunk(Chunk& chunk);
	void rebuild_surrounding_chunks(const Chunk& chunk);

	bool add_block_vertices_to_chunk(const glm::vec3& position, Chunk& chunk);
	void build_chunk_mesh(Chunk& chunk);

	glm::ivec2 get_coord(const glm::vec3& position);
	Chunk* get_chunk_at_coord(const glm::ivec2& coord);
	Chunk* get_chunk_at_position(const glm::vec3& position);

	BlockType get_block_at_position(const glm::vec3& position);

	void update_thread();

	coral_3d::coral_device& device_;

	const uint16_t render_distance_{ 5 }; // In chunks, in each direction (render_distance * 2 + 1)
	const uint16_t chunk_size_{ 16 };
	const uint16_t world_height_{ 32 };

	std::vector<Chunk> chunks_{};
	std::vector<glm::ivec2> chunks_to_generate_{};

	glm::ivec2 old_player_chunk_coord{0};

	std::condition_variable cv_{}; // Condition variable for the update thread
	std::mutex cv_mutex_{}; // Mutex for the condition variable
	std::jthread update_thread_{};
	bool update_thread_running_{ true };
};
