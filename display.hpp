#pragma once
#include <SDL3/SDL.h>
#include <vector>

// Definitions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800 // Classic "Doom" resolution

static unsigned long FPS =  30;
#define FRAME_TARGET_TIME (1000 / FPS)

// Declarations
bool createWindow();
void destroyWindow();

// Keeps the loop running at consistent speed
void fixFrameRate();

// Clears the buffer to a specific color
void clearFrameBuffer(uint32_t color);

// Copies the buffer to the texture and presents it
void renderFrameBuffer();

// Sets a single pixel in the CPU buffer
void drawPixel(int x, int y, uint32_t color);