#pragma once

// STD
#include <unordered_map>

// Libs
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>

// Coral
#include "coral_mesh.h"
#include "voxel_data.h"

struct Chunk
{
	glm::ivec2 coord{};
	glm::vec2 world_position{};

	bool is_active{ false };

	std::shared_ptr<coral_3d::coral_mesh> mesh{};
	std::vector<coral_3d::Vertex> vertices{};
	std::vector<uint32_t> indices{};

	std::unordered_map<glm::vec3, Block> block_map{};
};