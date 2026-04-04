#pragma once

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <stdlib.h>

#include "global_variables.h"
#include "pac.h"
#include "button.h"

typedef struct menu {
    char **menutext;
    char *menuTitle;
    int numOptions;
    int mx;
    int my;
    int indent;
    int fontSize;
    button **menuButtons;
    button menuTitleButton;
    int bg_Channel;
    int click_channel;
    int choice;
    int isrunning;
    Mix_Chunk *bgSound;
    Mix_Chunk *clickSound;
    SDL_Renderer *renderer;
    SDL_Color seleColor;
    SDL_Color idleColor;
} menu;

menu newMenu(SDL_Renderer *ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk *bg_sound, int click_channel,
             Mix_Chunk *clickSound);
void checkMenuEvents(menu* m);
int showMenu(menu m);
