add_executable(minecraft_clone
    "main.cpp"
    "first_app.h"
    "first_app.cpp"
    "voxel_data.h"
    "voxel_data.cpp"
    "chunk.h"
    "chunk.cpp"
    "voxel_renderer.h"
    "voxel_renderer.cpp"
 "atlas_generator.h" "atlas_generator.cpp" "world_generator.h" "world_generator.cpp")

set_property(TARGET coral_renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:minecraft_clone>")

# add assets to output dir
add_custom_command(TARGET minecraft_clone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:minecraft_clone>/assets/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:minecraft_clone>/assets/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/shaders/compiled/"
        "$<TARGET_FILE_DIR:minecraft_clone>/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:minecraft_clone>/assets/meshes/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets/meshes"
        "$<TARGET_FILE_DIR:minecraft_clone>/assets/meshes"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:minecraft_clone>/assets/textures/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets/textures"
        "$<TARGET_FILE_DIR:minecraft_clone>/assets/textures"
)