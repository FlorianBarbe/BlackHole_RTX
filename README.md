# BlackHole RTX

<img width="1282" height="752" alt="image" src="https://github.com/user-attachments/assets/017a6c4c-88a3-4a52-8ebd-ed4be5147f5f" />

## Journal de Développement

Ce projet a pour but de simuler un **trou noir de Schwarzschild** en temps réel en utilisant le **Ray Marching** et l'accélération GPU via **OpenGL/CUDA**. Voici l'historique de son développement, des premiers tests aux résultats finaux.

### 📅 23 Novembre 2025 : Initialisation
Lancement du projet. Mise en place de l'environnement de développement et réflexion sur l'architecture du moteur de rendu. L'objectif est de porter les équations géodésiques sur GPU pour un rendu interactif.

### 📅 02 Décembre 2025 : Premiers Tests Visuels & Bugs
Premières tentatives d'intégration du Ray Marching pour la déformation de l'espace-temps.

**Premier bug majeur :** Problème de coordonnées de texture et de distorsion. L'effet de lentille gravitationnelle était déformé de manière incohérente, créant des artefacts visuels sur les bords de l'écran.

![Bug Texture](data/Capture%20d'%C3%A9cran%202025-12-02%20222558.png)
*Figure 1 : Artefacts visuels lors des premiers tests de mapping.*

**Correction & Progrès en fin de soirée :** Après correction des vecteurs de direction dans le fragment shader, l'horizon des événements commence à se dessiner correctement, bien que l'accretion disk manque encore de détails.

![Progrès Soirée](data/Capture%20d'%C3%A9cran%202025-12-02%20234404.png)
*Figure 2 : Rendu corrigé, l'ombre du trou noir est visible.*

### 📅 03 Décembre 2025 : Implémentation Core
Intégration complète des shaders (`fragment_btz.glsl`) et du moteur principal (`main_btz.cpp`).
- Implémentation finale de la métrique de Schwarzschild.
- Optimisation du pas de Ray Marching pour garantir 60 FPS.
- Ajout d'une texture de fond (Ciel étoilé) pour mieux visualiser l'effet de lentille (Lensing).

### 📅 18 Janvier 2026 : Finalisation & Push
Reprise du projet pour nettoyage et publication.
- Merge de la branche `lsmc` contenant les dernières optimisations.
- Documentation et archivage des résultats.

---
*Projet réalisé explorer le rendu volumétrique relativiste en temps réel.*
