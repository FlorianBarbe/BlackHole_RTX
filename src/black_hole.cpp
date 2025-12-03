#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include <cstdio>
#include <direct.h>

#include "rayon.h"
#include "cercle.h"
#include "utilities.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

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

	g_yaw += (float)dx * 0.003f;
	g_pitch -= (float)dy * 0.003f; // inversion Y correcte

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
	// 2. CHARGEMENT TEXTURE CIEL
	// =========================
	GLuint starTexture;
	{
		int w, h, ch;
		unsigned char* data = stbi_load("ciel.jpg", &w, &h, &ch, STBI_rgb);

		glGenTextures(1, &starTexture);
		glBindTexture(GL_TEXTURE_2D, starTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}


	// =========================
	// 3. CAMERA
	// =========================
	float camDist = 8.0f;

	glm::mat4 proj =
		glm::perspective(glm::radians(60.0f),
			2560.f / 1080.f,
			0.1f,
			500.0f);


	// =========================
	// 4. BLACK HOLE
	// =========================
	cercle blackHole(0.0f, 0.0f, 0.0f, 0.45f, 0.1f);


	// =========================
	// 5. ÉCRAN RECTANGULAIRE
	// =========================
	std::vector<rayon> rays;

	int Nx = 100;
	int Ny = 100;

	float W = 1.5f;
	float H = 1.5f;
	float Z0 = 0.50f;

	float dx = W / Nx;
	float dy = H / Ny;

	glm::vec3 screenCenter(0.0f, 0.0f, Z0);
	glm::vec3 right(1.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);

	for (int j = 0; j < Ny; ++j)
	{
		for (int i = 0; i < Nx; ++i)
		{
			float u = (i + 0.5f) * dx - W * 0.5f;
			float v = (j + 0.5f) * dy - H * 0.5f;

			glm::vec3 origin = screenCenter + u * right + v * up;
			glm::vec3 dir = glm::normalize(glm::vec3(0, 0, -1));

			rays.emplace_back(origin, dir, glm::vec3(1, 1, 1));
		}
	}

	float dt = 0.01f;


	// =========================
	// 6. MAIN LOOP
	// =========================
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// ---------------- CAMERA ORBITALE ----------------
		glm::vec3 camPos;
		camPos.x = camDist * cos(g_pitch) * sin(g_yaw);
		camPos.y = camDist * sin(g_pitch);
		camPos.z = camDist * cos(g_pitch) * cos(g_yaw);

		glm::mat4 view = glm::lookAt(
			camPos,
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);


		// ---------------- CLEAR ----------------
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// ============ SET PROJECTION ============
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&proj[0][0]);


		// ====================================================
		// SKY SPHERE — fond étoilé
		// ====================================================
		glDisable(GL_DEPTH_TEST);

		glMatrixMode(GL_MODELVIEW);

		// enlever la translation → fond infini
		glm::mat4 skyView = glm::mat4(glm::mat3(view));
		glLoadMatrixf(&skyView[0][0]);

		glBindTexture(GL_TEXTURE_2D, starTexture);
		glEnable(GL_TEXTURE_2D);

		float R = 100.0f;
		int slices = 50;
		int stacks = 50;

		for (int i = 0; i < stacks; i++)
		{
			float lat0 = M_PI * (-0.5f + (float)(i) / stacks);
			float z0 = sin(lat0);
			float zr0 = cos(lat0);

			float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
			float z1 = sin(lat1);
			float zr1 = cos(lat1);

			glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0; j <= slices; j++)
			{
				float lng = 2.0f * M_PI * (float)j / slices;
				float x = cos(lng);
				float y = sin(lng);

				float u = (float)j / slices;
				float v0 = (float)i / stacks;
				float v1 = (float)(i + 1) / stacks;

				glTexCoord2f(u, v1);
				glVertex3f(R * x * zr1, R * y * zr1, R * z1);

				glTexCoord2f(u, v0);
				glVertex3f(R * x * zr0, R * y * zr0, R * z0);
			}
			glEnd();
		}

		glDisable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);


		// ====================================================
		//   DESSIN TROU NOIR (on remet la vraie view)
		// ====================================================
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&view[0][0]);

		glColor3f(0, 0, 0);
		blackHole.draw3D();


		// ====================================================
		//   RAYS
		// ====================================================
		glColor3f(1, 1, 1);
		glLineWidth(1.0f);

		for (size_t i = 0; i < rays.size(); )
		{
			if (rays[i].isAbsorbed())
			{
				rays.erase(rays.begin() + i);
				continue;
			}

			update(dt, rays[i], blackHole);

			glBegin(GL_LINE_STRIP);
			for (auto& p : rays[i].getTrail())
				glVertex3fv(&p.x);
			glEnd();

			i++;
		}

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
