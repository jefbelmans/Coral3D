#pragma once

#include "coral_camera.h"
#include "coral_gameobject.h"

// LIBS
#include <vulkan/vulkan.h>

#define MAX_POINT_LIGHTS 8

namespace coral_3d
{
    struct PointLight
    {
        glm::vec4 position{1.f}; // w is radius
        glm::vec4 color{1.f}; // w is intensity
    };

    struct GlobalUBO
    {
        // MATRICES
        glm::mat4 view{1.f};
        glm::mat4 view_inverse{1.f};
        glm::mat4 view_projection{1.f};

        // GLOBAL LIGHT
        glm::vec4 global_light_direction{ glm::normalize(glm::vec4{ -0.477f, 0.477f, -0.477f, 0.f})}; // w is ignored
        glm::vec4 ambient_lighting{0.14f, 0.12f, 0.10f, 0.15f}; // w is intensity

        // POINT LIGHTS
        PointLight point_lights[MAX_POINT_LIGHTS];
        float num_lights{0.f};
    };

	struct FrameInfo
	{
		int frame_index{};
		float frame_tine{};
		VkCommandBuffer command_buffer{};
		coral_camera& camera;
		VkDescriptorSet global_descriptor_set;
		coral_gameobject::Map& gameobjects;
	};
}