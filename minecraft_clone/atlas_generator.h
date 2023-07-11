#pragma once

#include <cstdint>

struct Block;
class atlas_generator final
{
public:
	static void calculate_uvs(Block& block);

private:
	static const uint8_t blocks_per_row_{ 16 };
	static const float tile_size_;
};
