#include <SDL.h>
#include <vector>
#include <cstdint>
#include <iostream>

const int WIDTH  = 800;
const int HEIGHT = 600;

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

                uint8_t r = static_cast<uint8_t>(255.0 * x / WIDTH);
                uint8_t g = static_cast<uint8_t>(255.0 * y / HEIGHT);
                uint8_t b = 0;
                uint8_t a = 255;

                framebuffer[y * WIDTH + x] =
                    (r << 24) | (g << 16) | (b << 8) | a;
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
