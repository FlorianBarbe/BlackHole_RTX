#include "cercle.h"
#include <GLFW/glfw3.h>
#include <cmath>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void cercle::draw3D() const
{
    const int slices = 32;
    const int stacks = 32;

    glColor3f(r, g, b)

    for (int i = 0; i < stacks; ++i)
    {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float z0 = std::sin(lat0);
        float zr0 = std::cos(lat0);

        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
        float z1 = std::sin(lat1);
        float zr1 = std::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
            float lng = 2.0f * M_PI * (float)j / slices;
            float x = std::cos(lng);
            float y = std::sin(lng);

            glVertex3f(
                cx + rayon * x * zr0,
                cy + rayon * y * zr0,
                cz + rayon * z0
            );

            glVertex3f(
                cx + rayon * x * zr1,
                cy + rayon * y * zr1,
                cz + rayon * z1
            );
        }
        glEnd();
    }
}
