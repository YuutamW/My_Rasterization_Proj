#include "display.hpp"
#include <iostream>

// --- Internal Global Variables ---
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* colorBufferTexture = nullptr;
static std::vector<uint32_t> colorBuffer; 

// Time tracking
static uint64_t previous_frame_time = 0;
#define WINDOW_SCALE 1

bool createWindow() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // DIRECT SIZE: Window size now matches the buffer size exactly (128x128)
    if (!SDL_CreateWindowAndRenderer("Rasterizer", WINDOW_SCALE * SCREEN_WIDTH, WINDOW_SCALE *SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        std::cerr << "Error creating window/renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create the texture
    colorBufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    // IMPORTANT: This ensures pixels look like sharp blocks, not blurry blobs
    SDL_SetTextureScaleMode(colorBufferTexture, SDL_SCALEMODE_NEAREST);

    // Initialize the pixel buffer vector
    colorBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT);

    return true;
}

void destroyWindow() {
    SDL_DestroyTexture(colorBufferTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void fixFrameRate() {
    uint64_t time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    previous_frame_time = SDL_GetTicks();
}

void clearFrameBuffer(uint32_t color) {
    std::fill(colorBuffer.begin(), colorBuffer.end(), color);
}

void renderFrameBuffer() {
    SDL_UpdateTexture(
        colorBufferTexture,
        nullptr,
        colorBuffer.data(),
        (int)(SCREEN_WIDTH * sizeof(uint32_t))
    );

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, colorBufferTexture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void drawPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        colorBuffer[(SCREEN_WIDTH * y) + x] = color;
    }
}