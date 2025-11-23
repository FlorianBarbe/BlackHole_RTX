#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include <GLFW/glfw3.h>


class rayon {
private:
    float xr, yr;    // position dans le repère centré (OpenGL)
    float dx, dy;  // direction normalisée
    float r, g, b; // couleur
    std::vector<glm::vec2> trail;
    bool absorbed = false;
    int L, H;


         
public:
    rayon(float x, float y, float dx, float dy, float r, float g, float b, int Lwin, int Hwin)
        : xr(x), yr(y), dx(dx), dy(dy), r(r), g(g), b(b), L(Lwin), H(Hwin)
    {
        trail.push_back(glm::vec2(x, y));
    }


    rayon(float x, float y, rayon& r2) : xr(x), yr(y), dx(r2.dx), dy(r2.dy), r(r2.r), g(r2.g), b(r2.b)
    {
        trail.push_back(glm::vec2(x, y));
    }
        void addPoint() {
        trail.push_back(glm::vec2(xr, yr));
    }

    const std::vector<glm::vec2>& getTrail() const { return trail; }

    void draw() const; // affichage OpenGL

    float getX() const { return xr; }
    float getY() const { return yr; }
    float getdx() { return dx; }
    float getdy() { return dy; }

    void setX(float X) { xr = X; }
    void setY(float Y) { yr = Y; }
    void setdx(float dx2) { dx = dx2; }
    void setdy(float dy2) { dy = dy2; }

    bool isAbsorbed() const { return absorbed; }
    void setAbsorbed(bool v) { absorbed = v; }
};
