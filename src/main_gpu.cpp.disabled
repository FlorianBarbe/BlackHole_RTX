#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdio>

#include "raygen.h"
#include "utilities.h"
#include "cercle.h"
#include "rayon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float g_yaw = 0.0f;
static float g_pitch = 0.0f;

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    static double lastX = xpos;
    static double lastY = ypos;

    double dx = xpos - lastX;
    double dy = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    g_yaw += dx * 0.002f;
    g_pitch += dy * 0.002f;

    if (g_pitch > 1.5f) g_pitch = 1.5f;
    if (g_pitch < -1.5f) g_pitch = -1.5f;
}

int main()
{
    // ---------------- GLFW ------------------
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(900, 900, "BlackHole RT", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    glewInit();

    // ------------ Resolution fixe ------------
    const int W = 300;
    const int H = 300;

    // ------------ Camera pour ray-gen --------
    glm::vec3 camPos = glm::vec3(0, 0, 5);
    glm::vec3 target = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);

    glm::mat4 view_initial = glm::lookAt(camPos, target, up);

    // ------------ Trou noir ------------------
    cercle c(0, 0, 0, 0.4f, 0.1f);

    // ------------ Rays init ------------------
    std::vector<rayon> rays = generateCameraRays(
        W, H,
        60.0f,
        camPos,
        view_initial
    );

    const float dt = 0.01f;
    const float HIT_RADIUS = c.getRayon();
    const float FAR_CLIP = 50.0f;
    const int   MAX_STEPS = 800;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(2.0f);

        int idx = 0;

        glBegin(GL_POINTS);

        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
            {
                rayon r = rays[idx++];
                bool hitBH = false;
                bool escaped = false;

                for (int k = 0; k < MAX_STEPS; k++)
                {
                    update(dt, r, c);

                    const glm::vec3 p = r.pos;

                    float d = glm::length(p);
                    if (d < HIT_RADIUS)
                    {
                        hitBH = true;
                        break;
                    }
                    if (d > FAR_CLIP)
                    {
                        escaped = true;
                        break;
                    }
                }

                if (hitBH)
                    glColor3f(1, 1, 1);  // blanc (disque d’ombre inversée)
                else
                    glColor3f(0, 0, 0);  // noir

                float x = (float(i) / W) * 2 - 1;
                float y = (float(j) / H) * 2 - 1;

                glVertex2f(x, y);
            }

        glEnd();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
