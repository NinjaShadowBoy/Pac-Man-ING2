#pragma once

#include "stdio.h"
#include "conio.h"
#include "windows.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "characters.h"
#include <math.h>

#include "global_variables.h"

typedef struct {
    int i;
    int j;
} coord;

typedef struct {
    coord* steps;
    unsigned int len;
} path;

typedef struct {
    int x;
    int y;
    int foods[5];
    int *l_top;
    int *l_left;
    int *l_right;
    int *l_bottom;
    int timesVisited;
    int distance;
    double size;
    double spacing;
    int isColorful;
    coord index;
    SDL_FRect rect;
    SDL_FRect foodrects[5];
    SDL_Window* window;
    SDL_Renderer* renderer;
} cell;

typedef struct {
    int sizeX;
    int sizeY;
    double top;
    double left;
    double right;
    double bottom;
    double cellSize;
    double cellSpacing;
    int numFoods;
    int isColorful;
    cell **self;
    SDL_Color cellCol;
    SDL_Color borderCol;
    SDL_Texture* foodTexture;
    SDL_Texture* bonusTexture;
    SDL_Renderer* renderer;
} plane;

void color(int fg, int bg);
plane InitializePlane(int numCellsX, int numCellsY, double topEdge, double leftEdge, double cellSize, double cellSpacing, SDL_Color cellCol, SDL_Color borderCol, SDL_Window* window, SDL_Renderer* renderer);
void DrawPlane(plane p, int showFood);
int existsIsolated(plane p);
void MazifyPlane(plane *p, int showProcess);
void frame(int x1, int x2, int y1, int y2);
void DrawCell(cell c, double size, double cellSpacing, SDL_Color cellCol, SDL_Color borderCol);
void DrawCellBorders(cell c, int size, int cellSpacing, SDL_Color borderCol);
int RandlinkCell(plane *p, int i, int j);
int RandlinkCellToIsolated(plane *p, int i, int j);
int IsCellIsolated(plane p, int i, int j);
int NumLinksAround(plane p, int i, int j);
int NumLinks(plane p, int i, int j);
int NumIsolatedAround(plane p, int i, int j);
int NumVisitedAround(plane p, int i, int j, int numberOfTimesVisited);
path DepthFirstSearch(plane p, coord start, coord end, int numberOfTimesVisited);
path shortPath(plane p, coord start, coord end);
double distanceBtw(cell a, cell b);
void ClassicMaze(plane *p);
