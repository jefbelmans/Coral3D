#pragma once

#include <cstdint>

struct Block;
class atlas_generator final
{
public:
	void calculate_uvs(Block& block);

private:
	const uint8_t blocks_per_row_{ 16 };
	const float tile_size_{ 1.f / blocks_per_row_ };
};
