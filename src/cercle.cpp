#include "cercle.h"
void cercle::draw() const
{
    float ratio = (float)L / (float)H;   // L=fbW, H=fbH

    glColor3f(r, g, b);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= n; i++) {
        float angle = i * 2.0f * 3.14159f / n;

        float px = x + (rayon * cosf(angle)) / ratio;  // compensation horizontale
        float py = y + (rayon * sinf(angle));

        glVertex2f(px, py);
    }

    glEnd();
}



