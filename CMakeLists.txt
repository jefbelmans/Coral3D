# Root
cmake_minimum_required(VERSION 3.22)

Project(Coral3D VERSION 0.0.1)

# Project settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/W4 /WX)

# Vulkan
find_package(Vulkan REQUIRED)

# Executable
add_subdirectory(vk_renderer)

# Third party libraries
add_subdirectory(third_party)

# Link libs
target_link_libraries(vulkan_renderer PRIVATE
	Vulkan::Vulkan
	glfw
	glm
	vkbootstrap
	tinyobjloader
	vma
	stb_image
)