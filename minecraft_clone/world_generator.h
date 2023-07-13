#pragma once

#include <vector>
#include <condition_variable>
#include <thread>

#include "vk_job.h"
#include "chunk.h"



class coral_3d::coral_device;
class world_generator final
{
public:
	world_generator(coral_3d::coral_device& device);
	~world_generator();

	void generate_world();
	void update_world(const glm::vec3& position);

	Chunk& add_chunk(const glm::ivec2& chunk_coord);

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

	coral_3d::coral_device& device_;

	const uint16_t render_distance_{ 1 }; // In chunks, in each direction (render_distance * 2 + 1)
	const uint16_t chunk_size_{ 16 };
	const uint16_t world_height_{ 32 };

	std::vector<Chunk> chunks_{};
	glm::ivec2 old_player_chunk_coord{0};

	// Multithreading
	void start_worker_threads();

	size_t thread_count_{ 0 };
	std::mutex chunk_mutex_{};
	std::vector<VkCommandPool> command_pools_{}; // One for each thread
	std::vector<VkCommandBuffer> command_buffers_{}; // One for each thread
	std::vector<std::jthread> worker_threads_{};
	bool threads_finished_{ false };
	vk_work_queue work_queue_{};

	friend class GenerateChunk;
};

/* ================ Jobs ================ */
class GenerateChunk final : public vk_job
{
public:
	GenerateChunk(coral_3d::coral_device& device, world_generator& generator, const glm::ivec2& chunk_coord);
	virtual void execute(VkCommandBuffer command_buffer) override;

private:
	coral_3d::coral_device& device_;
	world_generator& generator_;
	glm::ivec2 chunk_coord_;
};