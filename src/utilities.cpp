#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/glm.hpp>

#include "utilities.h"
#include "rayon.h"
#include "cercle.h"

// ---------------------------------------------------------
//   PHYSIQUE 3D : update du rayon
// ---------------------------------------------------------

void update(float dt, rayon& r, const cercle& c)
{
    if (r.isAbsorbed()) return;

    glm::vec3 pos = r.pos();
    glm::vec3 dir = r.dir();

    // distance au trou noir
    glm::vec3 diff = pos - glm::vec3(c.cx, c.cy, c.cz);
    float R = glm::length(diff);

    // absorption ?
    if (R < c.rayon)
    {
        r.setAbsorbed(true);
        return;
    }

    // gravité newtonienne 3D
    float G = 1.0f;
    glm::vec3 accel = -G * c.masse * diff / (R * R * R);

    // intégration Euler
    dir += accel * dt;
    pos += dir * dt;

    // mise à jour
    r.dir() = dir;
    r.pos() = pos;

    r.addPoint(pos);
}
