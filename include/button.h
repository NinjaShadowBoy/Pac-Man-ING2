#pragma once

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef struct button {
    char text[100];
    char *fontDirectory;
    SDL_Color color;
    SDL_Renderer *renderer;
    SDL_Rect rect;
    SDL_Texture *texture;
    int fontSize;
} button;

typedef struct record {
    char name[20];
    char mode[20];
    int lvl;
    int score;
} record;

void createButtonTexture(button *b);
button newButton(SDL_Renderer *ren, char *fontDirectory, int fontSize, SDL_Color txtColor, char *txt);
void setButtonText(button *b, char *txt, int fontSize);
int drawButton(button b);
void printScore(button digits[20], int num, int x, int y);
void showScoreAnimation(SDL_Renderer* ren, float score, float multiplier, int x, int y, int soundChannel, Mix_Chunk* coin);
void sortRecords(record** r, int count);
void writeRecordToFile(record rcrd, char* filename);
button* LoadRecordsIntoList(SDL_Renderer* ren, char* filename, int x, int y, int* numLoaded);
char* who_are_you(SDL_Renderer* ren, int x, int y, int typeChannel, Mix_Chunk* typeSound);
