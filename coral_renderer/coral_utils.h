#pragma once
#include <functional>

namespace coral_3d
{
	namespace utils
	{
		template<typename T, typename... args>
		void hash_combine(std::size_t& seed, const T& v, const args&... rest)
		{
			seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			(hash_combine(seed, rest), ...);
		}
	}
}
