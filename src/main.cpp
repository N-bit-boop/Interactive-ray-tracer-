#include <SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>

#include "rtweekend.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "vec.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "material.hpp"


const int WIDTH  = 800;
const int HEIGHT = 600;
const int max_depth = 10;

int main(int argc, char* argv[])
{
    // 1. Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    // 2. Create window
    SDL_Window* window = SDL_CreateWindow(
        "CPU Framebuffer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // 3. Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Create CPU framebuffer (RGBA8888)
    std::vector<uint32_t> framebuffer(WIDTH * HEIGHT);

    // 5. Create SDL texture (GPU-side)
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;

    //Creating the scene 

    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.8,0.8,0.0));
    auto center_material = make_shared<lambertian>(color(0.1,0.2,0.5));

    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100, ground_material));
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, center_material));

    camera cam;
    cam.aspect_ratio = 16.0/9.0;
    cam.image_width = WIDTH;
    cam.samples_per_pixel = 1;
    cam.max_depth = 50;


    cam.vfov = 90;
    cam.lookat = point3(0,0,-1);
    cam.camera_position = point3(-2,2,1);
    cam.lookfrom = cam.camera_position;
    cam.vup = vec3(0,1,0);

    cam.prepare();


    // 6. Main loop
    while (running) {

        // --- Event handling ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // --- Render into CPU framebuffer ---
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                ray r = cam.get_ray(x, y);
                color pixel_color = cam.ray_color(r, cam.max_depth, world);
                pixel_color *= cam.pixel_samples_scale;
                framebuffer[y*WIDTH + x] = pack_color(pixel_color);
            }
        }

        // --- Upload framebuffer to texture ---
        SDL_UpdateTexture(
            texture,
            nullptr,
            framebuffer.data(),
            WIDTH * sizeof(uint32_t)
        );

        // --- Draw texture to window ---
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // 7. Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
