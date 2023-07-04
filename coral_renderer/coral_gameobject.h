#pragma once

#include "coral_mesh.h"

// STD
#include <memory>

namespace coral_3d
{
	struct TransformComponent
	{
		glm::vec3 translation{0.0f, 0.0f, 0.0f};
		glm::vec3 scale{1.f, 1.f, 1.f};

		glm::mat3 mat3()
		{ 
			glm::mat3 scale_mat
			{
				{scale.x, 0.f, 0.f},
				{0.f, scale.y, 0.f},
				{0.f, 0.f, scale.z}
			};

			return scale_mat;
		}
	};

	class coral_gameobject final
	{
	public:
		using id_t = unsigned int;

		static coral_gameobject create_gameobject()
		{
			static id_t current_id = 0;
			return coral_gameobject{ current_id++ };
		}

		coral_gameobject(const coral_gameobject&) = delete;
		coral_gameobject& operator=(const coral_gameobject&) = delete;
		coral_gameobject(coral_gameobject&&) = default;
		coral_gameobject& operator=(coral_gameobject&&) = default;

		id_t get_id() const { return id_; }

		std::shared_ptr<coral_mesh> mesh_;
		glm::vec3 color_{};
		TransformComponent transform_{};

	private:
		coral_gameobject(id_t object_id) : id_{ object_id } {}
		id_t id_;
	};
}