find_package(Vulkan REQUIRED)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/libs")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/libs")

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

# IMGUI
add_library(imgui STATIC)

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

target_sources(imgui PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_stdlib.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_stdlib.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"

		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_vulkan.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp"
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)