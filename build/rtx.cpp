#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdio>
#include <cmath>
#include "rayon.h"
#include "cercle.h"
#include "utilities.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// --- CONFIG ---
static float g_yaw = 0.0f;
static float g_pitch = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double lastX = xpos, lastY = ypos;
	double dx = xpos - lastX;
	double dy = ypos - lastY;
	lastX = xpos; lastY = ypos;
	g_yaw += (float)dx * 0.003f;
	g_pitch -= (float)dy * 0.003f;
	if (g_pitch > 1.4f) g_pitch = 1.4f;
	if (g_pitch < -1.4f) g_pitch = -1.4f;
}

// Convert direction → UV
inline void dirToUV(const glm::vec3& d, float& u, float& v)
{
	u = 0.5f + atan2(d.x, d.z) / (2.0f * M_PI);
	v = 0.5f - asin(d.y) / M_PI;
	u = fmodf(u + 1.0f, 1.0f);
	v = fmodf(v + 1.0f, 1.0f);
}

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Raytraced Black Hole", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewInit();

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Load sky texture
	int skyW, skyH, skyC;
	unsigned char* skyPixels = stbi_load("ciel.jpg", &skyW, &skyH, &skyC, 3);
	skyC = 3;
	printf("Sky loaded: %p  %d x %d  channels=%d\n", skyPixels, skyW, skyH, skyC);

	if (!skyPixels) {
		printf("ERREUR : image non trouvée !\n");
	}
	




	// --- Raytracing buffer ---
	const int Nx = 10;
	const int Ny = 10;
	std::vector<float> framebuffer(Nx * Ny * 3, 0.0f);

	// Screen
	float W = 1.5f;
	float H = 1.5f;
	float Z0 = 0.5f;
	float dx = W / Nx;
	float dy = H / Ny;
	glm::vec3 screenCenter(0, 0, Z0);
	glm::vec3 right(1, 0, 0), up(0, 1, 0);

	// Black hole
	cercle blackHole(0, 0, 0, 0.45f, 0.1f);
	float dt = 0.01f;
	float camDist = 8.0f;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Camera orbit
		glm::vec3 camPos;
		camPos.x = camDist * cos(g_pitch) * sin(g_yaw);
		camPos.y = camDist * sin(g_pitch);
		camPos.z = camDist * cos(g_pitch) * cos(g_yaw);

		// Raytrace each pixel
		for (int py = 0; py < Ny; py++) {
			for (int px = 0; px < Nx; px++) {

				float u = (px + 0.5f) * dx - W * 0.5f;
				float v = (py + 0.5f) * dy - H * 0.5f;
				glm::vec3 origin = screenCenter + u * right + v * up;
				glm::vec3 dir = glm::normalize(origin - camPos);

				rayon r(origin, dir, glm::vec3(1, 1, 1));


				glm::vec3 color(0);
				bool finished = false;

				for (int step = 0; step < 2000 && !finished; step++) {
					update(dt, r, blackHole);

					if (r.isAbsorbed()) {
						color = glm::vec3(0, 0, 0);
						finished = true;
					}
					else if (glm::length(r.pos()) > 50.0f) {
						float U, V;
						dirToUV(glm::normalize(r.dir()), U, V);
						int sx = int(U * skyW);
						int sy = int(V * skyH);
						sx = std::max(0, std::min(sx, skyW - 1));
						sy = std::max(0, std::min(sy, skyH - 1));
						int idx = (sy * skyW + sx) * 3;
						color = glm::vec3(skyPixels[idx] / 255.0f,
							skyPixels[idx + 1] / 255.0f,
							skyPixels[idx + 2] / 255.0f);
						finished = true;
					}
				}

				int index = (py * Nx + px) * 3;
				framebuffer[index + 0] = color.r;
				framebuffer[index + 1] = color.g;
				framebuffer[index + 2] = color.b;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glRasterPos2f(-1, -1);
		glDrawPixels(Nx, Ny, GL_RGB, GL_FLOAT, framebuffer.data());
		glfwSwapBuffers(window);
	}

	stbi_image_free(skyPixels);
	glfwTerminate();
	return 0;
}