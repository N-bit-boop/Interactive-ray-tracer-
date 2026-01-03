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
const double MOVE_SPEED = 3.0;
double yaw   = -90.0; // facing -Z initially
double pitch =  0.0;
const double MOUSE_SENS = 0.1;

const int MAX_ACCUM_FRAMES = 64;

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
    std::vector<color> accum_buffer(WIDTH *HEIGHT , color(0,0,0));
    int frame_count{0};

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
    cam.aspect_ratio = double(WIDTH)/HEIGHT;
    cam.image_width = WIDTH;
    cam.samples_per_pixel = 1;
    cam.max_depth = 50;


    cam.vfov = 90;
    cam.lookat = point3(0,0,-1);
    cam.camera_position = point3(-2,2,1);
    cam.lookfrom = cam.camera_position;
    cam.vup = vec3(0,1,0);

    cam.prepare();

    bool camera_moved = false;

    Uint64 last_ticks  = SDL_GetPerformanceCounter();
    double delta_time = 0.0; //Seconds since last frame

      //enable relative mouse mode
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // 6. Main loop
    while (running) {

        // --- Event handling ---
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        Uint64 current_ticks = SDL_GetPerformanceCounter();
        delta_time  = (double)(current_ticks - last_ticks) / SDL_GetPerformanceFrequency();
        last_ticks = current_ticks;
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        int dx, dy;
        
        SDL_GetRelativeMouseState(&dx, &dy);
        yaw += dx * MOUSE_SENS;
        pitch -= dy * MOUSE_SENS;

        if(pitch > 89.0) pitch = 89.0;
        if(pitch < -89.0) pitch = -89.0; //Prevents camera flipping and gimbal lock

        vec3 forward(
            cos(degrees_to_radians(yaw)) * cos(degrees_to_radians(pitch)),
            sin(degrees_to_radians(pitch)),
            sin(degrees_to_radians(yaw)) * cos(degrees_to_radians(pitch))
        );
        forward = unit_vector(forward);
        
        vec3 right = unit_vector(cross(forward, cam.vup));


        if (keys[SDL_SCANCODE_W]) {
        cam.camera_position += forward * MOVE_SPEED * delta_time;
        }

        if (keys[SDL_SCANCODE_S]) {
            cam.camera_position -= forward * MOVE_SPEED * delta_time;
        }

        if (keys[SDL_SCANCODE_A]) {
            cam.camera_position -= right * MOVE_SPEED * delta_time;
        }

        if (keys[SDL_SCANCODE_D]) {
            cam.camera_position += right * MOVE_SPEED * delta_time;
        }
        
        if (abs(dx) > 0 || abs(dy) > 0 ||
            keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_A] ||
            keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_D]) {
            camera_moved = true;
        }
        
        
       if(camera_moved){
        cam.lookfrom = cam.camera_position;
        cam.lookat =cam.camera_position + forward;
        cam.prepare(); //Because we moved around the internal math was invalid, need to re inititialize
        std::fill(accum_buffer.begin(), accum_buffer.end(), color(0,0,0));
        frame_count = 0; //Averaging samples from different camera positions and requires a clear 
       } // Old samples are invalid as the camera changes
       
       frame_count++;


        // --- Render into CPU framebuffer ---
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                int idx = y * WIDTH + x;
                //Trace one new sample
                ray r = cam.get_ray(x,y);
                color sample = cam.ray_color(r,cam.max_depth, world);
                
                
                

                if (frame_count < MAX_ACCUM_FRAMES){
                
                    //Accumulate in float buffer
                    accum_buffer[idx] += sample;
                }

                //Compute average colour
                int denom = std::min(frame_count, MAX_ACCUM_FRAMES);
                color averaged = accum_buffer[idx] / denom;

                //convert into a displayable pixel
                framebuffer[idx] = pack_color(averaged);

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
