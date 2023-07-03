#include "coral_pipeline.h"

#include <iostream>
#include <fstream>

using namespace coral_3d;

coral_pipeline::coral_pipeline(
	const std::string& vert_file_path, const std::string& frag_file_path)
{
	create_graphics_pipeline(vert_file_path, frag_file_path);
}

std::vector<char> coral_pipeline::read_file(const std::string& file_path)
{
	std::ifstream file{file_path, std::ios::ate | std::ios::binary };

	if (!file.is_open())
		throw std::runtime_error("ERROR! coral_pipeline::read_file() >> Failed to open file: " + file_path);

	// Create buffer sized to the file
	size_t file_size{ static_cast<size_t>(file.tellg()) };
	std::vector<char> buffer(file_size);

	// Read file into buffer
	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();
	return buffer;
}

void coral_pipeline::create_graphics_pipeline(
	const std::string& vert_file_path, const std::string& frag_file_path)
{
	auto vertShader = read_file(vert_file_path);
	auto fragShader = read_file(frag_file_path);

	std::cout << "Vertex shader code size: " << vertShader.size() << "\n";
	std::cout << "Frag shader code size: " << fragShader.size() << "\n";
}