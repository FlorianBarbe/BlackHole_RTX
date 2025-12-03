#include <cmath>
#include <vector>
#include <cstdio>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define _USE_MATH_DEFINES
const float M_PI = 3.14159265358979323846f;
// -------------------------------------------------
// Config
// -------------------------------------------------
static const int WIDTH = 200;
static const int HEIGHT = 200;

static float g_yaw = 0.0f;
static float g_pitch = 0.0f;
static float g_dist = 6.0f;   // distance caméra au centre

// HDR
static float* g_hdrPixels = nullptr;
static int g_hdrW = 0, g_hdrH = 0, g_hdrCh = 0;

// -------------------------------------------------
// Input souris : caméra orbitale
// -------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool   first = true;
	static double lastX = 0.0;
	static double lastY = 0.0;

	if (first) {
		lastX = xpos;
		lastY = ypos;
		first = false;
	}

	double dx = xpos - lastX;
	double dy = ypos - lastY;
	lastX = xpos;
	lastY = ypos;

	g_yaw += (float)dx * 0.0025f;
	g_pitch -= (float)dy * 0.0025f;

	const float pitchLimit = 1.4f;
	if (g_pitch > pitchLimit) g_pitch = pitchLimit;
	if (g_pitch < -pitchLimit) g_pitch = -pitchLimit;
}

// -------------------------------------------------
// Mapping direction -> HDR (equirectangulaire)
// -------------------------------------------------
glm::vec3 sampleHDR(const glm::vec3& dirIn)
{
	if (!g_hdrPixels || g_hdrW <= 0 || g_hdrH <= 0 || g_hdrCh < 3)
		return glm::vec3(0.0f);

	glm::vec3 d = glm::normalize(dirIn);

	// equirect : longitude = atan2(x,z), latitude = asin(y)
	float u = 0.5f + std::atan2(d.x, d.z) / (2.0f * (float)M_PI);
	float v = 0.5f - std::asin(d.y) / (float)M_PI;

	// wrap
	u = u - std::floor(u);
	v = v - std::floor(v);

	int x = (int)(u * g_hdrW);
	int y = (int)(v * g_hdrH);

	if (x < 0) x = 0;
	if (x >= g_hdrW) x = g_hdrW - 1;
	if (y < 0) y = 0;
	if (y >= g_hdrH) y = g_hdrH - 1;

	int idx = (y * g_hdrW + x) * g_hdrCh;

	return glm::vec3(
		g_hdrPixels[idx + 0],
		g_hdrPixels[idx + 1],
		g_hdrPixels[idx + 2]
	);
}

// -------------------------------------------------
// Approximatif : courbure Schwarzschild
// -------------------------------------------------
glm::vec3 gravitate(const glm::vec3& pos)
{
	// trou noir au centre
	const float Rs = 1.0f;      // rayon de Schwarzschild "artistique"
	float r = glm::length(pos);
	if (r < 1e-4f) return glm::vec3(0.0f);

	glm::vec3 radial = pos / r;
	float g = 0.12f / (r * r);  // intensité gravitationnelle

	return -g * radial;         // tire vers le centre
}

// -------------------------------------------------
// Main
// -------------------------------------------------
int main()
{
	// ---------------- GLFW ----------------
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	GLFWwindow* window =
		glfwCreateWindow(WIDTH, HEIGHT, "Fast Raytraced Sky (HDR + GR)", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	// ---------------- HDR load ----------------
	{
		stbi_set_flip_vertically_on_load(false); // equirect classique

		if (!stbi_is_hdr("waw2.hdr")) {
			std::printf("ERROR: waw2.hdr n'est pas détecté comme HDR.\n");
			return -1;
		}

		g_hdrPixels = stbi_loadf("waw2.hdr", &g_hdrW, &g_hdrH, &g_hdrCh, 3);
		if (!g_hdrPixels) {
			std::printf("ERROR: impossible de charger waw2.hdr : %s\n",
				stbi_failure_reason());
			return -1;
		}

		std::printf("HDR loaded: %d x %d  channels=%d\n",
			g_hdrW, g_hdrH, g_hdrCh);
	}

	// ---------------- Texture écran ----------------
	GLuint screenTex;
	glGenTextures(1, &screenTex);
	glBindTexture(GL_TEXTURE_2D, screenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
		WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	std::vector<unsigned char> framebuffer(WIDTH * HEIGHT * 3);

	const float fovDeg = 60.0f;
	const float fovRad = fovDeg * (float)M_PI / 180.0f;
	const float tanHalf = std::tan(fovRad * 0.5f);
	const float aspect = (float)WIDTH / (float)HEIGHT;

	// ---------------- Boucle principale ----------------
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// --- construire la base caméra depuis yaw/pitch ---
		glm::vec3 forward(
			std::cos(g_pitch) * std::sin(g_yaw),
			std::sin(g_pitch),
			std::cos(g_pitch) * std::cos(g_yaw)
		);
		forward = glm::normalize(forward);

		glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		glm::vec3 camPos = -forward * g_dist;

		// --- Raytracing CPU ---
		const int MAX_STEPS = 150;
		const float dt = 0.05f;
		const float horizonR = 0.8f;   // rayon "trou noir"
		const float farLimit = 40.0f;  // distance où on considère être "loin"

		const float exposure = 1.5f;
		const float invGamma = 1.0f / 2.2f;

		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				// coords normalisées [-1,1]
				float ndcX = ((x + 0.5f) / (float)WIDTH) * 2.0f - 1.0f;
				float ndcY = ((y + 0.5f) / (float)HEIGHT) * 2.0f - 1.0f;

				// rayon en espace caméra
				glm::vec3 dirCam(
					ndcX * aspect * tanHalf,
					-ndcY * tanHalf,
					-1.0f
				);
				dirCam = glm::normalize(dirCam);

				// -> espace monde
				glm::vec3 dir = glm::normalize(
					dirCam.x * right +
					dirCam.y * up +
					dirCam.z * forward
				);

				glm::vec3 pos = camPos;
				glm::vec3 color(0.0f);
				bool done = false;

				for (int s = 0; s < MAX_STEPS; ++s)
				{
					// gravité
					glm::vec3 a = gravitate(pos);
					dir = glm::normalize(dir + a * dt);
					pos += dir * dt;

					float r = glm::length(pos);

					// absorbé par le trou noir
					if (r < horizonR) {
						color = glm::vec3(0.0f);
						done = true;
						break;
					}

					// on considère qu'on est "loin" -> on échantillonne le ciel
					if (r > farLimit) {
						color = sampleHDR(dir);
						done = true;
						break;
					}
				}

				if (!done) {
					// si on n'a touché ni le trou noir ni le ciel loin,
					// on prend quand même le ciel dans la direction finale
					color = sampleHDR(dir);
				}

				// tonemapping très simple
				color *= exposure;
				color = glm::clamp(color, 0.0f, 50.0f);
				color = glm::pow(color, glm::vec3(invGamma)); // gamma 2.2

				int idx = (y * WIDTH + x) * 3;
				framebuffer[idx + 0] = (unsigned char)(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
				framebuffer[idx + 1] = (unsigned char)(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
				framebuffer[idx + 2] = (unsigned char)(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
			}
		}

		// --- upload vers la texture écran ---
		glBindTexture(GL_TEXTURE_2D, screenTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
			framebuffer.data());

		// --- rendu du quad plein écran ---
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, WIDTH, HEIGHT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, screenTex);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
		glEnd();

		glDisable(GL_TEXTURE_2D);

		glfwSwapBuffers(window);
	}

	// clean
	if (g_hdrPixels)
		stbi_image_free(g_hdrPixels);

	glDeleteTextures(1, &screenTex);
	glfwTerminate();
	return 0;
}
