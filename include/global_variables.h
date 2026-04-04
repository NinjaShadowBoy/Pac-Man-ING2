#pragma once

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

#define SCREEN_WIDTH 1920 / 1.5
#define SCREEN_HEIGHT 1080 / 1.5

#define TRACK0 0
#define TRACK1 1
#define TRACK2 2
#define TRACK3 3
#define TRACK4 4
#define TRACK5 5
#define TRACK6 6

#define NUM_OF_LEVEL_SOUND_TRACKS 7

#define STAGE_CLEAR NUM_OF_LEVEL_SOUND_TRACKS+1
#define BG1 NUM_OF_LEVEL_SOUND_TRACKS+2
#define EAT NUM_OF_LEVEL_SOUND_TRACKS+3
#define SPECIAL_FOOD NUM_OF_LEVEL_SOUND_TRACKS+4
#define HURT NUM_OF_LEVEL_SOUND_TRACKS+5
#define GHOST_EATEN NUM_OF_LEVEL_SOUND_TRACKS+6
#define DIE NUM_OF_LEVEL_SOUND_TRACKS+7
#define CLICK NUM_OF_LEVEL_SOUND_TRACKS+8
#define MAIN_MENU NUM_OF_LEVEL_SOUND_TRACKS+9
#define STAGE_SELECT NUM_OF_LEVEL_SOUND_TRACKS+10
#define LEADERBOARDS NUM_OF_LEVEL_SOUND_TRACKS+11
#define ALARM NUM_OF_LEVEL_SOUND_TRACKS+12
#define NAME_ENTRY NUM_OF_LEVEL_SOUND_TRACKS+13
#define BEGIN NUM_OF_LEVEL_SOUND_TRACKS+14
#define COIN NUM_OF_LEVEL_SOUND_TRACKS+15
#define TYPE NUM_OF_LEVEL_SOUND_TRACKS+16
#define FAIL_LEVEL NUM_OF_LEVEL_SOUND_TRACKS+17

#define NUM_CHANNELS NUM_OF_LEVEL_SOUND_TRACKS+18

extern int Global_quit;
extern int mm;
extern int pause;
extern int pauseOption;
extern int playing;

extern SDL_Event event;
extern Mix_Chunk *sounds[NUM_CHANNELS];

extern SDL_Color bg_color;
extern SDL_Color borderColor;
extern SDL_Color white;
extern SDL_Color yellow;
