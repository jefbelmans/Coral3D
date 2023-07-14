# Coral3D
### Vulkan Graphics Engine with Voxel Terrain Generator

This repository contains a basic Vulkan graphics engine implemented in C++ using CMake. It provides abstractions for common Vulkan logic, making it easier to develop graphics applications. Additionally, a branch has been created to develop a small voxel terrain generator using the engine from the main branch.
Features
- Abstractions for common Vulkan logic, including surface creation, command pools, synchronization structures, and command buffers.
- Support for validation layers in debug builds.
- Buffer and image creation helpers.
- Buffer management for efficient data handling and memory allocation.
- VMA (Vulkan Memory Allocator) integration for managing Vulkan memory.
- Swapchain support and surface capabilities querying.
- Queue family and device extension support checking.

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

The coral_device class provides the main interface for the Vulkan graphics engine. It handles device initialization, command buffer creation, buffer and image management, and more. Refer to the coral_device.h header for detailed documentation of the available methods.
Voxel Terrain Generator

The voxel terrain generator is being developed on a separate branch from the main engine. You can switch to the branch and explore the code to understand the implementation. The coral_buffer class provides buffer management for efficient handling of voxel data. Refer to the coral_buffer.h header for detailed documentation of the available methods.
## Contributing

Contributions to this project are welcome. If you find any issues or want to add new features, feel free to open a pull request.
## License

This project is licensed under the MIT License. Feel free to use and modify the code according to your needs.
## Acknowledgments

This Vulkan graphics engine is inspired by various online tutorials and resources such as [Vulkan Tutorial](https://vulkan-tutorial.com/), [Vulkan Guide](https://vkguide.dev/) and [this playlist](https://www.youtube.com/playlist?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR) by Brendan Galea. Go check these resources out, as this engine wouldn't have existed without them!
