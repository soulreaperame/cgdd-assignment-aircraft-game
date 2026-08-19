# Aircraft Fighting Game

A simple 2D top-down aircraft shooter written in C++ using OpenGL/GLUT.

Made in 2022 as a university assignment.

---

## Objective
Find and destroy all enemy aircraft scattered across the map to win.

## Controls
- **WASD** or **Arrow Keys** — Move
- **F** — Shoot
- **M** — Pause / Main menu
- **[ / ]** — Decrease / Increase aircraft speed

## Features
- Procedurally generated obstacles and starfield
- Randomly spawned enemy aircraft with patrol movement patterns
- Bullet and collision system (player, enemies, obstacles, map bounds)
- Explosion effects
- Win/lose screens with destroyed-enemy count

## Dependencies
- OpenGL
- GLUT (`glut.h`)

## Building
Compile `main.cpp` with a C++ compiler, linking against OpenGL and GLUT/FreeGLUT. For example:

```
g++ main.cpp -o game -lglut -lGLU -lGL
```

---
By Amier — 82688
