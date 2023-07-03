add_executable(coral_renderer 
    "main.cpp"
    "coral_window.h"
    "coral_window.cpp"
    "first_app.h"
    "first_app.cpp"
    "coral_pipeline.h"
    "coral_pipeline.cpp"
)

add_dependencies(coral_renderer Shaders)

set_property(TARGET coral_renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:coral_renderer>")

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
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(coral_renderer Shaders)

# add shaders to output dir
add_custom_command(TARGET coral_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:coral_renderer>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shaders/compiled/"
        "$<TARGET_FILE_DIR:coral_renderer>/shaders"
        )