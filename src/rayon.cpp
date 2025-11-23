// rayon.cpp
#include "rayon.h"
#include <GLFW/glfw3.h>

void rayon::draw3D() const
{
    if (trail.empty()) return;

    glColor3f(color.r, color.g, color.b);
    glBegin(GL_LINE_STRIP);

    for (const glm::vec3& p : trail)
        glVertex3f(p.x, p.y, p.z);

    glEnd();
}
