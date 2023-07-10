#pragma once

// STD
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

#include "coral_mesh.h"
#include "voxel_data.h"
#include "atlas_generator.h"

class coral_3d::coral_device;
class chunk final
{
public:
	chunk(coral_3d::coral_device& device);
	~chunk() = default;

	void load(coral_3d::coral_device& device);

	coral_3d::coral_mesh* get_mesh() const { return mesh_.get(); }
	glm::vec3 get_position() const { return position_; }

private:
	void add_block(glm::vec3 position);
	void build_mesh(coral_3d::coral_device& device);

	atlas_generator atlas_generator_{};
	std::vector<Block> blocks_{};
	uint32_t num_faces_{};

	std::unordered_map<glm::vec3, bool> voxel_map_;

	std::shared_ptr<coral_3d::coral_mesh> mesh_;
	glm::vec3 position_{0.f};

	const uint16_t chunk_width_{ 16 };
	const uint16_t chunk_height_{ 64 };
};
