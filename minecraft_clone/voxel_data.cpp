#include "voxel_data.h"

std::unordered_map<BlockType, std::unordered_map<FaceOrientation, FaceType>> voxel_data::block_face_map_ =
{
	{
		BlockType::GRASS_BLOCK,
		{
			{FaceOrientation::FRONT, FaceType::GRASS_SIDE},
			{FaceOrientation::BACK, FaceType::GRASS_SIDE},
			{FaceOrientation::LEFT, FaceType::GRASS_SIDE},
			{FaceOrientation::RIGHT, FaceType::GRASS_SIDE},
			{FaceOrientation::TOP, FaceType::GRASS_TOP},
			{FaceOrientation::BOTTOM, FaceType::DIRT}
		}
	},
	{
		BlockType::OAK_LOG,
		{
			{FaceOrientation::FRONT, FaceType::OAK_LOG_SIDE},
			{FaceOrientation::BACK, FaceType::OAK_LOG_SIDE},
			{FaceOrientation::LEFT, FaceType::OAK_LOG_SIDE},
			{FaceOrientation::RIGHT, FaceType::OAK_LOG_SIDE},
			{FaceOrientation::TOP, FaceType::OAK_LOG_TOP},
			{FaceOrientation::BOTTOM, FaceType::OAK_LOG_TOP}
		}
	},
	{
		BlockType::SANDSTONE,
		{
			{FaceOrientation::FRONT, FaceType::SANDSTONE_SIDE},
			{FaceOrientation::BACK, FaceType::SANDSTONE_SIDE},
			{FaceOrientation::LEFT, FaceType::SANDSTONE_SIDE},
			{FaceOrientation::RIGHT, FaceType::SANDSTONE_SIDE},
			{FaceOrientation::TOP, FaceType::SANDSTONE_TOP},
			{FaceOrientation::BOTTOM, FaceType::SANDSTONE_BOTTOM}
		}
	}
};

std::vector<VoxelFace> voxel_data::block_faces_{};

Block voxel_data::get_block(const BlockType& block_type)
{
	Block block;
	block.type = block_type;
	block.is_solid = is_block_solid(block_type);
	
	block.faces = get_faces();

	// Calculate the face types for the block
	for (int i = 0; i < 6; i++)
	{
		// If the block type is not in the map, it has the same texture on all faces
		if (block_face_map_.find(block_type) == block_face_map_.end())
		{
			block.faces[i].type = static_cast<FaceType>(block_type);
			continue;
		}

		// If the block type is in the map, use the face types from the map
		block.faces[i].type = block_face_map_[block_type][static_cast<FaceOrientation>(i)];
	}

	return block;
}

bool voxel_data::is_block_solid(const BlockType& block_type)
{
	// TODO: Improve by using a lookup table
	return block_type != BlockType::AIR && block_type != BlockType::WATER;
}

std::vector<coral_3d::Vertex> voxel_data::get_vertices()
{
	return
	{
		// FRONT FACE
		{ {0.f, 0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f, -1.f} },
		{ {1.f, 0.f, 0.f}, {1.f, 0.f}, {0.f, 0.f, -1.f} },
		{ {1.f, 1.f, 0.f}, {1.f, 1.f}, {0.f, 0.f, -1.f} },
		{ {0.f, 1.f, 0.f}, {0.f, 1.f}, {0.f, 0.f, -1.f} },

		// BACK FACE
		{ {1.f, 0.f, 1.f}, {0.f, 0.f}, {0.f, 0.f, 1.f} },
		{ {0.f, 0.f, 1.f}, {1.f, 0.f}, {0.f, 0.f, 1.f} },
		{ {0.f, 1.f, 1.f}, {1.f, 1.f}, {0.f, 0.f, 1.f} },
		{ {1.f, 1.f, 1.f}, {0.f, 1.f}, {0.f, 0.f, 1.f} },

		// LEFT FACE
		{ {0.f, 0.f, 1.f}, {0.f, 0.f}, {-1.f, 0.f, 0.f} },
		{ {0.f, 0.f, 0.f}, {1.f, 0.f}, {-1.f, 0.f, 0.f} },
		{ {0.f, 1.f, 0.f}, {1.f, 1.f}, {-1.f, 0.f, 0.f} },
		{ {0.f, 1.f, 1.f}, {0.f, 1.f}, {-1.f, 0.f, 0.f} },

		// RIGHT FACE
		{ {1.f, 0.f, 0.f}, {0.f, 0.f}, {1.f, 0.f, 0.f} },
		{ {1.f, 0.f, 1.f}, {1.f, 0.f}, {1.f, 0.f, 0.f} },
		{ {1.f, 1.f, 1.f}, {1.f, 1.f}, {1.f, 0.f, 0.f} },
		{ {1.f, 1.f, 0.f}, {0.f, 1.f}, {1.f, 0.f, 0.f} },

		// TOP FACE
		{ {0.f, 1.f, 0.f}, {0.f, 0.f}, {0.f, 1.f, 0.f} },
		{ {1.f, 1.f, 0.f}, {1.f, 0.f}, {0.f, 1.f, 0.f} },
		{ {1.f, 1.f, 1.f}, {1.f, 1.f}, {0.f, 1.f, 0.f} },
		{ {0.f, 1.f, 1.f}, {0.f, 1.f}, {0.f, 1.f, 0.f} },

		// BOTTOM FACE
		{ {0.f, 0.f, 1.f}, {0.f, 0.f}, {0.f, -1.f, 0.f} },
		{ {1.f, 0.f, 1.f}, {1.f, 0.f}, {0.f, -1.f, 0.f} },
		{ {1.f, 0.f, 0.f}, {1.f, 1.f}, {0.f, -1.f, 0.f} },
		{ {0.f, 0.f, 0.f}, {0.f, 1.f}, {0.f, -1.f, 0.f} },
	};
}

std::vector<uint32_t> voxel_data::get_indices()
{
	return
	{
		{
			0, 1, 2, 2, 3, 0, // FRONT FACE
			4, 5, 6, 6, 7, 4, // BACK FACE
			8, 9, 10, 10, 11, 8, // LEFT FACE
			12, 13, 14, 14, 15, 12, // RIGHT FACE
			16, 17, 18, 18, 19, 16, // TOP FACE
			20, 21, 22, 22, 23, 20 // BOTTOM FACE
		}
	};
}

std::vector<VoxelFace> voxel_data::get_faces()
{
	if (!block_faces_.empty()) return block_faces_;

	auto verts = get_vertices();
	auto indices = get_indices();

	// Copy the vertices and indices for each face
	for (int i = 0; i < 6; i++)
	{
		VoxelFace block_face;
		block_face.vertices =
			std::vector<coral_3d::Vertex>(verts.begin() + (i * 4), verts.begin() + (i * 4) + 4);

		block_face.indices =
			std::vector<uint32_t>(indices.begin() + (i * 6), indices.begin() + (i * 6) + 6);

		block_face.type = FaceType::AIR;

		block_faces_.emplace_back(block_face);
	}

	return block_faces_;
}
