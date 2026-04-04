#pragma once

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "maze.h"

typedef struct pac {
    clock_t then;
    clock_t now;
    clock_t lastFrameSwap;
    int frame;
    int numOfFrames;
    int pow;
    int startOfPower;
    Uint8 lives;
    Uint8 lvl;
    double size;
    double minSize;
    double deltaSize;
    double damageFactor;
    double maxSize;
    int score;
    int hasEatenGhost;
    int movingLeft;
    int movingRight;
    int movingDown;
    int movingUp;
    double angle;
    double targetAngle;
    int direction;
    double speed;
    double x;
    double y;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture** Textures;
    SDL_FRect rect;
    cell pos;
} pac;

typedef struct Ghost {
    clock_t then;
    clock_t now;
    clock_t lastFrameSwap;
    clock_t lastStateSwap;
    clock_t lastTimeHit;
    int frame;
    int numOfFrames;
    int type;
    int state;
    int movingLeft;
    int movingRight;
    int movingDown;
    int movingUp;
    int angle;
    int direction;
    int hasDestination;
    int numtraces;
    double speed;
    double frightSpeed;
    double normalSpeed;
    double eatenSpeed;
    double size;
    double normalSize;
    double eatenSize;
    double x;
    double y;
    coord target;
    coord immediateDest;
    path trace;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture*** Textures;
    SDL_Texture* eyeTexture;
    SDL_FRect rect;
    cell pos;
} Ghost;

int initPac(pac* p, double size, SDL_Window* window, SDL_Renderer* renderer, int lvl);
void updatePacFrame(pac* p);
int updatePacPos(pac* p, plane plan);
int drawPac(pac* p);

int initGhost(Ghost *g, double size,  SDL_Window* window, SDL_Renderer* renderer, int numGhosts, int level, plane plan, pac p);
void updateGhostFrame(Ghost* g);
void setChasetarget(Ghost* g, plane plan, pac* p);
void reverseDirection(Ghost* g, plane plan);
void changeGhostDestination(Ghost* g, plane plan, pac* p, int numGhosts);
int updateGhostPos(Ghost* g, plane plan, coord way);
int drawGhost(Ghost* g);
