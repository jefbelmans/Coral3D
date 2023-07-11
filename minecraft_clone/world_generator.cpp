#include "world_generator.h"

#include <iostream>

#include "coral_device.h"

world_generator::world_generator(coral_3d::coral_device& device)
	: device_{ device }
{}

void world_generator::generate_world()
{
	uint32_t vertex_count{};
	uint32_t index_count{};
	for (int x = 0; x < world_size_; x++)
	{
		for (int z = 0; z < world_size_; z++)
		{
			chunks_.emplace_back(device_, ChunkPosition{ glm::vec2{x,z} });
			vertex_count += chunks_.back().get_vertices().size();
			index_count += chunks_.back().get_indices().size();
		}
	}

	std::cout << "Generated world with:\n\t"
		<< chunks_.size() << " chunks\n\t"
		<< vertex_count << " vertices\n\t"
		<< index_count << " indices\n";

}
