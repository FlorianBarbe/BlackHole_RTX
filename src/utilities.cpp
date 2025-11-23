#include <utilities.h>



glm::vec2 OpenGLtoPolar(float x, float y){
    float R = sqrt(x * x + y * y);
    float theta = atan2f(y, x);
    return glm::vec2(R,theta);//(R,theta)
} 

void update(float dt, rayon& r, cercle& c)
{
    if (r.isAbsorbed()) return; // NE PLUS RIEN FAIRE

    glm::vec2 polar = OpenGLtoPolar(r.getX(), r.getY());
    float R = polar.x;

    if (R < c.getrlim()) {
        r.setAbsorbed(true);
        return;
    }

    // physique normale…
    float ax = -c.getm() * r.getX() / (R * R * R);
    float ay = -c.getm() * r.getY() / (R * R * R);

    r.setdx(r.getdx() + ax * dt);
    r.setdy(r.getdy() + ay * dt);

    r.setX(r.getX() + r.getdx() * dt);
    r.setY(r.getY() + r.getdy() * dt);

    r.addPoint();
}
