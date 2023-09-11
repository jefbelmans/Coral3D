add_executable(coral_renderer
    "main.cpp"
    "coral_window.h"
    "coral_window.cpp"
    "first_app.h"
    "first_app.cpp"
    "coral_pipeline.h"
    "coral_pipeline.cpp"
    "coral_device.h"
    "coral_device.cpp"
    "vk_initializers.h"
    "vk_initializers.cpp"
    "vk_types.h"
    "coral_swapchain.h"
    "coral_swapchain.cpp"
    "coral_mesh.h"
    "coral_mesh.cpp"
    "coral_gameobject.h"
    "coral_gameobject.cpp"
    "coral_renderer.h"
    "coral_renderer.cpp"
    "render_system.h"
    "render_system.cpp"
    "coral_camera.h"
    "coral_camera.cpp"
    "coral_utils.h"
    "coral_utils.cpp"
    "coral_texture.h"
    "coral_texture.cpp"
    "coral_buffer.h"
    "coral_buffer.cpp"
    "coral_frame_info.h"
    "coral_frame_info.cpp"
    "coral_descriptors.h"
    "coral_descriptors.cpp"
    "coral_material.h"
    "coral_material.cpp"
)

if(MSVC)
    target_compile_options(coral_renderer PRIVATE /W4 /WX)
else()
    target_compile_options(coral_renderer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

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
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

# iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/compiled/${FILE_NAME}.spv")
  message(STATUS ${GLSL})

  # execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
	COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/shaders/compiled/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL}
  )
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(coral_renderer Shaders)

set(source_dir "${CMAKE_SOURCE_DIR}/shaders/compiled")
set(target_dir "${CMAKE_BINARY_DIR}/assets/shaders")
file(COPY ${source_dir} DESTINATION ${target_dir})

# Define a custom target to copy files
add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/assets/"

        # Shaders
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/assets/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shaders/compiled/"
        "$<TARGET_FILE_DIR:coral_renderer>/assets/shaders"

        # Meshes
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/assets/meshes/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets/meshes"
        "$<TARGET_FILE_DIR:coral_renderer>/assets/meshes"

        # Materials
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/assets/materials/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets/materials"
        "$<TARGET_FILE_DIR:coral_renderer>/assets/materials"

        # Textures
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/assets/textures/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets/textures"
        "$<TARGET_FILE_DIR:coral_renderer>/assets/textures"
)