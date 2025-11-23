// black_hole.cpp
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <vector>

#include "utilities.h"
#include "cercle.h"
#include "rayon.h"

int main()
{
    // === 1) Paramètres de la simulation ===
    const float M = 0.1f;    // masse du trou noir
    const float dt = 0.01f;   // pas de temps
    const int   N = 300;     // nombre de rayons
    const float R0 = 1.5f;    // rayon de lancement des rayons

    // === 2) Initialisation GLFW ===
    if (!glfwInit()) {
        std::fprintf(stderr, "Erreur : glfwInit a échoué\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Borderless fullscreen windowed
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        "Black Hole 3D",
        nullptr, nullptr
    );
    if (!window) {
        std::fprintf(stderr, "Erreur : glfwCreateWindow a échoué\n");
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(window, 0, 0);
    glfwMakeContextCurrent(window);

    // Taille réelle du framebuffer
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    std::printf("Framebuffer = %d x %d\n", fbW, fbH);

    glViewport(0, 0, fbW, fbH);
    glEnable(GL_DEPTH_TEST);

    // === 3) Matrices de projection et de vue (caméra) ===
    float aspect = static_cast<float>(fbW) / static_cast<float>(fbH);

    // Projection perspective
    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f),
        aspect,
        0.1f,
        100.0f
    );

    // Caméra placée sur l’axe Z, regardant le trou noir à l’origine
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 camTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // === 4) Création du trou noir (sphère 3D) ===
    cercle c(
        0.0f, 0.0f, 0.0f,   // centre 3D
        0.4f,               // rayon de la sphère
        M                   // masse
    );

    // === 5) Création des rayons 3D ===
    std::vector<rayon> rayons;
    rayons.reserve(N);

    for (int i = 0; i < N; ++i)
    {
        float angle = 2.0f * 3.1415926f * i / N;

        // Position initiale : cercle de rayon R0 dans le plan z = 0
        glm::vec3 origin(
            R0 * std::cos(angle),
            R0 * std::sin(angle),
            0.0f
        );

        // Direction : principalement vers -X, avec une légère composante Y
        glm::vec3 dir(
            -1.0f,
            0.2f * std::sin(angle),
            0.0f
        );

        dir = glm::normalize(dir);

        rayons.emplace_back(origin, dir, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // === 6) Boucle principale ===
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Matrice vue (recalcul si tu veux animer la caméra)
        glm::mat4 view = glm::lookAt(camPos, camTarget, camUp);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Charger la projection ---
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&proj[0][0]);

        // --- Charger la vue ---
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(&view[0][0]);

        // Dessin du trou noir en 3D
        c.draw3D();

        // Mise à jour + dessin de chaque rayon en 3D
        for (auto& r : rayons)
        {
            update(dt, r, c);
            r.draw3D();
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
