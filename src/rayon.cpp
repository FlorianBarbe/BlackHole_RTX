#include "rayon.h"



void rayon::draw() const
{
    float ratio = (float)L / (float)H;

    // point du rayon
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2f(xr / ratio, yr);
    glEnd();

    // traînée
    glBegin(GL_LINE_STRIP);
    for (const auto& p : trail) {
        glVertex2f(p.x / ratio, p.y);
    }
    glEnd();
}

