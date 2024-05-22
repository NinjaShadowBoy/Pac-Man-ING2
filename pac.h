#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <maze.h>


typedef struct pac {
    clock_t then;
    clock_t now;
    clock_t lastFrameSwap;

    int frame; // Frame at which animation is
    int numOfFrames;
    int pow;
    int startOfPower;

    double size;
    double minSize;
    double deltaSize;
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
    SDL_Rect rect;
    cell pos;
} pac;

int drawPac(pac* p);

int initPac(pac* p, int size, SDL_Window* window, SDL_Renderer* renderer, int lvl) {
    // Inherit window and renderer
    p->renderer = renderer;
    p->window = window;

    // Set time
    p->then = clock();
    p->now = clock();
    p->lastFrameSwap = clock();

    // Set direction and movement fields
    p->movingLeft = 0;
    p->movingRight = 0;
    p->movingDown = 0;
    p->movingUp = 0;
    p->angle = 0;

    p->x = 10;
    p->y = 10;
    p->speed = (3.5 + lvl * 0.20) * size;
    p->direction = 1;


    // Set size
    p->maxSize = size;
    p->size = p->maxSize;
    p->minSize = p->size / 2.5;
    p->deltaSize = (p->maxSize - p->minSize) / (15 - lvl/3);
    p->score = 0;
    p->hasEatenGhost = 0;


    // Set number of frames and first frame
    p->numOfFrames = 9;
    p->frame = 0;
    p->pow = 0;

    // Create surfaces for images to be loaded on
    SDL_Surface* characterSurfaces[p->numOfFrames];
    p->Textures = (SDL_Texture**)malloc(sizeof(SDL_Texture*)*p->numOfFrames);
    char fileDirectory[200] = ""; // Buffer to store the name of the image file

    // Load images and create textures
    for (int i = 0; i < p->numOfFrames; i++) {
        sprintf(fileDirectory, "images/Pac%d.png", i);
        // Load an image (replace with your own image path)
        characterSurfaces[i] = IMG_Load(fileDirectory);

        if (!characterSurfaces[i]) { // If no surface was created raise an error
            printf("Error loading image: %s\n", IMG_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        // Create a texture from the image surface
        p->Textures[i] = SDL_CreateTextureFromSurface(renderer, characterSurfaces[i]);
        SDL_FreeSurface(characterSurfaces[i]); // We do not need the surface anymore
    }
    return 0;
}


void updatePacFrame(pac* p) {
    // Change the frame which will be draw when draw_pac() will be called
    if ((clock() - p->lastFrameSwap) > 20) {
        p->frame++;
        p->lastFrameSwap = clock();
    }
    if (p->frame >= p->numOfFrames) {
        p->frame = 0;
    }
}

int updatePacPos(pac* p, plane plan) {
    // Calculate time elapsed since last update
    double dt = (double)((clock() - p->then) / ((double)(CLOCKS_PER_SEC)));
    p->then = clock();
    if (dt > 0.008) {
        dt = 0.008;
    }

    int left = p->rect.x;
    int right = p->rect.x + p->rect.w;
    int top = p->rect.y;
    int bottom = p->rect.y + p->rect.h;
    int leftLimit = p->pos.rect.x;
    int rightLimit = p->pos.rect.x + p->pos.rect.w;
    int topLimit = p->pos.rect.y;
    int bottomLimit = p->pos.rect.y + p->pos.rect.h;

    if (*p->pos.l_bottom) {
        bottomLimit = 100000000;
    }
    if (*p->pos.l_top) {
        topLimit = -1;
    }
    if (*p->pos.l_right) {
        rightLimit = 100000000;
    }
    if (*p->pos.l_left) {
        leftLimit = -1;
    }

    // Permit walking pass the wall if it is the left portal
    if (p->pos.index.i == 0 && p->pos.index.j == plan.sizeY / 2) {
        leftLimit -= 50;
        SDL_Rect portal = {.x = p->pos.rect.x - plan.cellSpacing, .y = p->pos.rect.y, .w = plan.cellSpacing / 2, .h = plan.cellSize};
        SDL_SetRenderDrawColor(p->renderer, 255, 140, 255, 0);
        SDL_RenderFillRect(p->renderer, &portal);
        if (SDL_HasIntersection(&p->rect, &portal)) {
            p->pos = plan.self[plan.sizeX - 1][plan.sizeY / 2];
            p->x = p->pos.rect.x + p->pos.rect.w - p->size + plan.cellSpacing / 2 - 2;
            p->y = p->pos.y + p->pos.rect.h / 2 - p->size / 2;
        }
    }
    // Permit walking pass the wall if it is the right portal
    if (p->pos.index.i == plan.sizeX - 1 && p->pos.index.j == plan.sizeY / 2) {
        rightLimit += 50;
        SDL_Rect portal = {.x = p->pos.rect.x + p->pos.rect.w + plan.cellSpacing / 2, .y = p->pos.rect.y, .w = plan.cellSpacing / 2, .h = plan.cellSize};
        SDL_SetRenderDrawColor(p->renderer, 140, 255, 255, 0);
        SDL_RenderFillRect(p->renderer, &portal);
        if (SDL_HasIntersection(&p->rect, &portal)) {
            p->pos = plan.self[0][plan.sizeY / 2];
            p->x = p->pos.x - p->pos.spacing / 2 + 2;
            p->y = p->pos.y + p->pos.rect.h / 2 - p->size / 2;
        }
    }

    // Set orientation and Update y and x attributes
    if (p->movingLeft && left - dt * p->speed > leftLimit) {
        p->x -= dt * p->speed;
    } else if (p->movingRight && right + dt * p->speed < rightLimit) {
        p->x += dt * p->speed;
    } else if (p->movingUp && top - dt * p->speed > topLimit) {
        p->y -= dt * p->speed;
    } else if (p->movingDown && bottom + dt * p->speed < bottomLimit) {
        p->y += dt * p->speed;
    }

    // Set orientation (angle)
    if (p->movingLeft) {
        if (p->angle < 0) {
            p->targetAngle = -180;
        } else {
            p->targetAngle = 180;
        }
    } else if (p->movingRight) {
        p->targetAngle = 0;
    } else if (p->movingUp) {
        if (p->targetAngle == 180) {
            p->angle = -180;
        }
        p->targetAngle = -90;
    } else if (p->movingDown) {
        if (p->targetAngle == -180) {
            p->angle = 180;
        }
        p->targetAngle = 90;
    }

    if (p->angle != p->targetAngle) {
        if (p->angle > p->targetAngle) {
            p->angle -= 120 * p->speed / p->size * dt;
        } else {
            p->angle += 120 * p->speed / p->size * dt;
        }
    }

    // Rectify position in  cell

    if (p->x < leftLimit + 1) {
        p->x += dt * p->speed;
    }
    if (p->x + p->rect.w > rightLimit - 1) {
        p->x -= dt * p->speed;
    }
    if (p->y < topLimit + 1) {
        p->y += dt * p->speed;
    }
    if (p->y + p->rect.h > bottomLimit - 1) {
        p->y -= dt * p->speed;
    }

    updatePacFrame(p);
    return 0;
}


int drawPac(pac* p) {
    updatePacFrame(p);
    SDL_Rect characterRect = { (int)p->x, (int)p->y, (int)p->size, (int)p->size};
    p->rect = characterRect;
    SDL_RenderCopyEx(p->renderer, p->Textures[p->frame], NULL, &characterRect, p->angle, NULL, SDL_FLIP_NONE);
//	SDL_RenderCopy(p->renderer, p->Textures[p->frame], NULL, &characterRect);
    return 0;
}


typedef struct Ghost {
    clock_t then;
    clock_t now;
    clock_t lastFrameSwap;
    clock_t lastStateSwap;
    clock_t lastTimeHit;
    clock_t startOfFright;
    clock_t startOfscatter;
    clock_t startOfchase;

    int scatterTime; // Duration in scatter mode
    int chaseTime; // Duration in chasemode

    int frightTime;

    int frame; // Frame at which animation is
    int state; // 0 for scatter; 1 for chase; 2 for frighthened; 3 for eaten
    int type; // 0 for Blinky; 1 for Pinky; 2 for Inky; 3 for Clyde
    int numOfFrames;

    float size;
    float normalSize;
    float eatenSize;

    int movingLeft;
    int movingRight;
    int movingDown;
    int movingUp;
    int angle;
    int direction;

    double speed;
    double frightSpeed;
    double normalSpeed;
    double eatenSpeed;
    double x;
    double y;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture** Textures;
    SDL_Texture** frightTextures;
    SDL_Texture* eyeTexture;
    SDL_Texture* eatenTexture;
    SDL_Rect rect;
    cell pos;
    cell target;
    coord immediateDest;
} Ghost;

int initGhost(Ghost *g, int size,  SDL_Window* window, SDL_Renderer* renderer, int numGhosts, int level) {
    // Inherit window and renderer
    for (int i = 0; i < numGhosts; i++) {
//		g[i] = (Ghost*)malloc(sizeof(Ghost));
        g[i].renderer = renderer;
        g[i].window = window;
        g[i].lastTimeHit = clock();
        g[i].startOfscatter = clock();
        g[i].startOfchase = clock();
        g[i].scatterTime = 15000 - 1000*level;
        g[i].chaseTime = 30000;
        g[i].frightTime = 9000 - 500*level;


        // Set time
        g[i].then = clock();
        g[i].now = clock();
        g[i].lastFrameSwap = clock();

        // Set direction and movement fields

        g[i].movingLeft = 0;
        g[i].movingRight = 0;
        g[i].movingDown = 0;
        g[i].movingUp = 0;
        g[i].angle = 0;
        g[i].type = i % 4;

        if (i == 0) g[i].movingUp = 1;

        g[i].x = 10;
        g[i].y = 10;
        g[i].speed = 80;
        g[i].direction = 0;
        g[i].state = 1;


        // Set size
        g[i].size = size;
        g[i].normalSize = g[i].size;
        g[i].eatenSize = g[i].size / 2;

        // Set number of frames and first frame
        g[i].numOfFrames = 2;
        g[i].frame = 0;

        SDL_Rect r = {(int)g[i].x, (int)g[i].y, (int)g[i].size, (int)(g[i].size * 1.15) };
        g[i].rect = r;

        // Create surfaces for images to be loaded on
        SDL_Surface* characterSurfaces[g[i].numOfFrames];
        g[i].Textures = (SDL_Texture**)malloc(sizeof(SDL_Texture*)*g[i].numOfFrames);
        g[i].frightTextures = (SDL_Texture**)malloc(sizeof(SDL_Texture*)*g[i].numOfFrames);
        char fileDirectory[200] = ""; // Buffer to store the name of the image file

        // Load images and create textures

        for (int j = 0; j < g[i].numOfFrames; j++) {
            sprintf(fileDirectory, "images/Ghost%d%d.png", i>3?i%5:g[i].type, j);
            // Load an image (replace with your own image path)
            characterSurfaces[j] = IMG_Load(fileDirectory);

            if (!characterSurfaces[j]) { // If no surface was created raise an error
                printf("Error loading image: %s\n", IMG_GetError());
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
            }

            // Create a texture from the image surface
            g[i].Textures[j] = SDL_CreateTextureFromSurface(renderer, characterSurfaces[j]);

            // For fright texture
            strcpy(fileDirectory, "");
            sprintf(fileDirectory, "images/Ghost%d%d.png", 5, j);
            characterSurfaces[j] = IMG_Load(fileDirectory);
            g[i].frightTextures[j] = SDL_CreateTextureFromSurface(renderer, characterSurfaces[j]);
            characterSurfaces[j] = IMG_Load("images/eye.png");
            g[i].eyeTexture = SDL_CreateTextureFromSurface(renderer, characterSurfaces[j]);
            SDL_FreeSurface(characterSurfaces[j]); // We do not need the surface anymore
        }
    }
    return 0;
}


void updateGhostFrame(Ghost* g) {
    // Change the frame
    if ((g->now - g->lastFrameSwap) > 166) {
        g->frame++;
        g->lastFrameSwap = clock();
    }
    if (g->frame >= g->numOfFrames) {
        g->frame = 0;
    }
}

void setChasetarget(Ghost* g, plane plan, pac* p) {
    // Set target index to pac man's position index
    int i = p->pos.index.i;
    int j = p->pos.index.j;
//	int midi = plan.sizeX / 2;
    int midj = plan.sizeY / 2;

    if (g->state == 2) { // if in frightened mode
        int i, j;
        do {
            do {
                i = rand() % plan.sizeX;
                j = rand() % plan.sizeY;
                // Avoid targetting inaccessible cells
            } while ((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 2 && (j == midj - 1 || j == midj + 1)) || IsCellIsolated(plan, i, j));
            g->target = plan.self[i][j];
        } while (IsCellIsolated(plan, i, j));
        // To avoid targetting an isolated cell
        return;
    }
    if (g->state == 3) { // if in eaten mode
        g->target = plan.self[(plan.sizeX - 1) / 2][(plan.sizeY - 1) / 2];
        if (g->pos.index.i == g->target.index.i && g->pos.index.j == g->target.index.j) {
            // If back to ghost house
            g->state = 1; // get back to chase mode
            g->size = g->normalSize;
            g->speed = g->normalSpeed;
            g->y = g->pos.y + g->pos.rect.h / 2 - 1.15 * g->size / 2;
            g->x = g->pos.x + g->pos.rect.w / 2 - g->size / 2;
        }
        return;
    }
    // Modify position index according to ghost type
    if (g->type == 0) { // Red
        if (g->state == 0) { // if in scatter mode
            g->target = plan.self[plan.sizeX - 1][0];
            return;
        }
    }
    if (g->type == 2) { // blue
        if (g->state == 0) { // if in scatter mode
            g->target = plan.self[plan.sizeX - 1][plan.sizeY - 1];
            return;
        }
        int pi = i;
        int pj = j;
        if (p->direction == 0) {
            pj -= 1;
        } else if (p->direction == 1) {
            pi += 1;
        } else if (p->direction == 2) {
            pj += 1;
        } else if (p->direction == 3) {
            pi -= 1;
        }
        int bi = g[0].pos.index.i;
        int bj = g[0].pos.index.j;

        i = pi - bi + pi;
        j = pj - bj + pj;
        if (i < 0) {
            i = 0;
        }
        if (i > plan.sizeX - 1) {
            i = plan.sizeX - 1;
        }
        if (j < 0) {
            j = 0;
        }
        if (j > plan.sizeY - 1) {
            j = plan.sizeY - 1;
        }
        if (IsCellIsolated(plan, i, j)) {
            j -= 1;
        }
        if ((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 2 && (j == midj - 1 || j == midj + 1)) || IsCellIsolated(plan, i, j)) {
            j += 1;
        }
    }
    if (g->type == 3) { // Yellow
        if (distanceBtw(p->pos, g->pos) < 4 * (plan.cellSize + plan.cellSpacing) || g->state == 0) {
            i = 0;
            j = plan.sizeY - 1;
        }
    }
    if (g->type == 1) { // Pink
        if (g->state == 0) { // if in scatter mode
            g->target = plan.self[0][0];
            return;
        }
        if (p->direction == 0) {
            if (j - 2 >= 0) {
                j -= 2;
            }
        } else if (p->direction == 1) {
            if (i + 2 < plan.sizeX) {
                i += 2;
            }
        } else if (p->direction == 2) {
            if (j + 2 < plan.sizeY) {
                j += 2;
            }
        } else if (p->direction == 3) {
            if (i - 2 >= 0) {
                i -= 2;
            }
        }
        if (IsCellIsolated(plan, i, j)) {
            j -= 1;
        }
        if ((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == plan.sizeX - 2 && (j == midj - 1 || j == midj + 1)) || IsCellIsolated(plan, i, j)) {
            j += 1;
        }
    }
    g->target = plan.self[i][j];
}

void reverseDirection(Ghost* g, plane plan) {
    // Reverse direction
    if (g->movingLeft && *(g->pos.l_right) == 1) {
        g->immediateDest.i += 2;
        if (g->immediateDest.i > plan.sizeX - 1) {
            g->immediateDest.i = plan.sizeX - 1;
        }
    } else if (g->movingRight && *(g->pos.l_left) == 1) {
        g->immediateDest.i -= 2;
        if (g->immediateDest.i < 0) {
            g->immediateDest.i = 0;
        }
    } else if (g->movingDown && *(g->pos.l_top) == 1) {
        g->immediateDest.j -= 2;
        if (g->immediateDest.j < 0) {
            g->immediateDest.j = 0;
        }
    } else if (g->movingUp && *(g->pos.l_bottom) == 1) {
        g->immediateDest.j += 2;
        if (g->immediateDest.i > plan.sizeY - 1) {
            g->immediateDest.i = plan.sizeY - 1;
        }
    }
}
void changeGhostDestination(Ghost* g, plane plan, pac* p, int numGhosts) {
    // Update ghost position cell
    for (int n = 0; n < numGhosts; n++) {
        int cx = g[n].rect.x + g[n].rect.w / 2;
        int cy = g[n].rect.y + g[n].rect.h / 2;
        int prec = 6;
        int gii = g[n].pos.index.i;
        int gjj = g[n].pos.index.j;
        int gi = g[n].pos.index.i;
        int gj = g[n].pos.index.j;
        for (int i = 0; i < plan.sizeX; i++) { // Update the position cell
            for (int j = 0; j < plan.sizeY; j++) {
                // If it's not the same position
                if (gii != i || gjj != j) {
                    int destx = plan.self[i][j].rect.x + plan.self[i][j].rect.w / 2;
                    int desty = plan.self[i][j].rect.y + plan.self[i][j].rect.h / 2;
                    int diffx = abs(cx - destx);
                    int diffy = abs(cy - desty);

                    if (diffx < prec && diffy < prec) { // If cell is close enough set as new position
                        g[n].pos = plan.self[i][j];
                        // printf("\nGhost%d changed position(%d, %d)", n, i, j);
//						g[n].y = g[n].pos.y + g[n].pos.rect.h / 2 - 1.15 * g[n].size / 2;
//						g[n].x = g[n].pos.x + g[n].pos.rect.w / 2 - g[n].size / 2;

                        // If ghost is chasing and has been chasing for long enough
                        // put ghost in scatter mode
                        g[n].now = clock();
                        if (g[n].state == 1 && g[n].now - g[n].startOfchase > g[n].chaseTime) {
                            g[n].state = 0;
                            g[n].startOfscatter = g[n].now;
                            reverseDirection(&g[n], plan);
                        }
                        // If ghost is scattering and has been scattering for long enough
                        // put ghost in chase mode
                        if (g[n].state == 0 && g[n].now - g[n].startOfscatter > g[n].scatterTime) {
                            g[n].state = 1;
                            g[n].startOfchase = g[n].now;
                            g[n].speed = g[n].normalSpeed;
                            reverseDirection(&g[n], plan);
                        }
                        // If ghost is frightened and has been frightened for long enough
                        // put ghost in chase mode
                        if (g[n].state == 2 && g[n].now - g[n].startOfFright > g[n].frightTime) {
                            g[n].state = 1;
                            g[n].speed = g[n].normalSpeed;
                            g[n].startOfchase = g[n].now;
                        }

                        // Set target according to Pac man's position and ghost state
                        setChasetarget(&g[n], plan, p);

                        // Update destination
                        gi = g[n].pos.index.i; // Present position
                        gj = g[n].pos.index.j;

                        double mindistance = 100000000000; // Infinity

                        // Find the closest square around ghost to target's position
                        // then move to that square (by setting it as destination)
                        // Ckecking left
                        if (!g[n].movingRight && *g[n].pos.l_left) {
                            double dis = distanceBtw(g[n].target, plan.self[gi - 1][gj]);
                            if (dis < mindistance) { // If distance is less than min, set new min
                                mindistance = dis;
                                g[n].immediateDest = plan.self[gi - 1][gj].index;
                            }
                        }
                        // Ckecking right
                        if (!g[n].movingLeft && *g[n].pos.l_right) {
                            double dis = distanceBtw(g[n].target, plan.self[gi + 1][gj]);
                            if (dis < mindistance) { // If distance is less than min, set new min
                                mindistance = dis;
                                g[n].immediateDest = plan.self[gi + 1][gj].index;
                            }
                        }
                        // Ckecking up
                        if (!g[n].movingDown && *g[n].pos.l_top) {
                            double dis = distanceBtw(g[n].target, plan.self[gi][gj - 1]);
                            if (dis < mindistance) { // If distance is less than min, set new min
                                mindistance = dis;
                                g[n].immediateDest = plan.self[gi][gj - 1].index;
                            }
                        }
                        // Ckecking down
                        if (!g[n].movingUp && *g[n].pos.l_bottom) {
                            double dis = distanceBtw(g[n].target, plan.self[gi][gj + 1]);
                            if (dis < mindistance) { // If distance is less than min, set new min
                                mindistance = dis;
                                g[n].immediateDest = plan.self[gi][gj + 1].index;
                            }
                        }

                        // Calculate the shortest path to target and set it
                        // as destination if not conflicting with direction
                        path pa = shortPath(plan, g[n].pos.index, g[n].target.index);
                        coord best = pa.steps[1]; // The first step on the shortest path
                        int bi = best.i;
                        int bj = best.j;
                        if (bi > -1 && bj > -1) { // Check if not already at the best
                            if (gi < bi && g[n].movingLeft == 0) {
                                g[n].immediateDest = best;
                            } else if (gi > bi && g[n].movingRight == 0) {
                                g[n].immediateDest = best;
                            } else if (gj > bj && g[n].movingDown == 0) {
                                g[n].immediateDest = best;
                            } else if (gj < bj && g[n].movingUp == 0) {
                                g[n].immediateDest = best;
                            }
                        }
                        goto end; // No need to continue looping through rest of maze
                    }

                }
            }
        }
end:
        if (1) {	}
    }
}

int updateGhostPos(Ghost* g, plane plan, coord way) {
    // Calculate time elapsed since last update
    double dt = (double)((clock() - g->then) / ((double)(CLOCKS_PER_SEC)));
    g->then = clock();
    if (dt > 0.008) {
        dt = 0.008;
    }

    int i = way.i;
    int j = way.j;
    int gi = g->pos.index.i;
    int gj = g->pos.index.j;

    if (gi > i) {
        g->movingLeft = 1;
        g->movingRight = 0;
        g->movingDown = 0;
        g->movingUp = 0;
        g->direction = 3;
    } else if (gj < j) {
        g->movingLeft = 0;
        g->movingRight = 0;
        g->movingDown = 1;
        g->movingUp = 0;
        g->direction = 2;
    } else if (gi < i) {
        g->movingLeft = 0;
        g->movingRight = 1;
        g->movingDown = 0;
        g->movingUp = 0;
        g->direction = 1;
    } else if (gj > j) {
        g->movingLeft = 0;
        g->movingRight = 0;
        g->movingDown = 0;
        g->movingUp = 1;
        g->direction = 0;
    }

//	int left = g->rect.x;
//	int right = g->rect.x + g->rect.w;
//	int top = g->rect.y;
//	int bottom = g->rect.y + g->rect.h;
//	int leftLimit = g->pos.rect.x;
//	int rightLimit = g->pos.rect.x + g->pos.rect.w;
//	int topLimit = g->pos.rect.y;
//	int bottomLimit = g->pos.rect.y + g->pos.rect.h;

    // Set orientation and Update y and x attributes
    if (g->movingLeft /*&& left - dt * g->speed > leftLimit*/) {
        g->angle = 0;
        g->x -= dt * g->speed;
    } else if (g->movingRight /*&& right + dt * g->speed < rightLimit*/) {
        g->angle = 0;
        g->x += dt * g->speed;
    } else if (g->movingUp /*&& top - dt * g->speed > topLimit*/) {
        g->angle = 0;
        g->y -= dt * g->speed;
    } else if (g->movingDown /*&& bottom + dt * g->speed < bottomLimit*/) {
        g->angle = 0;
        g->y += dt * g->speed;
    }

    // Permit walking pass the wall if it is the left portal
    if (g->pos.index.i == 0 && g->pos.index.j == plan.sizeY / 2) {
        SDL_Rect portal = {.x = g->pos.rect.x - plan.cellSpacing, .y = g->pos.rect.y, .w = plan.cellSpacing / 2, .h = plan.cellSize};
        SDL_SetRenderDrawColor(g->renderer, 255, 140, 255, 0);
        SDL_RenderFillRect(g->renderer, &portal);
        if (SDL_HasIntersection(&g->rect, &portal)) {
            g->pos = plan.self[plan.sizeX - 1][plan.sizeY / 2];
            g->x = g->pos.rect.x + g->pos.rect.w - g->size + plan.cellSpacing / 2 - 2;
            g->y = g->pos.y + g->pos.rect.h / 2 - 1.15 * g->size / 2;
        }
    }
    // Permit walking pass the wall if it is the right portal
    if (g->pos.index.i == plan.sizeX - 1 && g->pos.index.j == plan.sizeY / 2) {
        SDL_Rect portal = {.x = g->pos.rect.x + g->pos.rect.w + plan.cellSpacing / 2, .y = g->pos.rect.y, .w = plan.cellSpacing / 2, .h = plan.cellSize};
        SDL_SetRenderDrawColor(g->renderer, 140, 255, 255, 0);
        SDL_RenderFillRect(g->renderer, &portal);
        if (SDL_HasIntersection(&g->rect, &portal)) {
            g->pos = plan.self[0][plan.sizeY / 2];
            g->x = g->pos.x - g->pos.spacing / 2 + 2;
            g->y = g->pos.y + g->pos.rect.h / 2 - 1.15 * g->size / 2;
        }
    }
    updateGhostFrame(g);
    return 0;
}


int drawGhost(Ghost* g) {
    g->now = clock();
    SDL_Rect characterRect = { (int)g->x, (int)g->y, (int)(g->size), (int)(g->size * 1.15) };
    g->rect = characterRect;
    if (g->state != 3) { // If ghost is not eaten draw the body
        if (g->state == 2) { // If the ghost is frightened
            SDL_RenderCopyEx(g->renderer, g->frightTextures[g->frame], NULL, &g->rect, g->angle, NULL, SDL_FLIP_NONE);
        } else { // Else draw normal ghost texture
            SDL_RenderCopyEx(g->renderer, g->Textures[g->frame], NULL, &g->rect, g->angle, NULL, SDL_FLIP_NONE);
        }
    }

    float eye_size = g->size/2.5;
    int x = g->x + g->size * 1.0 / 4.0 - eye_size / 2 + g->size * 1.0 / 60.0;
    int y = g->y + 1.15 * g->size * 0.33333 - eye_size / 2;
    SDL_Rect eyeRect = { x, y, (int)(eye_size), (int)(eye_size) };
    // Draw eyes in particlar orientation
    if (g->movingRight) {
    	x += g->size * 1.0 / 30.0;
    	eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, g->angle, NULL, SDL_FLIP_NONE);
        x += g->size * 2.0 / 4.0 - g->size * 1.0 / 60.0;
        eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, g->angle, NULL, SDL_FLIP_NONE);
    } else if (g->movingLeft) {
    	x -= g->size * 1.0 / 30.0;
    	eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, 180, NULL, SDL_FLIP_NONE);
        x += g->size * 2.0 / 4.0 - g->size * 1.0 / 60.0;
        eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, 180, NULL, SDL_FLIP_NONE);
    } else if (g->movingUp) {
    	y -= 1.15 * g->size * 1.0 / 30.0;
    	eyeRect.y = y;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, -90, NULL, SDL_FLIP_NONE);
        x += g->size * 2.0 / 4.0 - g->size * 1.0 / 60.0;
        eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, -90, NULL, SDL_FLIP_NONE);
    } else if (g->movingDown) {
    	y += 1.15 * g->size * 1.0 / 30.0;
    	eyeRect.y = y;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, 90, NULL, SDL_FLIP_NONE);
        x += g->size * 2.0 / 4.0 - g->size * 1.0 / 60.0;
        eyeRect.x = x;
        SDL_RenderCopyEx(g->renderer, g->eyeTexture, NULL, &eyeRect, 90, NULL, SDL_FLIP_NONE);
    }
//	SDL_RenderCopy(g->renderer, g->Textures[g->frame], NULL, &characterRect);
    return 0;
}
