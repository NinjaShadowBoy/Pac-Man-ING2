#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>


#include <menu.h>

void printScore(button digits[20], int num, int x, int y);
void checkEvents(int numGhosts);
int initSDL();
int playLevel(int level, int gameMode, int previousScore);
void handleGhostPacCollisions(int numGhosts);
void handlePacMazeInteractions(int numGhosts);
void checkEvents(int numGhosts);
void main_Menu(menu MainMenu, int* choice, int* level);
