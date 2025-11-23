#include "utilities.h"

int main()
{
    // === 1) Constantes de la simulation ===
    float M = 0.01f;        // masse du trou noir
    float dt = 0.01f;       // pas de temps pour Euler
    int N = 200;            // nombre de rayons
    float R0 = 1.0f;        // rayon de lancement des rayons

    // === 2) Initialisation GLFW et fenêtre ===
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // écran en plein écran fenêtré
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        "OpenGL - Borderless Fullscreen",
        NULL,          // PAS de monitor → fenêtré
        NULL
    );

    // Déplacer la fenêtre en haut à gauche
    glfwSetWindowPos(window, 0, 0);

    glfwMakeContextCurrent(window);


    // récupérer les tailles réelles
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);

    printf("Framebuffer = %d x %d\n", fbW, fbH);

    // === 3) Création du trou noir ===
    cercle c(
        0.0f, 0.0f,     // centre
        0.1f, 0.1f,     // rayon visuel, rayon limite
        1.0f, 1.0f, 1.0f,
        M,
        fbW, fbH        // dimensions réelles de l'écran
    );

    // === 4) Création des rayons ===
    std::vector<rayon> rayons;
    rayons.reserve(N);

    for (int i = 0; i < N; i++)
    {
        float angle = (2.0f * 3.1415926f * i) / N;

        // position : cercle de rayon R0
        float x = R0 * cosf(angle);
        float y = R0 * sinf(angle);

        // direction tangentielle (effet lentille)
        float dx = -sinf(angle) * 0.3f;
        float dy = cosf(angle) * 0.1f;

        rayons.emplace_back(x, y, dx, dy, 1.0f, 1.0f, 1.0f, fbW, fbH);
    }

    // === 5) Boucle de rendu ===
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        c.draw();

        for (auto& r : rayons)
        {
            update(dt, r, c);
            r.draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // === 6) Fin ===
    glfwTerminate();
    return 0;
}
