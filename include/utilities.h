#pragma once
#include "rayon.h"
#include "cercle.h"

void update(float dt, rayon& r, const cercle& c);

// Coordonnées polaires (plan XY)
struct PolarCoord
{
	float r;    // rayon
	float phi;  // angle
};

// Conversions
PolarCoord cartesianToPolar(const glm::vec3& p);
glm::vec3   polarToCartesian(const PolarCoord& P, float z = 0.0f);

// Paramètre d’impact
float computeImpactParameter(const glm::vec3& origin, const glm::vec3& dir);

// Normalisation photon (||v|| = 1)
glm::vec3 normalizePhoton(const glm::vec3& v);

// Clamp + conversions
float clampf(float x, float a, float b);
float degToRad(float deg);
float radToDeg(float rad);

// Debugging
void printPolar(const PolarCoord& P);
void printVec(const glm::vec3& v);