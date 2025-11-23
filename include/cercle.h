#pragma once
#include <cmath>
#include <GLFW/glfw3.h>

class cercle {
private:
	float x, y;//position
	float rayon; //rayou du trou noir
	float rlim; //rayon d'attraction limite
	float r, g, b;//couleur
	int n;//précision du contour
	int H, L;//dimension de la fenêtre
	float m;//masse
public:

	cercle(float x, float y, float rayon, float rlim, float r, float g, float b, float m, int fbW, int fbH)
		: x(x), y(y), rayon(rayon), rlim(rlim), r(r), g(g), b(b), n(100),
		L(fbW), H(fbH), m(m) {	}


	void draw() const;//tracé du cercle

	//getteurs
	float getr() const {return r;}
	float getg() const {return g;}
	float getb() const {return b;}
	float getx() const {return x;}
	float gety() const {return y;}
	float getrayon() const {return rayon;}
	float getrlim() const {return rlim;}
	int getH() const {return H;}
	int getL() const {return L;}
	float getm() const {return m;}

	void setm(float m2) { m = m2; }
};