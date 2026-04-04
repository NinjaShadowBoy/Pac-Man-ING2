#pragma once

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "maze.h"

typedef struct Ghost {
    clock_t then;
    clock_t now;
    clock_t lastFrameSwap;
    int frame;
    int numOfFrames;
    int size;
    int movingLeft;
    int movingRight;
    int movingDown;
    int movingUp;
    int angle;
    int direction;
    double speedx;
    double speedy;
    double x;
    double y;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture** Textures;
    SDL_Rect rect;
    cell pos;
} Ghost;

int initGhost(Ghost* g, SDL_Window* window, SDL_Renderer* renderer);
void updateGhostFrame(Ghost* g);
int updateGhostPos(Ghost* g);
int drawGhost(Ghost* g);
