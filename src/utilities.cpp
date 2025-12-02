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

    // ------------------- Constantes physiques (unités réduites) -------------------
    // On travaille en unités où c = 1 pour limiter les facteurs d'échelle. Le
    // rayon de Schwarzschild (2GM / c^2) devient donc simplement 2 * G * masse.
    const float G = 1.0f;
    const float c_light = 1.0f;

    // distance au trou noir
    glm::vec3 diff = pos - glm::vec3(c.cx, c.cy, c.cz);
    float R = glm::length(diff);

    // absorption ?
    if (R < c.rayon)
    {
        r.setAbsorbed(true);
        return;
    }

    // Modèle relativiste simplifié (Schwarzschild, trajectoire nulle) :
    // - on ne modifie que la direction (vitesse ~ c), l'accélération est
    //   appliquée orthogonalement au rayon pour courber sa trajectoire.
    // - la force est renforcée près de l'horizon (facteur (1 + 1.5 * Rs / R)).
    // - on renormalise la vitesse pour conserver |v| = c.
    const float Rs = 2.0f * G * c.masse / (c_light * c_light);

    glm::vec3 dirNorm = glm::normalize(dir);
    glm::vec3 radial = diff / R;
    glm::vec3 lateral = radial - glm::dot(radial, dirNorm) * dirNorm; // composante perpendiculaire

    glm::vec3 accel(0.0f);
    float latLen = glm::length(lateral);
    if (latLen > 1e-6f)
    {
        float strength = (G * c.masse) / (R * R);
        float relativisticBoost = 1.0f + 1.5f * (Rs / R);
        accel = -relativisticBoost * strength * (lateral / latLen);
    }

    // intégration Euler + renormalisation (|dir| = c)
    dir += accel * dt;
    dir = glm::normalize(dir) * c_light;
    pos += dir * dt;

    // mise à jour
    r.dir() = dir;
    r.pos() = pos;

    r.addPoint(pos);
}
