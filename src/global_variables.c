#include "global_variables.h"

int Global_quit = 0;

int mm = 1;
int pause = 0;
int pauseOption = 0;
int playing = 1;

SDL_Event event;

Mix_Chunk *sounds[NUM_CHANNELS];

SDL_Color bg_color = {0, 0, 10, 0};
SDL_Color borderColor = {0, 0, 200, 0};
SDL_Color white = {255, 255, 255, 0};
SDL_Color yellow = {255, 255, 0, 0};
