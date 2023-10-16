#include "coral_gameobject.h"

using namespace coral_3d;

coral_gameobject coral_gameobject::create_point_light(float intensity, float radius, glm::vec3 color)
{
    coral_gameobject gameobject = coral_gameobject::create_gameobject
            ("point_light");
    gameobject.transform_.scale.x = radius;
    gameobject.point_light_ = std::make_unique<PointLightComponent>();
    gameobject.point_light_->color = glm::vec4(color, intensity);
    gameobject.name_ += std::to_string(gameobject.id_);
    return gameobject;
}
