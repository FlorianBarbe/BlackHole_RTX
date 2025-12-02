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

	const float G = 100.0f;
	const float c_light = 10.0f;

	glm::vec3 diff = pos - glm::vec3(c.cx, c.cy, c.cz);
	float R = glm::length(diff);

	if (R < c.rayon)
	{
		r.setAbsorbed(true);
		return;
	}

	const float Rs = 2.0f * G * c.masse / (c_light * c_light);

	glm::vec3 dirNorm = glm::normalize(dir);
	glm::vec3 radial = diff / R;
	glm::vec3 lateral = radial - glm::dot(radial, dirNorm) * dirNorm;

	glm::vec3 accel(0.0f);
	float latLen = glm::length(lateral);

	if (latLen > 1e-6f)
	{
		float strength = (G * c.masse) / (R * R);
		float relativisticBoost = 1.0f + 1.5f * (Rs / R);
		accel = -relativisticBoost * strength * (lateral / latLen);
	}

	dir += accel * dt;
	dir = glm::normalize(dir) * c_light;
	pos += dir * dt;

	r.dir() = dir;
	r.pos() = pos;
	r.addPoint(pos);
}

