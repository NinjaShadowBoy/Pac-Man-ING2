#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef struct button {
    /*
    A button is a piece of text displayed on the screen using in a rect
    */
    char text[100];         // Text on the button
    char *fontDirectory;    // Directoty of the font used for the button
    SDL_Color color;        // Color of the button
    SDL_Renderer *renderer; // Renderer of the button
    SDL_Rect rect;          // Rect of the button
    SDL_Texture *texture;   // Botton texture (image of the button)
    int fontSize;           // Font size used when creating button image
} button;

void createButtonTexture(button *b);
button newButton(SDL_Renderer *ren, char *fontDirectory, int fontSize, SDL_Color txtColor, char *txt);
void setButtonText(button *b, char *txt, int fontSize);
int drawButton(button b);
void printScore(button digits[20], int num, int x, int y);

void createButtonTexture(button *b) {
    /*
    This function is responsible for loading a font file and creating a texture for the button passed as parameter
    */

    TTF_Font *font = TTF_OpenFont(b->fontDirectory, b->fontSize); // open font file with specific fontSize
    if (font == NULL) {
        // Error handling
        // printf("\nError opening font file at %s", b->fontDirectory);
    }
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, b->text, b->color); // Create  surface
    b->texture = SDL_CreateTextureFromSurface(b->renderer, surface);        // Create texture
    SDL_FreeSurface(surface);                                               // We do not need the surface anymore
    SDL_QueryTexture(b->texture, NULL, NULL, &b->rect.w, &b->rect.h);
    TTF_CloseFont(font); // Close the font file
}

button newButton(SDL_Renderer *ren, char *fontDirectory, int fontSize, SDL_Color txtColor, char *txt) {
    /*
    Creates a new button and ruturns it
    */
    button b;
    b.renderer = ren;                // Set renderer
    b.rect.x = b.rect.y = 0;         // Position of rect
    b.color = txtColor;              // Set text color
    b.fontSize = fontSize;           // Set font size
    b.fontDirectory = fontDirectory; // Directory of the font file
    strcpy(b.text, txt);             // Text on the button
    createButtonTexture(&b);         // Create a texture for the button
    return b;
}

int drawButton(button b) {
    // Draw button to its screen
    return SDL_RenderCopy(b.renderer, b.texture, NULL, &b.rect);
}

void setButtonText(button *b, char *txt, int fontSize) {
    /*
    Modify the text and/or font size on the button
    */
    strcpy(b->text, txt);
    ;
    b->fontSize = fontSize;
    createButtonTexture(b);
}

void showScoreAnimation(SDL_Renderer* ren, float score, float multiplier, int x, int y, int soundChannel, Mix_Chunk* coin) {
    SDL_Rect ScreenRect = {.x = x, .y = y, . w = 600, .h = 200};
    SDL_Color txtcol = {255, 255, 25, 0};
    SDL_Color white = {255, 255, 255, 0};
    button digits[20];
    char txt[10];
    for (int i = 0; i < 20; i++) {
        sprintf(txt, "%d", i);
        digits[i] = newButton(ren, "fonts/joystix monospace.otf", 60, txtcol, txt);
    }

    float tempScore = 0;
    button S = newButton(ren, "fonts/joystix monospace.otf", 50, white, "Score: ");
    char word[10] = "";
    sprintf(word, "x%.1f", multiplier);
    button M = newButton(ren, "fonts/joystix monospace.otf", 50, white, word);
    S.rect.x = ScreenRect.x + 4;
    S.rect.y = ScreenRect.y + 4;
    M.rect.x = ScreenRect.x + digits[0].rect.w * 3.5;
    M.rect.y = ScreenRect.y + digits[0].rect.h * 0.8;
    SDL_Event e;
    int del = 80, delay = 500;
    int ds = 5;
    SDL_bool run = SDL_TRUE;
    clock_t t = clock();
    Mix_PlayChannel(soundChannel, coin, 0);

    while (run) {
        tempScore += ds;
        if (clock() - t > del) {
            Mix_PlayChannel(soundChannel, coin, 0);
            t = clock();
        }
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = SDL_FALSE;
            }
            if (e.type == SDL_KEYDOWN) {
                ds += 5;
                delay -= 100;
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0 );
        SDL_RenderFillRect(ren, &ScreenRect);

        if (tempScore >= score) {
            tempScore = score;
        }

        drawButton(S);
        printScore(digits, tempScore, S.rect.w + S.rect.x, S.rect.y);

        SDL_RenderPresent(ren);
        if (tempScore == score) {
            SDL_Delay(300);
            drawButton(M);
            SDL_RenderPresent(ren);
            SDL_Delay(delay);
            run = SDL_FALSE;
        }
    }
    delay = 1500;
    run = SDL_TRUE;
    while (run) {
        tempScore += ds;
        if (clock() - t > del) {
            Mix_PlayChannel(soundChannel, coin, 0);
            t = clock();
        }
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                run = SDL_FALSE;
            }
            if (e.type == SDL_KEYDOWN) {
                del -= 10;
                ds += 5;
            }
        }
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0 );
        SDL_RenderFillRect(ren, &ScreenRect);

        if (tempScore >= score * multiplier) {
            tempScore = score * multiplier;
            run = SDL_FALSE;
        }

        drawButton(S);
        printScore(digits, score, S.rect.w + S.rect.x, S.rect.y);
        drawButton(M);
        printScore(digits, tempScore, S.rect.w + S.rect.x, S.rect.y + digits[0].rect.h * 1.25);

        SDL_RenderPresent(ren);
        if (!run) {
            tempScore = 0;
            SDL_Delay(delay);
            SDL_WaitEvent(&e);
        }
    }
}

typedef struct record {
    char name[20];
    char mode[20];
    int lvl;
    int score;
} record;

void sortRecords(record** r, int count) {
    int sc, lvl;
    for (int i = 0; i < count; i++) {
        for (int j = i; j < count; j++) {
            if (r[i]->score < r[j]->score) {
                char *md, *name;
                // Swap score
                sc = r[i]->score;
                r[i]->score = r[j]->score;
                r[j]->score = sc;

                // Swap lvl
                lvl = r[i]->lvl;
                r[i]->lvl = r[j]->lvl;
                r[j]->lvl = lvl;

                // Swap name
                sprintf(name, r[i]->name);
                sprintf(r[i]->name, r[j]->name);
                sprintf(r[j]->name, name);

                // Swap modes
                sprintf(md, r[i]->mode);
                sprintf(r[i]->mode, r[j]->mode);
                sprintf(r[j]->mode, md);
            }
        }
    }
}

void writeRecordToFile(record rcrd, char* filename) {
    FILE* fptr = fopen(filename, "a");
    fprintf(fptr, "%s,%s,%d,%d\n", rcrd.name, rcrd.mode, rcrd.lvl, rcrd.score);
    fclose(fptr);
}

button* LoadRecordsIntoList(SDL_Renderer* ren, char* filename, int x, int y, int* numLoaded) {
    record* examples;
    examples = (record*)malloc(50 * sizeof(record));
    FILE* fptr = fopen(filename, "r");
    int count = 0;
    while (!feof(fptr) && count < 50) {
        fscanf(fptr, "%50[^,],%10[^,],%d,%d\n", examples[count].name, examples[count].mode, &examples[count].lvl, &examples[count].score);
        printf("%d) %10s,%s,%d,%d\n", count, examples[count].name, examples[count].mode, examples[count].lvl, examples[count].score);
        count++;
    }
    sortRecords(&examples, count);

    SDL_Color white = {230, 230, 230, 0};
    SDL_Color grey = {230, 230, 150, 0};

    *numLoaded = count;
    button* b = (button*)malloc(count * sizeof(button));
    for (int i = 0; i < count; i++) {
        char txt[100] = "";
        sprintf(txt, "%2d> %-20s%-10s%-4d  %d", i + 1, examples[i].name, examples[i].mode, examples[i].lvl + 1, examples[i].score);
        printf("%s\n", txt);
        if (i % 2) {
            b[i] = newButton(ren, "fonts/joystix monospace.otf", 30, white, txt);
        } else {
            b[i] = newButton(ren, "fonts/joystix monospace.otf", 30, grey, txt);
        }
        b[i].rect.x = x;
        b[i].rect.y = y + i * b[i].rect.h;
    }
    fclose(fptr);

    return b;
}

char* who_are_you(SDL_Renderer* ren, int x, int y, int typeChannel, Mix_Chunk* typeSound) {
    SDL_Rect ScreenRect = {.x = x - 100, .y = y, . w = 5000, .h = 200};
    SDL_Color white = {255, 255, 255, 0};
    int ptsize = 55;

    SDL_Event event;
    int isrunning = 1;
    char word[20] = "";
    char* word1 = (char*)malloc(20 * sizeof(char));
    button text;
    button arrow = newButton(ren, "fonts/pricedown bl.otf", ptsize, white, "Who are you? -> ");
    arrow.rect.x = x;
    arrow.rect.y = ScreenRect.y + ScreenRect.h - arrow.rect.w / 2.5;

    button carret = newButton(ren, "fonts/pricedown bl.otf", ptsize, white, "|");
    int showCarret = 0;
    clock_t timer = clock();
    while (isrunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isrunning = 0;
                break;
            }

            if (event.type == SDL_TEXTINPUT && strlen(word) < 20) {
                strcat(word, event.text.text);
                Mix_PlayChannel(typeChannel, typeSound, 0);
            }

            if (event.type == SDL_KEYUP) {
                if (strlen(word) > 0) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        word[strlen(word) - 1] = '\0';
                    }
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        isrunning = 0;;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 20, 0);
        SDL_RenderFillRect(ren, &ScreenRect);

        drawButton(arrow);
        text = newButton(ren, "fonts/pricedown bl.otf", ptsize, white, word);
        text.rect.x = arrow.rect.x + arrow.rect.w;
        text.rect.y = arrow.rect.y;
        drawButton(text);

        if (clock() - timer > 300) {
            showCarret = 1 - showCarret;
            timer = clock();
        }
        if (showCarret) {
            carret.rect.x = text.rect.x + text.rect.w;
            carret.rect.y = text.rect.y;
            drawButton(carret);
        }

        SDL_RenderPresent(ren);
    }

    strcpy(word1, word);
    return word1;
}
