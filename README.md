###Interactive ray tracer 


# Interactive CPU Ray Tracer
Interactive ray tracing engine written in **C++** with an **SDL2 frontend**.  
Supports real-time camera movement, accumulation-based anti-aliasing, and basic sphere scenes.

---

### 1. Build Requirements

- C++17 or later
- SDL2
- A system capable of running real-time CPU ray tracing (modern CPU recommended)

Ensure SDL2 is installed and properly linked.

---

### 2. Compile the Project

Example using g++ on Windows/Linux:

```bash
g++ *.cpp -o raytracer -lSDL2

```
⚠️ Compilation flags may vary depending on your SDL2 installation.

### 3. Run the Ray Tracer
```bash

./raytracer
```

A window will open displaying the scene. The CPU will continuously render frames into the framebuffer with accumulation-based anti-aliasing.


### 4. Controls
Camera Movement

Key	Action
W	Move forward
S	Move backward
A	Move left
D	Move right
Mouse	Look around / rotate

Other Controls

ESC → Quit application

Movement updates the camera position and resets frame accumulation for correct rendering.

### 5. Technical Highlights
Interactive camera with pitch/yaw rotation and WASD movement

CPU framebuffer with RGBA8888 format

Accumulation buffer for anti-aliasing (up to 64 frames)

Basic scene setup: spheres with Lambertian materials

Real-time re-rendering when the camera moves

SDL2 renderer and texture upload for display

Fully modular: camera, ray, vec, color, hittable_list, sphere, material

### 6. Project Structure
```bash
main.cpp                → Main loop, SDL initialization, event handling
camera.hpp / camera.cpp  → Camera implementation and ray generation
ray.hpp                  → Ray class
vec.hpp                  → 3D vector math
color.hpp                → Color utilities
hittable_list.hpp        → Scene object list
sphere.hpp               → Sphere geometry
material.hpp             → Material definitions (e.g., Lambertian)
rtweekend.hpp            → Utility functions (math, random, constants)
```
###7. Future Improvements
GPU acceleration for faster rendering

More complex materials (metal, dielectric, etc.)

Support for additional primitives (planes, triangles, meshes)

Scene loading from external files

Multi-threaded rendering to fully utilize CPU cores
