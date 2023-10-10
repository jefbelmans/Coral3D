# Root
cmake_minimum_required(VERSION 3.22)

Project(Coral3D VERSION 0.0.1)

# Project settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vulkan
find_package(Vulkan REQUIRED)

# New renderer
add_subdirectory(coral_renderer)

# Third party libraries
add_subdirectory(third_party)

# Link libs
target_link_libraries(coral_renderer PRIVATE
		Vulkan::Vulkan
		glfw
		glm
		vkbootstrap
		tinyobjloader
		tinygltf
		vma
		stb_image
		imgui
)