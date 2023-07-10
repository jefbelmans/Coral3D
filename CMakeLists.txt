# Root
cmake_minimum_required(VERSION 3.22)

Project(Coral3D VERSION 0.0.1)

# Project settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/W4 /WX)

# Vulkan
find_package(Vulkan REQUIRED)

# New renderer
add_subdirectory(coral_renderer)

add_subdirectory(minecraft_clone)

# Third party libraries
add_subdirectory(third_party)

# Link libs
target_link_libraries(coral_renderer
	Vulkan::Vulkan
	glfw
	glm
	vkbootstrap
	tinyobjloader
	vma
	stb_image
)

target_link_libraries(minecraft_clone PUBLIC
	coral_renderer
)

target_include_directories(minecraft_clone PUBLIC
	"${RendererIncludeDir}"
)