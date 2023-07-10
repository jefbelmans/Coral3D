#pragma once

#include <vector>
#include <unordered_map>

#include "coral_mesh.h"

enum class BlockType : uint8_t
{
	AIR = 0,
	DIRT = 1,
	GRASS_BLOCK = 2,
	SAND = 4,
	WATER = 5,
	BEDROCK = 6,
	STONE = 7,
	COBBLESTONE = 8,
	SANDSTONE = 9,
	OAK_LOG = 12,
	OAK_LEAVES = 14
};

enum class FaceType : uint8_t
{
	AIR,
	DIRT,
	GRASS_TOP,
	GRASS_SIDE,
	SAND,
	WATER,
	BEDROCK,
	STONE,
	COBBLESTONE,
	SANDSTONE_SIDE,
	SANDSTONE_BOTTOM,
	SANDSTONE_TOP,
	OAK_LOG_SIDE,
	OAK_LOG_TOP,
	OAK_LEAVES
};

enum class FaceOrientation : uint8_t
{
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	BOTTOM,
	TOP
};

struct VoxelFace
{
	std::vector<coral_3d::Vertex> vertices;
	std::vector<uint32_t> indices;
	FaceType type{ FaceType::STONE };

	bool operator==(const VoxelFace& other) const
	{
		return vertices == other.vertices && indices == other.indices;
	}
};

struct Block
{
	BlockType type{ BlockType::STONE };
	std::vector<VoxelFace> faces;
	bool is_transparent;
};

class voxel_data final
{
public:
	static Block get_block(const BlockType& block_type);

private:
	static std::vector<coral_3d::Vertex> get_vertices();
	static std::vector<uint32_t> get_indices();
	static std::vector<VoxelFace> get_faces(const BlockType& block_type);

	static std::unordered_map<BlockType, std::unordered_map<FaceOrientation, FaceType>> block_face_map_;
};