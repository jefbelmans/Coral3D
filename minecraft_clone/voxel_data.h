#pragma once

#include <vector>
#include "coral_mesh.h"

enum class BlockType : uint8_t
{
	AIR,
	DIRT,
	GRASS_BLOCK,
	SAND,
	WATER,
	BEDROCK,
	STONE,
	COBBLESTONE,
	SANDSTONE,
	OAK_LOG,
	OAK_LEAVES,
	GRASS,
	WOOL
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
	static Block get_block();

private:
	static std::vector<coral_3d::Vertex> get_vertices();
	static std::vector<uint32_t> get_indices();
	static std::vector<VoxelFace> get_faces(FaceType face_type);
};