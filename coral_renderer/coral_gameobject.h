#pragma once

#include "coral_mesh.h"

// LIBS
#include<gtc/matrix_transform.hpp>

// STD
#include <memory>
#include <unordered_map>

namespace coral_3d
{
	struct TransformComponent
	{
		glm::vec3 translation{0.f};
		glm::vec3 scale{1.f};
		glm::vec3 rotation{0.f};

		// Matrix corresponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4()
		{
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4
			{
				{
					scale.x* (c1* c3 + s1 * s2 * s3),
					scale.x* (c2* s3),
					scale.x* (c1* s2* s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{ translation.x, translation.y, translation.z, 1.0f }
			};
		}

		glm::mat4 normal_matrix()
		{
			return glm::transpose(glm::inverse(mat4()));
		}
	};

    struct PointLightComponent
    {
        glm::vec4 color{1.f}; // W is intensity
    };

	class coral_gameobject final
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, std::shared_ptr<coral_gameobject>>;

        // Creators
		static coral_gameobject create_gameobject()
		{
			static id_t current_id = 0;
			return coral_gameobject{ current_id++ };
		}

        static coral_gameobject create_point_light(float intensity = 1.f, float radius = .1f, glm::vec3 color = glm::vec3{1.f});

		coral_gameobject(const coral_gameobject&) = delete;
		coral_gameobject& operator=(const coral_gameobject&) = delete;
		coral_gameobject(coral_gameobject&&) = default;
		coral_gameobject& operator=(coral_gameobject&&) = default;

		id_t get_id() const { return id_; }

		TransformComponent transform_{};

        // Optional components
        std::shared_ptr<coral_mesh> mesh_;
        std::unique_ptr<PointLightComponent> point_light_ = nullptr;

	private:
		explicit coral_gameobject(id_t object_id) : id_{ object_id } {}
		id_t id_;
	};
}