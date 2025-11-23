#pragma once

class cercle {
public:
    float cx, cy, cz;
    float rayon;
    float masse;

    float r, g, b;

    cercle(float x, float y, float z, float R, float M)
        : cx(x), cy(y), cz(z), rayon(R), masse(M),
        r(1.0f), g(1.0f), b(1.0f) {
    }

    void draw3D() const;

    float getRayon() const { return rayon; }

};

