#include <SDL.h>

#include <stdio.h>

int main(int argc, char* args[]){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL could not initialize: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Real time Ray tracing",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,600,
        SDL_WINDOW_SHOWN

    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool running = true;

    while(running){
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20,20,20,255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}