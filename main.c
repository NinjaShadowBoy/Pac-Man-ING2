#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pac.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <string.h>


typedef struct button {
    /*
    A button is a piece of text displayed on the screen using in a rect
    */
    char text[100]; // Text on the button
    char* fontDirectory; // Directoty of the font used for the button
    SDL_Color color; // Color of the button
    SDL_Renderer* renderer; // Renderer of the button
    SDL_Rect rect; // Rect of the button
    SDL_Texture* texture; // Botton texture (image of the button)
    int fontSize; // Font size used when creating button image
} button;

typedef struct menu {
    /*
    A menu is a set of buttons:
    	- The title button
    	- The menu options' buttons
    Among those buttons, one is highlighted
    It also has a back ground music and a click sound (A sound to be played when a key is pressed while in menu)
    */
    char **menutext; // Set of strings of the menu options
    char *menuTitle; // Text of menu title
    int numOptions; // NUmber of options in a menu
    int mx; // x coordinate of the tile button
    int my; // y coordinate of the tile button
    int indent; // Indent of the menu optionn's buttons with respect to the menu title button
    int fontSize; // Font size of the menu options' buttons
    button* menuButtons; // Menu options buttons
    button menuTitleButton; // Menu title button
    int bg_Channel; // Channel of the bg music
    int click_channel; // Channel of the click sound
    Mix_Chunk* bgSound; // Back ground music
    Mix_Chunk* clickSound; // Click sound effect
    SDL_Renderer* renderer; // Renderer
    SDL_Color seleColor; // Highlighted button's and menu title's font color
    SDL_Color idleColor; // Other buttons' font color
} menu;

void createButtonTexture(button* b);
button newButton(SDL_Renderer* ren, char* fontDirectory, int fontSize, SDL_Color txtColor, char* txt);
int drawButton(button b);
void setButtonText(button* b, char* txt, int fontSize);
menu newMenu(SDL_Renderer* ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk* bg_sound, int click_channel,
             Mix_Chunk* clickSound);
int showMenu(menu m);
void printScore(button digits[20], int num, int x, int y);
void checkEvents(int numGhosts);
int initSDL();
/////////////////////////////
//     Global variables    //
/////////////////////////////

// Window and renderer
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;


// Window height and width
#define SCREEN_WIDTH 1920/1.9
#define SCREEN_HEIGHT 1080/1.2

// Different (track) channels
#define STAGE_MUSIC 0
#define BG1 1
#define EAT 2
#define SPECIAL_FOOD 3
#define HURT 4
#define GHOST_EATEN 5
#define DIE 6
#define CLICK 7
#define MAIN_MENU 8
#define WIN_LEVEL 9

#define NUM_CHANNELS 10 // Total number of sound tracks (channels)


// Pointer for an array of ghosts
Ghost* g;
// Create pacman
pac p;

int quit = 0;
int pause = 0;

// Variable to store keyboard events
SDL_Event event;

// A plane which will to transformed to a maze
plane plan;

// Sounds
Mix_Chunk* sounds[NUM_CHANNELS];

// Score buttons
button score[2];

button digits[20];

//    end/Global variables
/////////////////////////////

int playLevel(int level, int gameMode) {
    int numGhosts;
    if (gameMode == 0) {
        initPac(&p, 50, window, renderer, level);
        numGhosts = 4;
    } else {
        initPac(&p, 40, window, renderer, level);
        numGhosts = 4 + (level) / 10;
    }


    g = (Ghost*)malloc(numGhosts * sizeof(Ghost));
    // Manage errors while creating ghosts
    if (initGhost(g, (1 + level * 0.01)*p.size, window, renderer, numGhosts, level) == 0) {
        // // // printf("\nGhosts initialized\n");
    } else {
        // printf("Error Ghost");
    }
    Mix_PlayChannel(BG1, sounds[BG1], -1);

    DrawPlane(plan, 0);
    // Set pac position to top-left square in grid
    p.pos = plan.self[plan.sizeX / 2 - 1][plan.sizeY - 3];
    p.x = p.pos.rect.x + 5;
    p.y = p.pos.rect.y + 5;
    for (int i = 0; i < 5; i++) {
        p.pos.foods[i] = 0;
    }

    // Set ghost position in grid
    double gspeed;
    for (int i = 0; i < numGhosts; i++) {
        drawGhost(&g[i]);
        g[i].pos = plan.self[(int)((plan.sizeX) / 2 - (i % 4) % 2)][(int)((plan.sizeY / 2))];
        g[i].x = g[i].pos.rect.x + g[i].pos.rect.w / 2 - g[i].rect.w / 2 + (i % 4 - 1) * g[i].size;
        g[i].y = g[i].pos.rect.y + g[i].pos.rect.h / 2 - g[i].rect.h / 2;
        if (level > 7) {
            gspeed = p.speed;
        } else {
            gspeed = (0.7 + level * 0.04) * p.speed;
        }
        g[i].speed = gspeed;
        g[i].eatenSpeed = g[i].speed * 1.2;
        g[i].eatenSize = g[i].size / 2;
        g[i].frightSpeed = g[i].speed * 0.8;
        g[i].normalSize = g[i].size;
        g[i].normalSpeed = g[i].speed;
        if (i % 2) {
            g[i].immediateDest.i = g[i].pos.index.i + 1;
            g[i].immediateDest.j = g[i].pos.index.j;
        } else {
            g[i].immediateDest.i = g[i].pos.index.i - 1;
            g[i].immediateDest.j = g[i].pos.index.j;
        }
    }

    // Configure rects for health bar
    SDL_Rect redHealthBar;
    redHealthBar.x = plan.right;
    redHealthBar.y = plan.top;
    redHealthBar.w = (p.size - p.minSize) * 6;
    redHealthBar.h = p.size - p.minSize;
    SDL_Rect greenHealthBar = redHealthBar;
    SDL_Rect blackHealthBar = redHealthBar;
    blackHealthBar.x -= 5;
    blackHealthBar.w += 10;
    blackHealthBar.y -= 5;
    blackHealthBar.h += 10;


    // Score buttons
    sprintf(score[1].text, "%d", p.score);
    score[0] = newButton(renderer, "fonts/joystix monospace.otf", 30, (SDL_Color) {
        255, 255, 255, 0
    }, "Score");
    score[0].rect.x = plan.right;
    score[0].rect.y += blackHealthBar.y + blackHealthBar.h + 10;
    score[1] = newButton(renderer, "fonts/joystix monospace.otf", 30, (SDL_Color) {
        255, 255, 255, 0
    }, score[1].text);
    score[1].rect.x = plan.right;
    score[1].rect.y = score[0].rect.y + score[0].rect.h + 10;

    button p200 = newButton(renderer, "fonts/joystix monospace.otf", 15, (SDL_Color) {
        0, 255, 255, 0
    }, "200");


    // Main Game loop
    int start = 2;
    clock_t changeMaze = clock();
    clock_t wintime = 0;
    plane planCopy;
    int changeMazePeriod = 10000 - 500 * level;
    // Create another plane of identical size to the game
    planCopy = InitializePlane(plan.sizeX, plan.sizeY, plan.top, plan.left, plan.cellSize, plan.cellSpacing, plan.cellCol, plan.borderCol, window, renderer);
    while (!quit && (wintime == 0 || clock() - wintime < 2000)) {
        checkEvents(numGhosts);
        if (gameMode == 2 && clock() - changeMaze > changeMazePeriod) { // Change maze every 6 seconds
            planCopy.isColorful = plan.isColorful;
            for (int i = 0; i < plan.sizeX; i++) { // Copy food from plan to planCopy
                for (int j = 0; j < plan.sizeY; j++) {
                    for (int k = 0; k < 5; k++) {
                        planCopy.self[i][j].foods[k] = plan.self[i][j].foods[k];
                    }
                }
            }
            planCopy.numFoods = plan.numFoods;

            plan = InitializePlane(plan.sizeX, plan.sizeY, plan.top, plan.left, plan.cellSize, plan.cellSpacing, plan.cellCol, plan.borderCol, window, renderer);
            MazifyPlane(&plan, 0);

            plan.isColorful = planCopy.isColorful;
            for (int i = 0; i < plan.sizeX; i++) { // Copy food from planCopy to plan
                for (int j = 0; j < plan.sizeY; j++) {
                    for (int k = 0; k < 5; k++) {
                        plan.self[i][j].foods[k] = planCopy.self[i][j].foods[k];
                    }
                }
            }
            plan.numFoods = planCopy.numFoods;
            changeMaze = clock();
            p.pos = plan.self[p.pos.index.i][p.pos.index.j];
        }

        // Set drawing color color
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        // Clear the renderer
        SDL_RenderClear(renderer);

        if ((wintime == 0 || clock() - wintime < 1000) && !pause) {
            updatePacPos(&p, plan);
        }

        // Draw health bar
        if (greenHealthBar.w > (p.size - p.minSize) * 6) {
            greenHealthBar.w -= 2;
            if (p.size - p.minSize < 1) {
                greenHealthBar.w = 0;
            }
        } else if (greenHealthBar.w < (p.size - p.minSize) * 6) {
            greenHealthBar.w++;
        }

        // Show score
        drawButton(score[0]);
        printScore(digits, p.score, score[1].rect.x, score[1].rect.y);
//		SDL_DestroyTexture(score[1].texture);

        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 0);
        SDL_RenderFillRect(renderer, &blackHealthBar);
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 0);
        SDL_RenderFillRect(renderer, &redHealthBar);
        SDL_SetRenderDrawColor(renderer, ((p.maxSize - p.size) / (p.maxSize - p.minSize)) * 255, 250, 0, 0);
        SDL_RenderFillRect(renderer, &greenHealthBar);


        // Draw maze
        SDL_SetRenderDrawColor(renderer, 200, 10, 10, 10);
        DrawPlane(plan, 1);
        // Draw the character textures
        drawPac(&p);
        for (int i = 0; i < numGhosts; i++) {
            drawGhost(&g[i]);
        }

        if (!pause) {
            // Ghost operations
            if (wintime == 0) {
                changeGhostDestination(g, plan, &p, numGhosts);
                for (int i = 0; i < numGhosts; i++) {
                    updateGhostPos(&g[i], plan, g[i].immediateDest);
                }
            }
        }

        // To draw do several iterations of drawing p200 and delay after eaten ghost
        if (p.hasEatenGhost == 30) {
            drawButton(p200);
            p.hasEatenGhost = 0;
        } else if (p.hasEatenGhost != 0) {
            p200.rect.x = p.rect.x + p.rect.w / 2 - p200.rect.w / 2;
            p200.rect.y = p.rect.y - p200.rect.h;
            drawButton(p200);
            if (p.hasEatenGhost == 5) {
                SDL_Delay(300);
            }
            p.hasEatenGhost++;
        }

        // Set drawing color color,
        SDL_SetRenderDrawColor(renderer, 10, 10, 200, 10);

        // Present the renderer
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
        SDL_RenderDrawRect(renderer, &g[0].target.rect);
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
        SDL_RenderDrawRect(renderer, &g[1].target.rect);
        SDL_SetRenderDrawColor(renderer, 50, 50, 255, 0);
        SDL_RenderDrawRect(renderer, &g[2].target.rect);
        SDL_SetRenderDrawColor(renderer, 255, 250, 0, 0);
        SDL_RenderDrawRect(renderer, &g[3].target.rect);
        SDL_RenderDrawRect(renderer, &p.pos.rect);
        int x1 = g[0].pos.x;
        int x2 = g[2].target.x;
        int y1 = g[0].pos.y;
        int y2 = g[2].target.y;
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

        if (plan.numFoods == 0 && wintime == 0) {
            Mix_PlayChannel(WIN_LEVEL, sounds[WIN_LEVEL], 0);
            Mix_HaltChannel(BG1);
            Mix_HaltChannel(STAGE_MUSIC);
            wintime = clock();
        }
        SDL_RenderPresent(renderer);
        if (start) {
            if (start == 1) {
                SDL_Delay(1500);
            }
            start--;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    initSDL();
    Sleep(500);

    // This is the main manu text
    char** menutxt = (char**)malloc(10 * sizeof(char*));
    menutxt[0] = "Classic";
    menutxt[1] = "Random Maze";
    menutxt[2] = "Crazy Maze!?";
    menutxt[3] = "Leader Boards";
    menutxt[4] = "Quit";
    menutxt[5] = "!? PAC MAN ?!"; // Title of the menu with menu options just not to create another variable

    int menuNumOptions = 5;

    char** subMenutxt = (char**)malloc(16 * sizeof(char*));

    // Maze colors
    SDL_Color bg_color = {0, 0, 10, 0};
    SDL_Color borderColor = {0, 0, 200, 0};

    // Create Main menu
    SDL_Color highlight_color = {.r = 255, .g = 240, .b = 0, .a = 0};
    SDL_Color menu_text_color = {.r = 50, .g = 50, .b = 100, .a = 0	};
    menu MainMenu = newMenu(renderer, menutxt[menuNumOptions], menutxt, menuNumOptions, 50, 100, 20, 25, highlight_color, menu_text_color, MAIN_MENU, sounds[MAIN_MENU], CLICK, sounds[CLICK]);

    // Main menu
    int choice; // Stores what choice has been made in main menu
    int level = 1; // Stores what level was choosed in sub-menus (Classic, Random Maze, Crazy Maze)
begin:
    initPac(&p, 50, window, renderer, 1);

    choice = showMenu(MainMenu); // Which choice has been made. It also repreesents the game mode
    quit = 0; // The game will start

    // Handle the choices made in the menu
    int numLevels = 14; // Load the number of unlocked levels from a file
    if (choice == 0) { // If classic was choosed
        for (int i = 0; i < numLevels; i++) { // Create text for all levels
            subMenutxt[i] = (char*)malloc(20 * sizeof(char));
            sprintf(subMenutxt[i], "Level %d", i + 1);
        }
        subMenutxt[numLevels] = "Back"; // At last position for the options

        // Create a menu for the text created above
        menu subMenu = newMenu(renderer, "Classic", subMenutxt, numLevels + 1, 40, 20, 140, 100, highlight_color, menu_text_color, MAIN_MENU, sounds[MAIN_MENU], CLICK, sounds[CLICK]);
        level = showMenu(subMenu); // Show the classic menu
        if (level == numLevels) { // If back has been chosed
            goto begin; // Goto main menu
        }
        Mix_PlayChannel(STAGE_MUSIC, sounds[STAGE_MUSIC], -1);
        plan = InitializePlane(10, 11, p.size, p.size, p.size * 1.2, p.size * 0.5, bg_color, borderColor, window, renderer);
        ClassicMaze(&plan);
    } else if (choice == 1) { // If random maze was choosed
        for (int i = 0; i < numLevels; i++) { // Create text for all levels
            subMenutxt[i] = (char*)malloc(20 * sizeof(char));
            sprintf(subMenutxt[i], "Level %d", i + 1);
        }
        subMenutxt[numLevels] = "Back"; // At last position for the options

        // Create a menu for the text created above
        menu subMenu = newMenu(renderer, "Random Maze", subMenutxt, numLevels + 1, 40, 20, 140, 100, highlight_color, menu_text_color, MAIN_MENU, sounds[MAIN_MENU], CLICK, sounds[CLICK]);
        level = showMenu(subMenu); // Show the classic menu
        if (level == numLevels) { // If back has been chosed
            goto begin; // Goto main menu
        }
        Mix_PlayChannel(STAGE_MUSIC, sounds[STAGE_MUSIC], -1);

        initPac(&p, 35, window, renderer, level);
        plan = InitializePlane(15 + rand() % 3, 13 + rand() % 3, p.size, p.size, p.size * 1.2, p.size * 0.5, bg_color, borderColor, window, renderer);
        MazifyPlane(&plan, 1);
    } else if (choice == 2) { // If crazy maze was choosed
        for (int i = 0; i < numLevels; i++) { // Create text for all levels
            subMenutxt[i] = (char*)malloc(20 * sizeof(char));
            sprintf(subMenutxt[i], "Level %d", i + 1);
        }
        subMenutxt[numLevels] = "Back"; // At last position for the options

        // Create a menu for the text created above
        menu subMenu = newMenu(renderer, "CRaZyNeSs", subMenutxt, numLevels + 1, 40, 20, 140, 100, highlight_color, menu_text_color, MAIN_MENU, sounds[MAIN_MENU], CLICK, sounds[CLICK]);
        level = showMenu(subMenu); // Show the classic menu
        if (level == numLevels) { // If back has been chosed
            goto begin; // Goto main menu
        }
        Mix_PlayChannel(STAGE_MUSIC, sounds[STAGE_MUSIC], -1);

        initPac(&p, 35, window, renderer, level);
        plan = InitializePlane(15 + rand() % 3, 13 + rand() % 3, p.size, p.size, p.size * 1.2, p.size * 0.5, bg_color, borderColor, window, renderer);
        MazifyPlane(&plan, 1);
    } else if (choice == 4) { // If Quit was choosed
        quit = 1;
        goto finalEnd;
    }

    playLevel(level, choice);


    // Clean all sounds
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (i != DIE) {
            Mix_HaltChannel(i);
        }
    }
    SDL_Delay(1000);
    goto begin; // Goto main menu


    // Clean up and exit
    for (int i = 0; i < p.numOfFrames; i++) {
        if (p.Textures[i]) {
            SDL_DestroyTexture(p.Textures[i]);
        }
    }
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < g[j].numOfFrames; i++) {
            if (g[j].Textures[i]) {
                SDL_DestroyTexture(g[j].Textures[i]);
            }
        }
    }
//	SDL_DestroyTexture(plan.foodTexture);
//	Mix_Chunk* stageMusic;
//	Mix_Chunk* eat;
//	Mix_Chunk* hurt;
//	Mix_Chunk* die;
//	Mix_Chunk* ghostEaten;
//	Mix_Chunk* bg1;
//	Mix_Chunk* specialFood;
finalEnd:
    free(g);
    for (int i = 0; i < NUM_CHANNELS; i++) {
        Mix_FreeChunk(sounds[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    system("delete");

    if (argc || argv) {} // Remove annoying warning
    return 0;
}


void checkEvents(int numGhosts) {
    /*
    This funtion:
    	 - Manages keyboard events
    	 - Manages most interactions with between Pac Man and Ghosts when they collide (Other functionalities are in the ghost functions)
    	 - Updates the cell which represents the position of Pac Man in the game maze
    	 - Manages interactions with between Pac Man and food
    */

    while (SDL_PollEvent(&event)) { // Manage keyboard events
        if (event.type == SDL_QUIT) {
            quit = 1;
            break;
        }
        // Handle arrow key releases
//        if (event.type == SDL_KEYUP) {
//            if (event.key.keysym.sym == SDLK_LEFT) {
//                p.movingLeft = 0;
//            }
//            if (event.key.keysym.sym == SDLK_RIGHT) {
//                p.movingRight = 0;
//            }
//            if (event.key.keysym.sym == SDLK_UP) {
//                p.movingUp = 0;
//            }
//            if (event.key.keysym.sym == SDLK_DOWN) {
//                p.movingDown = 0;
//            }
//        }

        // Handle arrow key presses
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_LEFT) {
                p.movingLeft = 1;
                p.movingRight = 0;
                p.movingUp = 0;
                p.movingDown = 0;
                p.direction = 3;
            } else if (event.key.keysym.sym == SDLK_RIGHT) {
                p.movingLeft = 0;
                p.movingRight = 1;
                p.movingUp = 0;
                p.movingDown = 0;
                p.direction = 1;
            } else if (event.key.keysym.sym == SDLK_UP) {
                p.movingLeft = 0;
                p.movingRight = 0;
                p.movingUp = 1;
                p.movingDown = 0;
                p.direction = 0;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                p.movingLeft = 0;
                p.movingRight = 0;
                p.movingUp = 0;
                p.movingDown = 1;
                p.direction = 2;
            }
            if (event.key.keysym.sym == SDLK_SPACE) {
                pause = pause == 1 ? 0 : 1;
            }
        }
    }

    SDL_Rect inter;  // These rects will be used to store intersections
    SDL_Rect inter1; // between some other rects
    SDL_IntersectRect(&p.rect, &p.pos.rect, &inter1);
    // Update pac position cell
    for (int i = 0; i < plan.sizeX; i++) {
        for (int j = 0; j < plan.sizeY; j++) {
            // Go through the maze and find the cell which intersects the most with Pac Man
            // That cell will be set as Pac Man's poition

            if (abs(p.pos.index.i - i) < 2 && abs(p.pos.index.j - j) < 2) {
                for (int k = 0; k < 5; k++) { // Manage interactions between the 5 foods which may be on a cell
                    if ((plan.self[i][j].foods[k] != 0 && SDL_HasIntersection(&p.rect, &plan.self[i][j].foodrects[k]))) {
                        int value;
                        plan.numFoods--;
                        value = plan.self[i][j].foods[k]; // Value of the food
                        p.score += value * 10;
                        plan.self[i][j].foods[k] = 0; // Say there is no more food in the cell

                        if (p.size < p.maxSize) { // Prevents the size from exceeding the maximun size
                            p.size += value * (p.deltaSize / 10);
                        }
                        if (value == 1) { // Food is normal
                            Mix_PlayChannel(EAT, sounds[EAT], 0);
                        }
                        if (value == 2) { // Food is special
                            Mix_PlayChannel(SPECIAL_FOOD, sounds[SPECIAL_FOOD], 0);
                            p.pow = 1; // Set Pac Man to be Powerfull! He can now eat the ghosts
                            p.startOfPower = clock();
                            p.size += (p.maxSize - p.minSize) / 1.75;
                            // Compensate for change in size
                            p.x -= (p.maxSize - p.minSize) / 2;
                            p.y -= (p.maxSize - p.minSize) / 2;
                            if (p.size > (p.maxSize) * 1.5) {
                                p.size = (p.maxSize) * 1.5;
                            }
                            plan.isColorful = 1;
                            for (int i = 0; i < numGhosts; i++) {
                                if (g[i].state != 3) {
                                    g[i].state = 2; // Change state to frightened if not eaten or already frightened
                                    g[i].startOfFright = clock();
                                    g[i].speed = g[i].frightSpeed;
                                    reverseDirection(&g[i], plan);
                                }
                            }
                        }
                    }
                }
            }

            if (p.pow == 0 && p.size > p.maxSize) {
                p.size -= 0.001;
                // Compensate for change in size
                p.x += 0.001 / 2;
                p.y += 0.001 / 2;
            }

            // Stop plan colorful before end of fright (end of power up)
            if (p.pow && clock() - g[0].startOfFright > g[0].frightTime - 1000) {
                plan.isColorful = 0;
                p.pow = 0;
                Mix_HaltChannel(SPECIAL_FOOD);
            }

            if (
                SDL_IntersectRect(&p.rect, &plan.self[i][j].rect, &inter)
                && (inter.w * inter.h) > (inter1.w * inter1.h)
                // If pac intersects with another rect more than its position rect
                // set a that rect as new position
            ) {
                p.pos = plan.self[i][j];
                // printf("%d %d\n", i, j);
                goto end;
            }
        }
    }
end:
    // Handling collisions between Pac man and ghosts
    for (int i = 0; i < numGhosts; i++) {
        if (SDL_IntersectRect(&p.rect, &g[i].rect, &inter)) { // If Pac Man and ghost intersect
            if (g[i].state == 2) {// If in frightened mode get into eaten mode
                Mix_PlayChannel(GHOST_EATEN, sounds[GHOST_EATEN], 0);
                g[i].state = 3;
                g[i].speed = g[i].eatenSpeed;
                p.score += 200;
                p.hasEatenGhost = 1;
                // printf("Score %d", p.score); // update score button
//				g[i].size = g[i].eatenSize;

                // Ajust position of ghost to compensate for reduction in size
//                g[i].x = g[i].pos.rect.x + g[i].pos.rect.w / 2 - g[i].size / 2;
//                g[i].y = g[i].pos.rect.y + g[i].pos.rect.h / 2 - g[i].size * 1.2 / 2;
                continue;
            }
            if (g[i].state == 0 || g[i].state == 1) { // If in scatter or chase mode
                float intersection_area = inter.w * inter.h;
                float PacMan_area = p.rect.w * p.rect.h;
                // Check if PacMan and ghost intersect enough and that the ghost is not just from bitting PacMan (Iterations occur really fast you know)
                if (intersection_area > PacMan_area / 4 && clock() - g[i].lastTimeHit > (g[i].size / g[i].speed) * 1000) {
                    p.size -= p.deltaSize; // Reduce Pac Man's size by deltasize
                    p.score -= 50;
                    if (p.score < 0) p.score = 0;

                    // printf("Score %d", p.score); // update score button
                    // Ajust positon to compensate for change in size
                    p.x += p.deltaSize / 2;
                    p.y += p.deltaSize / 2;
                    Mix_PlayChannel(HURT, sounds[HURT], 0); // Play hurt sound

                    if (p.size < p.minSize) { // If size Pac Man is too small he dies
                        Mix_PlayChannel(DIE, sounds[DIE], 0); // Play die sound
                        quit = 1;
                    }
                    g[i].lastTimeHit = clock(); // Set that it is now that this ghost last bit Pac Man
                }
            }
        }
    }
}



int initSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { // If SDL_Init did not return 0 (i.e. it failed)
        // printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }
    // printf("SDL initialized\n");


    // Create a window
    window = SDL_CreateWindow("Pac test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) { // If window has not been created
        // printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { // If renderer has not been created
        // printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_Mixer
    TTF_Init();
    Mix_Init(0);
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
    Mix_AllocateChannels(NUM_CHANNELS);
    // Load sound
    sounds[STAGE_MUSIC] = Mix_LoadWAV("sounds/Demon_Slayer.mp3");
    Mix_VolumeChunk(sounds[STAGE_MUSIC], 30);
    sounds[EAT] = Mix_LoadWAV("sounds/Chomp High2.mp3");
    Mix_VolumeChunk(sounds[EAT], 25);
    sounds[BG1] = Mix_LoadWAV("sounds/Background 1.mp3");
    Mix_VolumeChunk(sounds[BG1], 35);
    sounds[HURT] = Mix_LoadWAV("sounds/Jump117.wav");
    Mix_VolumeChunk(sounds[HURT], 50);
    sounds[SPECIAL_FOOD] = Mix_LoadWAV("sounds/The super saiyan aura Sound and video.mp3");
    Mix_VolumeChunk(sounds[SPECIAL_FOOD], 120);
    sounds[DIE] = Mix_LoadWAV("sounds/Death.mp3");
    Mix_VolumeChunk(sounds[DIE], 80);
    sounds[GHOST_EATEN] = Mix_LoadWAV("sounds/Tagne.mp3");
    Mix_VolumeChunk(sounds[GHOST_EATEN], 127);
    sounds[CLICK] = sounds[EAT];
    sounds[MAIN_MENU] = Mix_LoadWAV("sounds/Nicky_Larson.mp3");
    Mix_VolumeChunk(sounds[MAIN_MENU], 80);
    sounds[WIN_LEVEL] = Mix_LoadWAV("sounds/mixkit-game-bonus-reached-2065.wav");


    SDL_Color txtcol = {255, 255, 25, 0};
    char txt[10];
    for (int i = 0; i < 20; i++) {
        sprintf(txt, "%d", i);
        digits[i] = newButton(renderer, "fonts/joystix monospace.otf", 40, txtcol, txt);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderPresent(renderer);
    // printf("Initializing ...\n");
    return 0;
}


void createButtonTexture(button* b) {
    /*
    This function is responsible for loading a font file and creating a texture for the button passed as parameter
    */

    TTF_Font* font = TTF_OpenFont(b->fontDirectory, b->fontSize); // open font file with specific fontSize
    if (font == NULL) { // Error handling
        // printf("\nError opening font file at %s", b->fontDirectory);
    }
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, b->text, b->color); //Create  surface
    b->texture = SDL_CreateTextureFromSurface(b->renderer, surface); // Create texture
    SDL_FreeSurface(surface); // We do not need the surface anymore
    SDL_QueryTexture(b->texture, NULL, NULL, &b->rect.w, &b->rect.h);
    TTF_CloseFont(font); // Close the font file
}

button newButton(SDL_Renderer* ren, char* fontDirectory, int fontSize, SDL_Color txtColor, char* txt) {
    /*
    Creates a new button and ruturns it
    */
    button b;
    b.renderer = ren; // Set renderer
    b.rect.x = b.rect.y = 0; // Position of rect
    b.color = txtColor; // Set text color
    b.fontSize = fontSize; // Set font size
    b.fontDirectory = fontDirectory; // Directory of the font file
    strcpy(b.text, txt); // Text on the button
    createButtonTexture(&b); // Create a texture for the button
    return b;
}

int drawButton(button b) { // Draw button to its screen
    return SDL_RenderCopy(b.renderer, b.texture, NULL, &b.rect);
}

void setButtonText(button* b, char* txt, int fontSize) {
    /*
    Modify the text and/or font size on the button
    */
    strcpy(b->text, txt);;
    b->fontSize = fontSize;
    createButtonTexture(b);
}


SDL_Window* win; // Window pointer for our game
SDL_Renderer* ren; // Renderer pointer for our game


menu newMenu(SDL_Renderer* ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk* bg_sound, int click_channel,
             Mix_Chunk* clickSound) {
    /*
    Creates a new button and ruturns it
    */
    menu m;
    m.menutext = menutext; // Array of strings which are the menu options
    m.numOptions = numOptions; // Number of options
    m.mx = mx; // x position of the menu title button
    m.my = my; // y position of the menu title button
    m.bg_Channel = bg_channel; // Channel of the background music
    m.bgSound = bg_sound; // Back ground music to be playing while in menu
    m.click_channel = click_channel; // Channel of the click sound effect
    m.clickSound = clickSound; // Click sound of the menu
    m.renderer = ren; // Renderer on which to draw the buttons
    m.menuTitle = menuTitle; // Taxt for the menu title
    m.fontSize = fontSize; // Font size of the menu options (That of the title will be a multiple of it)
    m.indent = indent; // Indentation of the menu options with respect to the menu title
    m.seleColor = seleColor; // Color of the highlighted optionin the menu
    m.idleColor = idleColor; // Color of other options in the menu

    // Allocate mamory to store the menu options' buttons
    m.menuButtons = (button*)malloc(m.numOptions * sizeof(button));

    return m; // return the menu created
}


int showMenu(menu m) {
    /*
    This functions indefinitely displays the menu passed as parameter until a choice is made
    */
    int isrunning = 1;
    int choice = 0; // A number which represents the highlighted option in the menu
    // By default is first option
    SDL_Event menuEvent;
    void checkMenuEvents() { // Manage key presses in the menu
        while (SDL_PollEvent(&menuEvent)) {
            if (menuEvent.type == SDL_QUIT) {
                isrunning = 0;
                choice = m.numOptions - 1;
                break;
            }
            if (menuEvent.type == SDL_KEYDOWN) {
                if (menuEvent.key.keysym.sym == SDLK_DOWN) { // If up key is pressed
                    choice++;
                    if (choice == m.numOptions) {
                        choice = 0;
                    }
                }
                if (menuEvent.key.keysym.sym == SDLK_UP) { // If down key is pressed
                    choice--;
                    if (choice == -1) {
                        choice = m.numOptions - 1;
                    }
                }
                if (menuEvent.key.keysym.sym == SDLK_RETURN) { // If an option is selected
                    isrunning = 0;
                    Mix_HaltChannel(m.bg_Channel); // Stop the backgroung music
                }
                Mix_PlayChannel(m.click_channel, m.clickSound, 0); // Play the click sound
            }
            if (menuEvent.type == SDL_MOUSEMOTION) {
                SDL_Point mouse;
                SDL_GetMouseState(&mouse.x, &mouse.y);
                for (int i = 0; i < m.numOptions; i++) {
                    if (SDL_PointInRect(&mouse, &m.menuButtons[i].rect)) {
                        choice = i;
                    }
                }
            }
        }
    }
    // Play the background music indefinitely
    Mix_PlayChannel(m.bg_Channel, m.bgSound, -1);
    while (isrunning) {
        checkMenuEvents();
        // Set bg color and fill the screen with it
        SDL_SetRenderDrawColor(m.renderer, 0, 0, 30, 0);
        SDL_RenderClear(m.renderer);

        // Create the menu title button fron a font
        m.menuTitleButton = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize * 2, m.seleColor, m.menuTitle);
        // Create a button for each option in menu
        for (int i = 0; i < m.numOptions; i++) {
            if (i == choice) { // If button is the highlighed button, create it with special font
                m.menuButtons[i] = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize, m.seleColor, m.menutext[i]);
                continue;
            }
            m.menuButtons[i] = newButton(m.renderer, "fonts/Crackman Back.otf", m.fontSize, m.idleColor, m.menutext[i]);
        }
        // Set menu title's position
        m.menuTitleButton.rect.x = m.mx;
        m.menuTitleButton.rect.y = m.my;
        // Set menu options' positions and draw them
        for (int i = 0; i < m.numOptions; i++) {
            m.menuButtons[i].rect.x = m.mx + m.indent;
            if (i == choice) { // If button is highlighted, indent it by 20 pixels
                m.menuButtons[i].rect.x += 20;
            }
            m.menuButtons[i].rect.y += i * m.menuButtons[i].rect.h + m.menuTitleButton.rect.h * 2;
            drawButton(m.menuButtons[i]); // Draw the button to the renderer
            SDL_DestroyTexture(m.menuButtons[i].texture); // Destroy the button texture. Another one will be created at next iteration
        }
        drawButton(m.menuTitleButton);
        SDL_DestroyTexture(m.menuTitleButton.texture); // Destroy menu title button
        SDL_RenderPresent(m.renderer); // Show the renderer
        SDL_Delay(50); // Little optional delay
    }
    return choice; // Return position of the highlighted option
}

void printScore(button digits[20], int num, int x, int y) {
    int number[20];
    int d = 0;
    int i = 0;
    while (num != -1 && d < 20) { // The digits of num in reverse order
        number[i] = num % 10;
        num /= 10;
        d++;
        i++;
        if (num == 0)num = -1;
    }
    for (int i = d - 1; i >= 0; i--) { // Set the position and draw the corresponding buttons
        digits[number[i]].rect.x = x + (d - 1 - i) * digits[number[i]].rect.w;
        digits[number[i]].rect.y = y;
        drawButton(digits[number[i]]);
    }
}
