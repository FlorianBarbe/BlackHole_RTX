#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdio>

#include "rayon.h"
#include "cercle.h"
#include "utilities.h"
#include "raygen.h"

#define _USE_MATH_DEFINES
#include <cmath>
const float M_PI = 3.14159265358979323846f;



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

int main()
{
    // =========================
    // 1. INIT GLFW + GLEW
    // =========================
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Black Hole – Scene 3D + Rays", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewInit();

    glEnable(GL_DEPTH_TEST);

    // =========================
    // 2. CAMERA
    // =========================
    float distance = 6.0f; // distance caméra -> centre

    // Projection perspective
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1280.f / 720.f, 0.1f, 200.0f);

    // =========================
    // 3. OBJET CENTRAL (TROU NOIR)
    // =========================
    cercle blackHole(0.0f, 0.0f, 0.0f, 0.4f, 0.1f);

    // =========================
    // 4. GÉNÉRATION DES RAYONS AUTOUR
    // =========================
    std::vector<rayon> rays;

    int Ntheta = 35;
    int Nphi = 35;
    float R0 = 3.0f; // rayon de départ des rayons

    for (int it = 0; it < Ntheta; ++it)
    {
        float theta = (float)it / (Ntheta - 1) * M_PI;

        for (int ip = 0; ip < Nphi; ++ip)
        {
            float phi = (float)ip / (Nphi - 1) * 2.0f * M_PI;

            glm::vec3 origin(
                R0 * sin(theta) * cos(phi),
                R0 * sin(theta) * sin(phi),
                R0 * cos(theta)
            );

            glm::vec3 dir = glm::normalize(-origin); // vers le centre

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

        // ----------- CAMÉRA ORBITALE -----------
        glm::vec3 camPos;
        camPos.x = distance * cos(g_pitch) * sin(g_yaw);
        camPos.y = distance * sin(g_pitch);
        camPos.z = distance * cos(g_pitch) * cos(g_yaw);

        glm::mat4 view = glm::lookAt(
            camPos,
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        // ----------- CLEAR -----------
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ----------- MATRICES OPENGL FIXED PIPELINE -----------
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&proj[0][0]);

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&view[0][0]);

        // =========================
        // DESSIN TROU NOIR
        // =========================
        blackHole.draw3D();

        // =========================
        // DESSIN RAYONS (lignes blanches)
        // =========================

        glColor3f(1, 1, 1);
        glLineWidth(1.5f);

        for (auto& r : rays)
        {
            // Mise à jour physique
            update(dt, r, blackHole);

            // Tracé
            glBegin(GL_LINE_STRIP);
            for (auto& p : r.getTrail())
                glVertex3fv(&p.x);
            glEnd();

            // Ajout du point actuel dans la trace
            r.addPoint(r.pos());
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
