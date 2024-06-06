#include <main.h>

#define RECORDS_FILE "data/records.dat"
#define HIGHSCORES_FILE "data/high.dat"

// Window and renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
/////////////////////////////
//     Global variables    //
/////////////////////////////

// A plane which will to transformed to a maze
plane plan;
// Pointer for an array of ghosts
Ghost *g;
// Create pacman
pac p;

pac livesp[3];

int start = 2;

// Score buttons
button score;
button highScore;

button digits[20];

double PlayingScore = 0;
int levels_reached;
int got_out_menu;

int classicHigh = 0;
int randomHigh = 0;
int crazyHigh = 0;
int numLevelsClassic; // Load the number of unlocked levels from a file
int numLevelsRandom;  // Load the number of unlocked levels from a file
int numLevelsCrazy;   // Load the number of unlocked levels from a file
//    end/Global variables
/////////////////////////////


void callBackForChannels(int channel);

int playLevel(int level, int gameMode, int previousScore) {
    int high;
    switch (gameMode) {
        case 0:
            high = classicHigh;
            break;
        case 1:
            high = randomHigh;
            break;
        case 2:
            high = crazyHigh;
            break;
        default:
            //TODO
            break;
    }

    PlayingScore = previousScore;
    playing = 1;
    pause = 0;
    start = 2;
    int numGhosts;
    int stageMusic = level % NUM_OF_LEVEL_SOUND_TRACKS;
    Mix_PlayChannel(stageMusic, sounds[stageMusic], -1);
    if (gameMode == 0) {
        initPac(&p, 60 * 3.2 / 4, window, renderer, level);
        p.deltaSize *= 4;
        numGhosts = 4;
        plan = InitializePlane(10, 11, p.size, p.size, p.size * 1.1, p.size * 0.5, bg_color, borderColor, window, renderer);
        ClassicMaze(&plan);
    } else {
        initPac(&p, 1400 / ((18 + level / 2) * 1.6), window, renderer, level);
        p.deltaSize *= 3;
        numGhosts = 4 + (level) / 4;
        plan = InitializePlane(10 + level / 2, 11 + level / 2, p.size, p.size, p.size * 1.1, p.size * 0.5, bg_color, borderColor, window, renderer);
        MazifyPlane(&plan, 1);
    }
    p.score = previousScore;


    g = (Ghost *)malloc(numGhosts * sizeof(Ghost));
    // Manage errors while creating ghosts
    if (initGhost(g, (0.95 + level * 0.01) * p.size / 1.15, window, renderer, numGhosts, level, plan, p) == 0) {
        // // // printf("\nGhosts initialized\n");
    } else {
        // printf("Error Ghost");
    }
    Mix_PlayChannel(BG1, sounds[BG1], -1);

    DrawPlane(plan, 0);

    // Set pac position to bottom square in grid
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
        if (level > 8) {
            gspeed = p.speed * 0.90;
        } else {
            gspeed = (0.7 + level * 0.03) * p.speed;
        }
        g[i].speed = gspeed;
        g[i].eatenSpeed = g[i].speed * 3;
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


    /*
      Side game buttons
    */

    // level button
    char lvltxt[20] = "";
    if (gameMode == 0) {
        sprintf(lvltxt, "Classic lvl %d", level + 1);
    } else if (gameMode == 1) {
        sprintf(lvltxt, "Random lvl %d", level + 1);
    } else if (gameMode == 2) {
        sprintf(lvltxt, "Classic lvl %d", level + 1);
    }
    button lvlbtn = newButton(renderer, "fonts/joystix monospace.otf", 50, white, lvltxt);
    lvlbtn.rect.x = plan.right;
    lvlbtn.rect.y = plan.top;

    // Configure rects for health bar
    double factor = 10;
    SDL_FRect redHealthBar;
    redHealthBar.x = plan.right;
    redHealthBar.y = lvlbtn.rect.y + lvlbtn.rect.h * 1.2;
    redHealthBar.w = (p.size - p.minSize) * factor;
    redHealthBar.h = p.size - p.minSize;
    SDL_FRect greenHealthBar = redHealthBar;
    SDL_FRect blackHealthBar = redHealthBar;
    blackHealthBar.x -= 5;
    blackHealthBar.w += 10;
    blackHealthBar.y -= 5;
    blackHealthBar.h += 10;

    // Score buttons
    highScore = newButton(renderer, "fonts/joystix monospace.otf", 35, (SDL_Color) {
        255, 255, 200, 0
    }, "High score");
    highScore.rect.x = plan.right;
    highScore.rect.y = blackHealthBar.y + blackHealthBar.h + 20;

    score = newButton(renderer, "fonts/joystix monospace.otf", 30, (SDL_Color) {
        255, 255, 255, 0
    }, "Score");
    score.rect.x = highScore.rect.x;
    score.rect.y = highScore.rect.y + highScore.rect.h + 90;

    // 200 points button
    button p200 = newButton(renderer, "fonts/joystix monospace.otf", 15, (SDL_Color) {
        0, 255, 255, 0
    }, "200");


    // Button to display the number of lives remaining with the three Pac man heads
    button lives = newButton(renderer, "fonts/joystix monospace.otf", 30, white, "Lives:");
    for (int i = 0; i < 3; i++) {
        livesp[i].x = plan.right + i * (livesp[i].size + 10);
        livesp[i].y = score.rect.y + score.rect.w * 2.3;
    }
    lives.rect.x = plan.right;
    lives.rect.y = livesp[0].y - livesp[0].size;

    // Pause Menu buttons
    button pauseMenu[2][2];
    pauseMenu[0][0] = newButton(renderer, "fonts/Crackman Back.otf", 25, white, "Continue");
    pauseMenu[0][0].rect.x = plan.right + 10;
    pauseMenu[0][0].rect.y = score.rect.y + score.rect.h + 100;

    pauseMenu[0][1] = newButton(renderer, "fonts/Crackman.otf", 30, yellow, ">>Continue");
    pauseMenu[0][1].rect.x = plan.right + 10;
    pauseMenu[0][1].rect.y = score.rect.y + score.rect.h + 100;

    pauseMenu[1][0] = newButton(renderer, "fonts/Crackman Back.otf", 25, white, "Main Menu");
    pauseMenu[1][0].rect.x = plan.right + 10;
    pauseMenu[1][0].rect.y = pauseMenu[0][0].rect.y + pauseMenu[0][0].rect.h + 10;

    pauseMenu[1][1] = newButton(renderer, "fonts/Crackman.otf", 30, yellow, ">>Main Menu");
    pauseMenu[1][1].rect.x = plan.right + 10;
    pauseMenu[1][1].rect.y = pauseMenu[0][0].rect.y + pauseMenu[0][0].rect.h + 10;



    // Main Game loop
    clock_t changeMaze = clock();
    clock_t wintime = 0;
    plane planCopy;
    int changeMazePeriod = 10000 - 500 * level;
    // Create another plane of identical size to the game
    planCopy = InitializePlane(plan.sizeX, plan.sizeY, plan.top, plan.left, plan.cellSize, plan.cellSpacing, plan.cellCol, plan.borderCol, window, renderer);
    while (!Global_quit && (wintime == 0 || clock() - wintime < 2000) && (playing || Mix_Playing(DIE))) {
        checkEvents(numGhosts);
        if (gameMode == 2 && clock() - changeMaze > changeMazePeriod) {
            // Change maze every 6 seconds
            planCopy.isColorful = plan.isColorful;
            for (int i = 0; i < plan.sizeX; i++) {
                // Copy food from plan to planCopy
                for (int j = 0; j < plan.sizeY; j++) {
                    planCopy.self[i][j].isColorful = plan.self[i][j].isColorful;
                    for (int k = 0; k < 5; k++) {
                        planCopy.self[i][j].foods[k] = plan.self[i][j].foods[k];
                    }
                }
            }
            planCopy.numFoods = plan.numFoods;

            plan = InitializePlane(plan.sizeX, plan.sizeY, plan.top, plan.left, plan.cellSize, plan.cellSpacing, plan.cellCol, plan.borderCol, window, renderer);
            MazifyPlane(&plan, 0);

            plan.isColorful = planCopy.isColorful;
            for (int i = 0; i < plan.sizeX; i++) {
                // Copy food from planCopy to plan
                for (int j = 0; j < plan.sizeY; j++) {
                    plan.self[i][j].isColorful = planCopy.self[i][j].isColorful;
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


        // Draw health bar
        if (greenHealthBar.w > (p.size - p.minSize) * factor) {
            greenHealthBar.w -= (greenHealthBar.w - (p.size - p.minSize) * factor) / 40;
            if (p.size - p.minSize < 1) {
                greenHealthBar.w = 0;
            }
        } else if (greenHealthBar.w < (p.size - p.minSize) * factor) {
            greenHealthBar.w += ((p.size - p.minSize) * factor - greenHealthBar.w) / 80;
        }

        // Show score
        drawButton(lvlbtn);
        drawButton(highScore);
        high = PlayingScore > high ? PlayingScore : high;
        printScore(digits, high, highScore.rect.x, highScore.rect.y + highScore.rect.h + 10);
        drawButton(score);
        printScore(digits, PlayingScore, score.rect.x, score.rect.y + score.rect.h + 10);

        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 0);
        SDL_RenderFillRectF(renderer, &blackHealthBar);
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 0);
        SDL_RenderFillRectF(renderer, &redHealthBar);
        SDL_SetRenderDrawColor(renderer, ((p.maxSize - p.size) / (p.maxSize - p.minSize)) * 255, 250, 0, 0);
        SDL_RenderFillRectF(renderer, &greenHealthBar);

        // Draw maze
        SDL_SetRenderDrawColor(renderer, 200, 10, 10, 10);
        DrawPlane(plan, 1);

        // Draw the character textures
        drawPac(&p);
        // the other Pac Man heads by the side and the button
        drawButton(lives);
        for (int i = 0; i < p.lives; i++) {
            drawPac(&livesp[i]);
            if (i == p.lives - 1) {
                SDL_SetRenderDrawColor(renderer, rand() % 255, rand() % 255, rand() % 255, 0);
                SDL_RenderDrawRectF(renderer, &livesp[i].rect);
            }
        }
        for (int i = 0; i < numGhosts; i++) {
            drawGhost(&g[i]);
        }

        if ((wintime == 0 || clock() - wintime < 1000) && !pause && !Mix_Playing(DIE) && !Mix_Playing(BEGIN)) {
            updatePacPos(&p, plan);
        }
        if (!pause && !Mix_Playing(DIE) && !Mix_Playing(BEGIN)) {
            // Ghost operations
            if (wintime == 0) {
                changeGhostDestination(g, plan, &p, numGhosts);
                for (int i = 0; i < numGhosts; i++) {
                    updateGhostPos(&g[i], plan, g[i].immediateDest);
                }
            }
        } else if (pause) {
            // Draw Pause mini menu
            for (int i = 0; i < 2; i++) {
                if (pauseOption == i) {
                    drawButton(pauseMenu[i][1]);
                } else {
                    drawButton(pauseMenu[i][0]);
                }
            }
        }

        // To draw do several iterations of drawing p200 and delay after eaten ghost
        if (p.hasEatenGhost == 100) {
            drawButton(p200);
            p.hasEatenGhost = 0;
        } else if (p.hasEatenGhost != 0) {
            p200.rect.x = p.rect.x + p.rect.w / 2 - p200.rect.w / 2;
            p200.rect.y = p.rect.y - p200.rect.h;
            drawButton(p200);
            if (p.hasEatenGhost == 10) {
                SDL_Delay(300);
                Mix_PlayChannel(ALARM, sounds[ALARM], -1);
            }
            p.hasEatenGhost++;
        }

        if (Mix_Playing(GHOST_EATEN)) {
            if (p.hasEatenGhost == 1) {
                p.hasEatenGhost = 0;
                SDL_Delay(300);
                Mix_PlayChannel(ALARM, sounds[ALARM], -1);
            }
            p200.rect.x = p.rect.x + p.rect.w / 2 - p200.rect.w / 2;
            p200.rect.y = p.rect.y - p200.rect.h;
            drawButton(p200);
        }

        // Set drawing color color,
        SDL_SetRenderDrawColor(renderer, 10, 10, 200, 10);

        // Present the renderer
//        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
//        SDL_RenderDrawRect(renderer, &g[0].target.rect);
//        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 0);
//        SDL_RenderDrawRect(renderer, &g[1].target.rect);
//        SDL_SetRenderDrawColor(renderer, 50, 50, 255, 0);
//        SDL_RenderDrawRect(renderer, &g[2].target.rect);
//        SDL_SetRenderDrawColor(renderer, 255, 250, 0, 0);
//        SDL_RenderDrawRect(renderer, &g[3].target.rect);
//        SDL_RenderDrawRect(renderer, &p.pos.rect);

//        int x1 = g[0].pos.x;
//        int x2 = g[2].target.x;
//        int y1 = g[0].pos.y;
//        int y2 = g[2].target.y;
//        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

        if (plan.numFoods == 0 && wintime == 0) {
            Mix_PlayChannel(STAGE_CLEAR, sounds[STAGE_CLEAR], 0);
            Mix_FadeOutChannel(BG1, 2000);
            Mix_FadeOutChannel(stageMusic, 2000);
            wintime = clock();
        }
        SDL_RenderPresent(renderer);
        if (start && p.lives > 0) {
//            if (start == 1) {
            Mix_FadeOutChannel(stageMusic, 1500);
            Mix_PlayChannel(BEGIN, sounds[BEGIN], 0);
//            }
//            start--;
            start = 0;
        } else if (!Mix_Playing(BEGIN) && !Mix_Playing(stageMusic) && p.lives > 0) {
            Mix_FadeInChannel(stageMusic, sounds[stageMusic], -1, 1500);
        }
    }


    // Clean all sounds
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (i != DIE) {
            Mix_FadeOutChannel(i, 1500);
        }
    }

    // Progressive black screen
//    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
//    for (int i = 0; i < 2000; i += 2) {
//        SDL_RenderDrawLine(renderer, i, 0, i, 1200);
//        SDL_RenderDrawLine(renderer, i + 1, 0, i + 1, 1200);
//        SDL_RenderPresent(renderer);
//    }

    // Clean up and exit
    for (int i = 0; i < p.numOfFrames; i++) {
        if (p.Textures[i]) {
            SDL_DestroyTexture(p.Textures[i]);
        }
    }

    // destroy the ghost textures
    for (int j = 0; j < numGhosts; j++) {
        for (int i = 0; i < g[j].numOfFrames; i++) {
            if (g[j].Textures[i]) {
                SDL_DestroyTexture(g[j].Textures[i]);
            }
        }
    }
    free(g);

    while (Mix_Playing(STAGE_CLEAR) || Mix_Playing(DIE)) {}

    // Save high score
    switch (gameMode) {
        case 0:
            classicHigh = high;
            break;
        case 1:
            randomHigh = high;
            break;
        case 2:
            crazyHigh = high;
            break;
        default:
            //TODO
            break;
    }

    if (plan.numFoods == 0) {
        levels_reached++;
        playLevel(level + 1, gameMode, p.score);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {

    // Load highscores and number of levels available
    FILE* fptr = fopen(HIGHSCORES_FILE, "r");
    fscanf(fptr, "%d\n%d\n%d\n%d\n%d\n%d", &classicHigh, &randomHigh, &crazyHigh, &numLevelsClassic, &numLevelsRandom, &numLevelsCrazy);
    fclose(fptr);

    initSDL();
    Sleep(500);

    // This is the main manu text
    char **menutxt = (char **)malloc(10 * sizeof(char *));
    menutxt[0] = "Classic";
    menutxt[1] = "Random Maze";
    menutxt[2] = "Crazy Maze!?";
    menutxt[3] = "Leader Boards";
    menutxt[4] = "Credits";
    menutxt[5] = "Quit";
    menutxt[6] = "!? PAC MAN ?!"; // Title of the menu with menu options just not to create another variable

    int menuNumOptions = 6;

    for (int i = 0; i < 3; i++) {
        initPac(&livesp[i], 50, window, renderer, 0);
    }

    // Create Main menu
    SDL_Color highlight_color = {.r = 255, .g = 240, .b = 0, .a = 0};
    SDL_Color menu_text_color = {.r = 50, .g = 50, .b = 100, .a = 0};
    menu MainMenu = newMenu(renderer, menutxt[menuNumOptions], menutxt, menuNumOptions, 50, 100, 20, 25, highlight_color, menu_text_color, MAIN_MENU, sounds[MAIN_MENU], CLICK, sounds[CLICK]);

    // Showing Main menu and handling result of choice
    int choice;    // Stores what choice has been made in main menu
    int level = 0; // Stores what level was choosed in sub-menus (Classic, Random Maze, Crazy Maze)

    while (!Global_quit) {
        Global_quit = 0;
        choice = showMenu(MainMenu); // Which choice has been made. It also repreesents the game mode

        // Sub menu text memory allocation
        char **subMenutxt = (char **)malloc(20 * sizeof(char *));

        // Handle the choices made in the menu

        if (choice == 0) {
            // If classic was choosed
            for (int i = 0; i < numLevelsClassic; i++) {
                // Create text for all levels
                subMenutxt[i] = (char*)malloc(30 * sizeof(char));
                sprintf(subMenutxt[i], "Level %d", i + 1);
            }
            subMenutxt[numLevelsClassic] = "Back"; // At last position for the options

            // Create a menu for the text created above
            menu subMenu = newMenu(renderer, "Classic", subMenutxt, numLevelsClassic + 1, 35, 20, 140, 50, MainMenu.seleColor, MainMenu.idleColor, STAGE_SELECT, sounds[STAGE_SELECT], CLICK, sounds[CLICK]);
            level = showMenu(subMenu); // Show the classic menu

        } else if (choice == 1) {
            // If random maze was choosed
            for (int i = 0; i < numLevelsRandom; i++) {
                // Create text for all levels
                subMenutxt[i] = (char*)malloc(30 * sizeof(char));
                sprintf(subMenutxt[i], "Level %d", i + 1);
            }
            subMenutxt[numLevelsRandom] = "Back"; // At last position for the options

            // Create a menu for the text created above
            menu subMenu = newMenu(renderer, "Random Maze", subMenutxt, numLevelsRandom + 1, 35, 20, 140, 50, MainMenu.seleColor, MainMenu.idleColor, STAGE_SELECT, sounds[STAGE_SELECT], CLICK, sounds[CLICK]);
            level = showMenu(subMenu); // Show the classic menu

        } else if (choice == 2) {
            // If crazy maze was choosed
            for (int i = 0; i < numLevelsCrazy; i++) {
                // Create text for all levels
                subMenutxt[i] = (char*)malloc(30 * sizeof(char));
                sprintf(subMenutxt[i], "Level %d", i + 1);
            }
            subMenutxt[numLevelsCrazy] = "Back"; // At last position for the options

            // Create a menu for the text created above
            menu subMenu = newMenu(renderer, "CRaZyNeSs", subMenutxt, numLevelsCrazy + 1, 35, 20, 140, 50,  MainMenu.seleColor, MainMenu.idleColor, STAGE_SELECT, sounds[STAGE_SELECT], CLICK, sounds[CLICK]);
            level = showMenu(subMenu); // Show the classic menu

        } else if (choice == 3) { // Show leader boards
            Mix_FadeInChannel(LEADERBOARDS, sounds[LEADERBOARDS], -1, 500);
            int num = 0;
            char h[100] = "";
            sprintf(h, "         %s              %s   %s   %s", "Name", "Mode", "lvl", "Score");
            button heads =  newButton(renderer, "fonts/Crackman.otf", 70, yellow, h);
            heads.rect.x = 100;
            heads.rect.y = 100;
            button* recs = LoadRecordsIntoList(renderer, RECORDS_FILE, heads.rect.x, heads.rect.y + heads.rect.h, &num);
            int learderBoards = 1;
            while (learderBoards == 1) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        learderBoards = 0;
                        Global_quit = 1;
                        break;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        learderBoards = 0;
                    }
                }
                SDL_RenderClear(renderer);
                drawButton(heads);
                for (int i = 0; i < num; i++) {
                    drawButton(recs[i]);
                }

                SDL_RenderPresent(renderer);
            }
            level = -1;
            Mix_FadeOutChannel(LEADERBOARDS, 1000);

            // Clean Screen smothly
            SDL_SetRenderDrawColor(renderer, 0, 0, 30, 0);
            for (int i = -1000; i < 3000; i += 2) {
                SDL_RenderDrawLine(renderer, i, 0, i, 1200);
                SDL_RenderDrawLine(renderer, i + 1, 0, i + 1, 1200);
                SDL_RenderPresent(renderer);
            }
        } else if (choice == 4) {
            // If Credits
            Mix_FadeInChannel(LEADERBOARDS, sounds[LEADERBOARDS], -1, 500);
            int num = 0;
            char h[100] = "";
            sprintf(h, "Credits");
            button head =  newButton(renderer, "fonts/Crackman.otf", 100, yellow, h);
            head.rect.x = 100;
            head.rect.y = 100;
			button btns[5];
			sprintf(h, "Developed by Abena Alex Nelson Ryan");
			btns[0] = newButton(renderer, "fonts/consolab.ttf", 50, white, h);
			btns[0].rect.x = head.rect.x;
			btns[0].rect.y = head.rect.y+head.rect.h;
			sprintf(h, "at Saint Jean University during his");
			btns[1] = newButton(renderer, "fonts/consolab.ttf", 50, white, h);
			sprintf(h, "last year of common core (2023-2024).");
			btns[2] = newButton(renderer, "fonts/consolab.ttf", 50, white, h);
			sprintf(h, "From March to May 2024 ");
			btns[3] = newButton(renderer, "fonts/consolab.ttf", 50, white, h);
			sprintf(h, "      Don't forget!! ...  Have fun ;)");
			btns[4] = newButton(renderer, "fonts/Crackman.otf", 60, white, h);
			
			for(int i=0;i<5;i++){
				btns[i].rect.x = btns[i-1].rect.x;
				btns[i].rect.y = btns[i-1].rect.y+btns[i-1].rect.h;
			}
			btns[4].rect.y += btns[4].rect.h;
            int credits = 1;
            while (credits == 1) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        credits = 0;
                        Global_quit = 1;
                        break;
                    }
                    if (event.type == SDL_KEYDOWN) {
                        credits = 0;
                    }
                }
                SDL_RenderClear(renderer);
                drawButton(head);
                for (int i = 0; i < 5; i++) {
                    drawButton(btns[i]);
                }

                SDL_RenderPresent(renderer);
            }
            level = -1;
            Mix_FadeOutChannel(LEADERBOARDS, 1000);

            // Clean Screen smothly
            SDL_SetRenderDrawColor(renderer, 0, 0, 30, 0);
            for (int i = -1000; i < 3000; i += 2) {
                SDL_RenderDrawLine(renderer, i, 0, i, 1200);
                SDL_RenderDrawLine(renderer, i + 1, 0, i + 1, 1200);
                SDL_RenderPresent(renderer);
            }
            level = -1;
        } else if (choice == -1) {
            // If Quit was choosed
            Global_quit = 1;
            level = -1;
        }

        if (level >= 0) {
            got_out_menu = 0;
            PlayingScore = 0;
            levels_reached = level;
            playLevel(level, choice, 0);

            // Register player record if did not go out to main menu
            if (got_out_menu == 0) {
                Mix_FadeInChannel(NAME_ENTRY, sounds[NAME_ENTRY], -1, 2000);
                showScoreAnimation(renderer, PlayingScore, 1 + (levels_reached) * 0.1, 500, 400, COIN, sounds[COIN]);
                record r;
                if (choice == 0) {
                    numLevelsClassic = levels_reached;
                    if (PlayingScore > classicHigh) classicHigh = PlayingScore * (1 + (levels_reached) * 0.1);
                    strcpy(r.mode, "Classic");
                } else if (choice == 1) {
                    numLevelsRandom = levels_reached;
                    if (PlayingScore > randomHigh) randomHigh = PlayingScore * (1 + (levels_reached) * 0.1);
                    strcpy(r.mode, "Random");
                } else if (choice == 1) {
                    numLevelsCrazy = levels_reached;
                    if (PlayingScore > crazyHigh) crazyHigh = PlayingScore * (1 + (levels_reached) * 0.1);
                    strcpy(r.mode, "Crazy");
                }
                r.lvl = levels_reached;
                r.score = PlayingScore;
                strcpy(r.name, who_are_you(renderer, 10, 400, TYPE, sounds[TYPE]));
                writeRecordToFile(r, RECORDS_FILE);
                Mix_FadeOutChannel(NAME_ENTRY, 1000);
            }

            // Clean Screen smothly
            SDL_SetRenderDrawColor(renderer, 0, 0, 30, 0);
            for (int i = 0; i < 2000; i += 2) {
                SDL_RenderDrawLine(renderer, i, 0, i, 1200);
                SDL_RenderDrawLine(renderer, i + 1, 0, i + 1, 1200);
                SDL_RenderPresent(renderer);
            }
        }
    }

    for (int i = 0; i < NUM_CHANNELS; i++) {
        Mix_FreeChunk(sounds[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    fptr = fopen(HIGHSCORES_FILE, "w");
    fprintf(fptr, "%d\n%d\n%d\n%d\n%d\n%d", classicHigh, randomHigh, crazyHigh, numLevelsClassic, numLevelsRandom, numLevelsCrazy);
    fclose(fptr);
    system("delete");

    if (argc || argv) {
    } // Remove annoying warning
    return 0;
}

void handlePacMazeInteractions(int numGhosts) {
    /*
      This will handle interactions between PacMan and the maze with the Food on it
      It is here that we "frighten" the ghosts and we set the maze to be colorful
    */

    SDL_FRect inter;  // These rects will be used to store intersections
    SDL_FRect inter1; // between some other rects
    SDL_IntersectFRect(&p.rect, &p.pos.rect, &inter1);
    // Update pac position cell
    for (int i = 0; i < plan.sizeX; i++) {
        for (int j = 0; j < plan.sizeY; j++) {
            // Go through the maze and find the cell which intersects the most with Pac Man
            // That cell will be set as Pac Man's poition

            // Go through only the 6 (or less) closest cells
            if (abs(p.pos.index.i - i) < 2 && abs(p.pos.index.j - j) < 2) {
                // Manage interactions between the 5 foods which may be on a cell
                for (int k = 0; k < 5; k++) {
                    if ((plan.self[i][j].foods[k] != 0 && SDL_HasIntersectionF(&p.rect, &plan.self[i][j].foodrects[k]))) {
                        int value;
                        plan.numFoods--;
                        value = plan.self[i][j].foods[k]; // Value of the food
                        p.score += value * 10;
                        PlayingScore += value * 10;
                        plan.self[i][j].foods[k] = 0; // Say there is no more food in the cell

                        if (p.size < p.maxSize) {
                            // Prevents the size from exceeding the maximun size
                            p.size += value * (p.deltaSize / 10);
                        }
                        if (value == 1) {
                            // Food is normal
                            Mix_PlayChannel(EAT, sounds[EAT], 0);
                        }
                        if (value == 2) {
                            // Food is special
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

                            // Change ghosts' state to frightened
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

                if (p.pow == 0 && p.size > p.maxSize) { // If no more powerfull and giga size, reduce size
                    p.size -= 0.01;
                    // Compensate for change in size
                    p.x += 0.01 / 2;
                    p.y += 0.01 / 2;
                }

                // Stop plan colorful before end of fright (end of power up)
                if (p.pow && clock() - g[0].startOfFright > g[0].frightTime - 1000) {
                    plan.isColorful = 0;
                    p.pow = 0;
                    Mix_HaltChannel(SPECIAL_FOOD);
                }

                if (
                    SDL_IntersectFRect(&p.rect, &plan.self[i][j].rect, &inter) && (inter.w * inter.h) > (inter1.w * inter1.h)
                    // If pac intersects with another rect more than its position rect
                    // set a that rect as new position
                ) {
                    p.pos = plan.self[i][j];
                    // printf("%d %d\n", i, j);
                    return;
                }
            }
        }
    }
}

void handleGhostPacCollisions(int numGhosts) {
    // Handling collisions between Pac man and ghosts
    SDL_FRect inter; // These rects will be used to store intersections
    SDL_bool shouldStopAlarm = SDL_TRUE; // We should stop the alarm sound if no ghost is in eaten mode
    for (int i = 0; i < numGhosts; i++) {
        if (!Mix_Playing(DIE))
            if (SDL_IntersectFRect(&p.rect, &g[i].rect, &inter)) {
                // If Pac Man and ghost intersect

                if (g[i].state == 2) {
                    // If in frightened mode get into eaten mode
                    Mix_PlayChannel(GHOST_EATEN, sounds[GHOST_EATEN], 0);
                    g[i].state = 3;
                    g[i].speed = g[i].eatenSpeed;
                    p.score += 200;
                    PlayingScore += 200;
                    p.hasEatenGhost = 1;
                    continue;
                }
                if (g[i].state == 0 || g[i].state == 1) {
                    // If in scatter or chase mode
                    float intersection_area = inter.w * inter.h;
                    float PacMan_area = p.rect.w * p.rect.h;
                    // Check if PacMan and ghost intersect enough and that the ghost is not just from bitting PacMan (Iterations occur really fast you know)
                    if (intersection_area > PacMan_area / 3 && clock() - g[i].lastTimeHit > (g[i].size / g[i].speed) * 500) {
                        p.size -= p.deltaSize / 2; // Reduce Pac Man's size by deltasize
                        p.score -= 50;
                        PlayingScore -= 50;
                        if (p.score < 0)
                            p.score = 0;
                            
						if (PlayingScore < 0)
      					PlayingScore = 0;

                        // printf("Score %d", p.score); // update score button
                        // Ajust positon to compensate for change in size
                        p.x += p.deltaSize / 2;
                        p.y += p.deltaSize / 2;
                        Mix_PlayChannel(HURT, sounds[HURT], 0); // Play hurt sound

                        // Death of pac Man
                        if (p.size < p.minSize) {
//                        start = 2;
                            p.lives = p.lives - 1;
                            PlayingScore = (double)p.score;
                            printf("lives %d", p.lives);



                            // If size Pac Man is too small he dies
                            Mix_PlayChannel(DIE, sounds[DIE], 0); // Play die sound

//                        while (Mix_Playing(DIE)) {} // Wait until end of die sound

                        }
                        if (p.lives <= 0) {
                            playing = 0;
                        }
                        g[i].lastTimeHit = clock(); // Set that it is now that this ghost last bit Pac Man
                    }
                }
            }
        if (g[i].state == 3) {
            shouldStopAlarm = SDL_FALSE;
        }
    }
    if (shouldStopAlarm) {
        Mix_HaltChannel(ALARM);
    }
}

void checkEvents(int numGhosts) {
    /*
    This funtion:
         - Manages keyboard events
         - Manages most interactions with between Pac Man and Ghosts when they collide (Other functionalities are in the ghost functions)
         - Updates the cell which represents the position of Pac Man in the game maze
         - Manages interactions with between Pac Man and food
    */

    while (SDL_PollEvent(&event)) {
        // Manage keyboard events
        if (event.type == SDL_QUIT) {
            Global_quit = 1;
            got_out_menu = 1;
            break;
        }
//         Handle arrow key releases
//                if (event.type == SDL_KEYUP) {
//                    if (event.key.keysym.sym == SDLK_LEFT) {
//                        p.movingLeft = 0;
//                    }
//                    if (event.key.keysym.sym == SDLK_RIGHT) {
//                        p.movingRight = 0;
//                    }
//                    if (event.key.keysym.sym == SDLK_UP) {
//                        p.movingUp = 0;
//                    }
//                    if (event.key.keysym.sym == SDLK_DOWN) {
//                        p.movingDown = 0;
//                    }
//                }

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
            if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                if (pause) {
                    pauseOption = pauseOption == 1 ? 0 : 1;
                }
            }

            // Pause Menu
            if (event.key.keysym.sym == SDLK_RETURN) {
                if (pause) {
                    if (pauseOption == 0) {
                        pause = 0;
                    } else {
                        playing = 0;
                        got_out_menu = 1;
                    }
                }
            }

            // Toggle pause on SPACE
            if (event.key.keysym.sym == SDLK_SPACE) {
                pause = pause == 1 ? 0 : 1;
                pauseOption = 0;
            }
        }
    }

    if (!pause) {
        handlePacMazeInteractions(numGhosts);
        handleGhostPacCollisions(numGhosts);
    }
}
int initSDL() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // If SDL_Init did not return 0 (i.e. it failed)
        // printf("Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }
    // printf("SDL initialized\n");

    // Create a window
    window = SDL_CreateWindow("!? Pac Man ?!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        // If window has not been created
        // printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        // If renderer has not been created
        // printf("Error creating renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, 1400, 1000);

    // Initialize SDL_Mixer
    TTF_Init();
    Mix_Init(0);
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
    Mix_AllocateChannels(NUM_CHANNELS);
    // Load sound
    sounds[TRACK0] = Mix_LoadWAV("sounds/lvl7.mp3");
    Mix_VolumeChunk(sounds[TRACK0], 30);
    sounds[TRACK1] = Mix_LoadWAV("sounds/lvl1.mp3");
    Mix_VolumeChunk(sounds[TRACK1], 30);
    sounds[TRACK2] = Mix_LoadWAV("sounds/lvl2.mp3");
    Mix_VolumeChunk(sounds[TRACK2], 30);
    sounds[TRACK3] = Mix_LoadWAV("sounds/lvl3.mp3");
    Mix_VolumeChunk(sounds[TRACK3], 30);
    sounds[TRACK4] = Mix_LoadWAV("sounds/lvl4.mp3");
    Mix_VolumeChunk(sounds[TRACK4], 30);
    sounds[TRACK5] = Mix_LoadWAV("sounds/lvl5.mp3");
    Mix_VolumeChunk(sounds[TRACK5], 30);
    sounds[TRACK6] = Mix_LoadWAV("sounds/lvl6.mp3");
    Mix_VolumeChunk(sounds[TRACK6], 30);

    sounds[STAGE_CLEAR] = Mix_LoadWAV("sounds/Stage Clear.mp3");
    Mix_VolumeChunk(sounds[STAGE_CLEAR], 70);
    sounds[STAGE_SELECT] = Mix_LoadWAV("sounds/Stage Select.mp3");
    Mix_VolumeChunk(sounds[STAGE_SELECT], 30);

    sounds[EAT] = Mix_LoadWAV("sounds/Chomp High2.mp3");
    Mix_VolumeChunk(sounds[EAT], 25);
    sounds[BG1] = Mix_LoadWAV("sounds/Background 1.mp3");
    Mix_VolumeChunk(sounds[BG1], 50);
    sounds[HURT] = Mix_LoadWAV("sounds/Jump117.wav");
    Mix_VolumeChunk(sounds[HURT], 50);
    sounds[SPECIAL_FOOD] = Mix_LoadWAV("sounds/The super saiyan aura Sound and video.mp3");
    Mix_VolumeChunk(sounds[SPECIAL_FOOD], 120);
    sounds[DIE] = Mix_LoadWAV("sounds/Death.mp3");
    Mix_VolumeChunk(sounds[DIE], 80);
    sounds[GHOST_EATEN] = Mix_LoadWAV("sounds/creature-cry-of-hurt.wav");
    Mix_VolumeChunk(sounds[GHOST_EATEN], 70);
    sounds[CLICK] = sounds[EAT];
    sounds[MAIN_MENU] = Mix_LoadWAV("sounds/Nicky_Larson.mp3");
    Mix_VolumeChunk(sounds[MAIN_MENU], 80);
    sounds[ALARM] = Mix_LoadWAV("sounds/Alarm.mp3");
    Mix_VolumeChunk(sounds[ALARM], 120);
    sounds[BEGIN] = Mix_LoadWAV("sounds/Start.mp3");
    Mix_VolumeChunk(sounds[BEGIN], 100);
    sounds[COIN] = Mix_LoadWAV("sounds/coin.mp3");
    sounds[TYPE] = Mix_LoadWAV("sounds/type.mp3");
    Mix_VolumeChunk(sounds[TYPE], 120);
    sounds[FAIL_LEVEL] = Mix_LoadWAV("sounds/mixkit-player-losing-or-failing-2042.wav");
    sounds[LEADERBOARDS] = Mix_LoadWAV("sounds/LeaderBoards.mp3");
    sounds[NAME_ENTRY] = Mix_LoadWAV("sounds/Name Entry (1st).mp3");

    void (*func_pointer)(int) = callBackForChannels;
    Mix_ChannelFinished(func_pointer);

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

void printScore(button digits[20], int num, int x, int y) {
    int number[20];
    int d = 0;
    int i = 0;
    while (num != -1 && d < 20) {
        // The digits of num in reverse order
        number[i] = num % 10;
        num /= 10;
        d++;
        i++;
        if (num == 0)
            num = -1;
    }
    for (int i = d - 1; i >= 0; i--) {
        // Set the position and draw the corresponding buttons
        digits[number[i]].rect.x = x + (d - 1 - i) * digits[number[i]].rect.w;
        digits[number[i]].rect.y = y;
        drawButton(digits[number[i]]);
    }
}

void callBackForChannels(int channel) {
    if (DIE == channel) {
        start = 2;

        if (p.lives > 0) {
            // Set pac position to bottom square in grid
            p.pos = plan.self[plan.sizeX / 2 - 1][plan.sizeY - 3];
            p.x = p.pos.rect.x + 5;
            p.y = p.pos.rect.y + 5;
            p.size = p.maxSize;
            for (int i = 0; i < 5; i++) {
                p.pos.foods[i] = 0;
            }
            p.size = p.maxSize;
        } else {
            Mix_PlayChannel(FAIL_LEVEL, sounds[FAIL_LEVEL], 0);
            SDL_Delay(1000);
        }
        // Set ghost position in grid
        for (int i = 0; i < 4; i++) {
//            drawGhost(&g[i]);
            g[i].pos = plan.self[(int)((plan.sizeX) / 2 - (i % 4) % 2)][(int)((plan.sizeY / 2))];
            g[i].x = g[i].pos.rect.x + g[i].pos.rect.w / 2 - g[i].rect.w / 2 + (i % 4 - 1) * g[i].size;
            g[i].y = g[i].pos.rect.y + g[i].pos.rect.h / 2 - g[i].rect.h / 2;

            if (i % 2) {
                g[i].immediateDest.i = g[i].pos.index.i + 1;
                g[i].immediateDest.j = g[i].pos.index.j;
            } else {
                g[i].immediateDest.i = g[i].pos.index.i - 1;
                g[i].immediateDest.j = g[i].pos.index.j;
            }
        }
        printf("\n Entered call back Function!\n");

    }
}
