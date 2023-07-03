add_executable(vulkan_renderer
	main.cpp
	vk_engine.h
	vk_engine.cpp
	vk_types.h
	vk_initializers.h
	vk_initializers.cpp
	vk_pipeline.h
	vk_pipeline.cpp
	vk_mesh.h
	vk_mesh.cpp
	vk_texture.h 
	vk_texture.cpp
)

add_dependencies(vulkan_renderer Shaders)