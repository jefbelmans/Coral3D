#pragma once

// STD
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

#include "coral_mesh.h"
#include "voxel_data.h"
#include "atlas_generator.h"

struct ChunkPosition
{
	glm::vec2 position{ 0, 0 };
};

class coral_3d::coral_device;
class chunk final
{
public:
	chunk(coral_3d::coral_device& device, ChunkPosition position);
	~chunk() = default;

	void load(coral_3d::coral_device& device);

	coral_3d::coral_mesh* get_mesh() const { return mesh_.get(); }
	glm::vec2 get_world_position() const { return { position_.position.x * chunk_width_, position_.position.y * chunk_width_ }; }

	std::vector<coral_3d::Vertex> get_vertices() const { return vertices_; }
	std::vector<uint32_t> get_indices() const { return indices_; }

private:
	void add_block(glm::vec3 position);
	void build_mesh(coral_3d::coral_device& device);

	std::vector<Block> blocks_{};
	uint32_t num_faces_{};

	std::shared_ptr<coral_3d::coral_mesh> mesh_;
	std::unordered_map<glm::vec3, BlockType> voxel_map_;
	ChunkPosition position_{};

	std::vector<coral_3d::Vertex> vertices_{};
	std::vector<uint32_t> indices_{};

	const uint16_t chunk_width_{ 16 };
	const uint16_t chunk_height_{ 32 };
};