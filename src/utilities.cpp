#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

#include "utilities.h"
#include "rayon.h"
#include "cercle.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------------------------------------------------------
//   KERR AVEC AXE DE ROTATION CHOISISSABLE
//   spinAxis : axe de spin du trou noir (normalisé)
//   On ne fait QUE la physique ici.
// ---------------------------------------------------------

void update(float dt, rayon& r, const cercle& c)
{
	if (r.isAbsorbed()) return;

	// ==================== POSITION & DIRECTION ====================
	glm::vec3 pos = r.pos();
	glm::vec3 dir = glm::normalize(r.dir());

	glm::vec3 center(c.cx, c.cy, c.cz);

	// ==================== PARAMÈTRES KERR ====================
	float Rs = c.rayon;    // rayon de Schwarzschild (graphique)
	float M = 0.5f * Rs;  // G = c = 1 → Rs = 2M → M = Rs/2
	float a = 1.80f * M;  // spin (modulable)

	// ==================== AXE DE SPIN ====================
	// Ici: rotation autour de l’axe Y
	glm::vec3 spinAxis = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));

	// ==================== GÉOMÉTRIE ====================
	glm::vec3 diff = pos - center;
	float R = glm::length(diff);

	// Absorption : horizon
	if (R < Rs) {
		r.setAbsorbed(true);
		return;
	}

	glm::vec3 radial = diff / R;

	// ============================================
	// Composante latérale (courbure type Schwarzschild)
	// ============================================
	glm::vec3 lateral = radial - glm::dot(radial, dir) * dir;
	float latLen2 = glm::dot(lateral, lateral);

	float grav = M / (R * R);
	float boost = 1.0f + 1.5f * (Rs / R);
	boost = std::min(boost, 50.0f);

	glm::vec3 accel_lat(0.0f);
	if (latLen2 > 1e-10f)
		accel_lat = -boost * grav * (lateral / std::sqrt(latLen2));

	// ============================================
	// FRAME DRAGGING (KERR)
	// ============================================
	glm::vec3 tangent = glm::cross(spinAxis, radial);
	float tlen = glm::length(tangent);
	if (tlen < 1e-10f) {
		tangent = glm::cross(spinAxis, diff);
		tlen = glm::length(tangent);
	}
	if (tlen > 1e-10f)
		tangent /= tlen;
	else
		tangent = glm::vec3(0.0f, 0.0f, 0.0f);

	// terme de frame dragging ~ aM / R^3
	float fd = (2.0f * a * M) / (R * R * R);
	glm::vec3 accel_fd = fd * tangent;

	// ============================================
	// ACCÉLÉRATION TOTALE
	// ============================================
	glm::vec3 accel = accel_lat + accel_fd;

	// ==================== MAJ DIRECTION ====================
	dir += accel * dt;
	dir = glm::normalize(dir);

	// ==================== MAJ POSITION ====================
	pos += dir * dt;

	r.pos() = pos;
	r.dir() = dir;
	r.addPoint(pos);  // facultatif, utile si tu veux garder la "trail"
}
