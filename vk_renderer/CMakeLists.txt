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

set_property(TARGET vulkan_renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:vulkan_renderer>")

## Shader compilation

# GLSL
find_program(
	GLSL_VALIDATOR glslangValidator 
	HINTS 
	/usr/bin
	/usr/local/bin
	$ENV{VULKAN_SDK}/Bin/
	$ENV{VULKAN_SDK}/Bin32/
)

# Shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/shaders/*.comp"
    )

# iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${CMAKE_SOURCE_DIR}/shaders/compiled/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  # execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/shaders/compiled/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(vulkan_renderer Shaders)

# add shaders to output dir
add_custom_command(TARGET vulkan_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:vulkan_renderer>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders/compiled/"
        "$<TARGET_FILE_DIR:vulkan_renderer>/shaders"
)