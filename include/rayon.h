#pragma once
#include <vector>
#include <glm/glm.hpp>

class rayon {
private:
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 color;

    std::vector<glm::vec3> trail;
    bool absorbed = false;

public:
    rayon(const glm::vec3& o, const glm::vec3& d, const glm::vec3& c)
        : origin(o), direction(d), color(c)
    {
        trail.push_back(o);
    }

    // GETTERS / SETTERS
    glm::vec3& pos() { return origin; }
    glm::vec3& dir() { return direction; }

    void addPoint(const glm::vec3& p) { trail.push_back(p); }
    const std::vector<glm::vec3>& getTrail() const { return trail; }

    bool isAbsorbed() const { return absorbed; }
    void setAbsorbed(bool v) { absorbed = v; }

    void draw3D() const;
};
