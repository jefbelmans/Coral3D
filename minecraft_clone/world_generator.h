#pragma once

#include <vector>

#include "chunk.h"

class coral_3d::coral_device;
class world_generator final
{
public:
	world_generator(coral_3d::coral_device& device);
	~world_generator() = default;

	void generate_world();

	std::vector<chunk>& get_chunks() { return chunks_; }
private:
	coral_3d::coral_device& device_;
	std::vector<chunk> chunks_{};

	const uint32_t world_size_{ 2 };
};
