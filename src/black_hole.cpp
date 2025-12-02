#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <random>
#include <cstdio>

#include "rayon.h"
#include "cercle.h"
#include "utilities.h"

#define _USE_MATH_DEFINES
#include <cmath>
const float M_PI = 3.14159265358979323846f;

// =============================
//   CAMERA ORBITALE
// =============================
static float g_yaw = 0.0f;
static float g_pitch = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static double lastX = xpos;
	static double lastY = ypos;

	double dx = xpos - lastX;
	double dy = ypos - lastY;

	lastX = xpos;
	lastY = ypos;

	g_yaw += dx * 0.003f;
	g_pitch += dy * 0.003f;

	if (g_pitch > 1.4f) g_pitch = 1.4f;
	if (g_pitch < -1.4f) g_pitch = -1.4f;
}


// ============================================================
//                     MAIN
// ============================================================
int main()
{
	// =========================
	// 1. INIT GLFW + GLEW
	// =========================
	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	GLFWwindow* window = glfwCreateWindow(2560, 1080, "Black Hole – Screen Rays", nullptr, nullptr);
	if (!window) { glfwTerminate(); return -1; }

	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();
	glEnable(GL_DEPTH_TEST);


	// =========================
	// 2. CAMERA
	// =========================
	float camDist = 8.0f;

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), 2560.f / 1080.f, 0.1f, 200.0f);


	// =========================
	// 3. BLACK HOLE
	// =========================
	cercle blackHole(0.0f, 0.0f, 0.0f, 0.45f, 0.1f);


	// =========================
	// 4. ÉCRAN RECTANGULAIRE
	// =========================
	std::vector<rayon> rays;

	int Nx = 20;     // résolution horizontale
	int Ny = 20;     // résolution verticale

	float W = 6.0f;   // largeur physique de l'écran
	float H = 4.0f;   // hauteur physique
	float Z0 = 4.0f;  // position écran sur l’axe Z

	float dx = W / Nx;
	float dy = H / Ny;

	glm::vec3 screenCenter(0.0f, 0.0f, Z0);
	glm::vec3 right(1.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	// ============================================
	// Génération des rayons "un par pixel"
	// ============================================
	for (int j = 0; j < Ny; ++j)
	{
		for (int i = 0; i < Nx; ++i)
		{
			float u = (i + 0.5f) * dx - W * 0.5f;
			float v = (j + 0.5f) * dy - H * 0.5f;

			glm::vec3 origin = screenCenter + u * right + v * up;

			// direction vers le trou noir
			glm::vec3 dir = glm::normalize(glm::vec3(0, 0, -1));// - origin);

			rays.emplace_back(origin, dir, glm::vec3(1, 1, 1));
		}
	}

	float dt = 0.01f;


	// =========================
	// 5. MAIN LOOP
	// =========================
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// ---------- caméra orbitale ----------
		glm::vec3 camPos;
		camPos.x = camDist * cos(g_pitch) * sin(g_yaw);
		camPos.y = camDist * sin(g_pitch);
		camPos.z = camDist * cos(g_pitch) * cos(g_yaw);

		glm::mat4 view = glm::lookAt(
			camPos,
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&proj[0][0]);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&view[0][0]);


		// =========================
		// Black hole (NOIR)
		// =========================
		glColor3f(0, 0, 0);
		blackHole.draw3D();


		// =========================
		// RAYS UPDATE + DRAW
		// =========================
		glColor3f(1, 1, 1);
		glLineWidth(1.0f);

		for (auto& r : rays)
		{
			update(dt, r, blackHole);

			glBegin(GL_LINE_STRIP);
			for (auto& p : r.getTrail())
				glVertex3fv(&p.x);
			glEnd();

			r.addPoint(r.pos());
		}

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
