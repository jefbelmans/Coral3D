#include "atlas_generator.h"

#include "voxel_data.h"

const float atlas_generator::tile_size_{ 1.f / blocks_per_row_ };

void atlas_generator::calculate_uvs(Block& block)
{
	for (auto& face : block.faces)
	{
		for (auto& vertex : face.vertices)
		{
			const float u{ (static_cast<int>(face.type) % blocks_per_row_) * tile_size_ };
			const float v{ (static_cast<int>(face.type) / blocks_per_row_) * tile_size_ };

			vertex.tex_coord =
			{
				vertex.tex_coord.x * tile_size_ + u,
				vertex.tex_coord.y * tile_size_ + v
			};
		}
	}
}