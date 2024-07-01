#include "homothety.h"

void Homothety::fromTo(glm::vec3 a3, glm::vec3 b3, glm::vec3 c3, glm::vec2 a2, glm::vec2 b2, glm::vec2 c2)
{
    float area3D = 0.5 * length(cross(b3 - a3, c3 - a3));
    float area2D = 0.5 * length(cross(glm::vec3(b2, 0.0) - glm::vec3(a2, 0.0), glm::vec3(c2, 0.0) - glm::vec3(a2, 0.0)));
    ratio = sqrt(area3D / area2D);
    center = glm::vec3((a2 + b2 + c2) / 3.0f, 0.0);
}

glm::vec3 Homothety::apply(glm::vec3 point) const
{
    float x = center.x + ratio * (point.x - center.x);
    float y = center.y + ratio * (point.y - center.y);
    float z = center.z + ratio * (point.z - center.z);
    return glm::vec3(x, y, z);
}

glm::vec3 Homothety::applyInverse() const
{
    return glm::vec3();
}
