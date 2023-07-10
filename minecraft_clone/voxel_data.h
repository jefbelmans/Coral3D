#pragma once

#include <vector>
#include "coral_mesh.h"

class voxel_data final
{
public:
	struct VoxelFace
	{
		std::vector<coral_3d::Vertex> vertices;
		std::vector<uint32_t> indices;

		bool operator==(const VoxelFace& other) const
		{
			return vertices == other.vertices && indices == other.indices;
		}
	};

	static std::vector<coral_3d::Vertex> get_vertices()
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

	static std::vector<uint32_t> get_indices()
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

	static std::vector<VoxelFace> get_faces()
	{
		auto verts = get_vertices();
		auto indices = get_indices();

		std::vector<VoxelFace> faces{6};
		for (int i = 0; i < 6; i++)
		{
			faces[i].vertices =
				std::vector<coral_3d::Vertex>(verts.begin() + (i * 4), verts.begin() + (i * 4) + 4);

			faces[i].indices =
				std::vector<uint32_t>(indices.begin() + (i * 6), indices.begin() + (i * 6) + 6);
		}

		return faces;
	}
};
