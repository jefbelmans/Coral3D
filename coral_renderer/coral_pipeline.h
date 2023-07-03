#pragma once

#include <string>
#include <vector>

namespace coral_3d
{
	class coral_pipeline final
	{
	public:
		coral_pipeline(const std::string& vert_file_path, const std::string& frag_file_path);

	private:
		static std::vector<char> read_file(const std::string& file_path);
		
		void create_graphics_pipeline(const std::string& vert_file_path, const std::string& frag_file_path);
	};
}

