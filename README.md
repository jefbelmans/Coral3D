# Coral3D
### Vulkan Graphics Engine with Voxel Terrain Generator

This repository contains a basic Vulkan graphics engine implemented in C++ using CMake. It provides abstractions for common Vulkan logic, making it easier to develop graphics applications. Additionally, a branch has been created to develop a small voxel terrain generator using the engine from the main branch.
Everything is still very much work in progress and will continue being developed over the summer of 2023.

![A screenshot of the famous Sponza scene inside Coral3D](https://github.com/jefbelmans/Coral3D/blob/main/images/coral_scifi_helmet.png)

### Features:
- Abstractions for common Vulkan logic, including surface creation, command pools, synchronization structures, and command buffers.
- Support for validation layers in debug builds.
- Buffer and image creation helpers.
- Buffer management for efficient data handling and memory allocation.
- VMA (Vulkan Memory Allocator) integration for managing Vulkan memory.
- Swapchain support and surface capabilities querying.
- Queue family and device extension support checking.
- Basic point light render system
- Basic skybox render system

### Roadmap (priority high to low):
- Shadow mapping (directional, omni & spot)
- PBR support
- Deferred rendering
- Compute shaders

## Prerequisites
- C++17 or higher
- CMake (version 3.15 or later)
- Vulkan SDK

## Getting Started
To use this Vulkan graphics engine and voxel terrain generator, follow these steps:
- Clone the repository and navigate to the project directory.
- Install the required dependencies.
- Build the project using CMake.

## Engine Usage

The coral_device class provides the main interface for the Vulkan graphics engine. It handles device initialization, command buffer creation, buffer and image management, and more. Refer to the coral_device.h header for a quick look of the available methods.
The coral_buffer class provides buffer management for efficient handling of data. Refer to the coral_buffer.h header for a quick look of the available methods.
## Voxel Terrain Generator (WIP)
![Screenshot of a 9x9 chunk world generated on the fly](https://github.com/jefbelmans/Coral3D/blob/voxel-terrain-generator/images/Voxel_Terrain_Generator_Screenshot01.png)

The voxel terrain generator is being developed on a separate branch from the main engine. You can switch to the branch and explore the code to understand the implementation.
It is still very much a work in progress and is actively being developed.
### Roadmap:
- A definitive world generation algorithm using perlin noise (it only generates flat chunks as of now).
- Async chunk generation.
- Chunk manipulation (you cannot place or destroy blocks as of now).
- Multiple, configurable biomes for use in the world gen. algorithm.

## Contributing

Contributions to this project are welcome. If you find any issues or want to add new features, feel free to open a pull request.
## License

This project is licensed under the MIT License. Feel free to use and modify the code according to your needs.
## Acknowledgments

This Vulkan graphics engine is inspired by various online tutorials and resources such as [Vulkan Tutorial](https://vulkan-tutorial.com/), [Vulkan Guide](https://vkguide.dev/) and [this playlist](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) by Brendan Galea. Go check these resources out, as this engine wouldn't have existed without them!
