#include <SDL2/SDL.h>
#include <emscripten.h>
#include <iostream>

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// Bouncing square state
float posX = 100.0f;
float posY = 100.0f;
float velX = 4.0f;
float velY = 3.0f;
int boxSize = 40;

void loop() {
    // 1. Get current canvas dimensions (adapts if resized)
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    // 2. Clear background to dark charcoal
    SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
    SDL_RenderClear(renderer);

    // 3. Update physics for the bouncing square
    posX += velX;
    posY += velY;

    if (posX <= 0 || posX + boxSize >= width)  velX = -velX;
    if (posY <= 0 || posY + boxSize >= height) velY = -velY;

    // 4. Draw static geometry
    // Yellow wireframe target box in center
    SDL_SetRenderDrawColor(renderer, 240, 200, 40, 255);
    SDL_Rect centerBox = { width / 2 - 80, height / 2 - 80, 160, 160 };
    SDL_RenderDrawRect(renderer, &centerBox);

    // Diagonal cross lines inside the center box
    SDL_RenderDrawLine(renderer, width / 2 - 80, height / 2 - 80, width / 2 + 80, height / 2 + 80);
    SDL_RenderDrawLine(renderer, width / 2 - 80, height / 2 + 80, width / 2 + 80, height / 2 - 80);

    // Red floor bar at the bottom
    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255);
    SDL_Rect floorBar = { 0, height - 12, width, 12 };
    SDL_RenderFillRect(renderer, &floorBar);

    // 5. Draw the animated bouncing green square
    SDL_SetRenderDrawColor(renderer, 40, 220, 100, 255);
    SDL_Rect bouncingSquare = { static_cast<int>(posX), static_cast<int>(posY), boxSize, boxSize };
    SDL_RenderFillRect(renderer, &bouncingSquare);

    // 6. Present current frame
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "SDL2 Drawing Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::cout << "SDL2 animation started!" << std::endl;
loop();
    // 0 = match browser requestAnimationFrame (~60fps)
    emscripten_set_main_loop([]{}, 20, 1);

    return 0;
}