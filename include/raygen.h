#pragma once
#include <cmath>
#include "rayon.h"

std::vector<rayon> generateCameraRays(
    int fbW,
    int fbH,
    float fovDeg,
    const glm::vec3& camPos,
    const glm::mat4& view
)
{
    std::vector<rayon> rays;
    rays.reserve(fbW * fbH);

    float fovRad = glm::radians(fovDeg);
    float scale = tan(fovRad / 2.0f);
    float aspect = (float)fbW / (float)fbH;

    // extraire la rotation de la vue (mat3), puis l'inverser = transpose
    glm::mat3 R = glm::mat3(view);
    glm::mat3 invR = glm::transpose(R);

    for (int j = 0; j < fbH; ++j)
    {
        for (int i = 0; i < fbW; ++i)
        {
            // coords normalisées
            float x = (2.0f * (i + 0.5f) / fbW - 1.0f) * aspect * scale;
            float y = (1.0f - 2.0f * (j + 0.5f) / fbH) * scale;

            glm::vec3 rayDir_cam = glm::normalize(glm::vec3(x, y, -1.0f));

            // passer en espace monde
            glm::vec3 rayDir_world = glm::normalize(invR * rayDir_cam);

            rays.emplace_back(camPos, rayDir_world, glm::vec3(1, 1, 1));
        }
    }

    return rays;
}
