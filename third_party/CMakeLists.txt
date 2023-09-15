find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(GLFW)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

# vkbootstrap
add_library(vkbootstrap STATIC)
target_sources(vkbootstrap PRIVATE
	vkbootstrap/VkBootstrap.h
	vkbootstrap/VkBootstrap.cpp
)

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

# VMA
add_library(vma INTERFACE)
target_include_directories(vma SYSTEM INTERFACE vma)

# tinyobjloader
add_library(tinyobjloader STATIC)
target_sources(tinyobjloader PRIVATE
	tinyobjloader/tiny_obj_loader.h
	tinyobjloader/tiny_obj_loader.cc
)

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

# tinygltf
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE tinygltf)

# STB
add_library(stb_image INTERFACE)
target_include_directories(stb_image SYSTEM INTERFACE stb_image)