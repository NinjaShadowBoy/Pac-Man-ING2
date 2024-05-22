#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <maze.h>


typedef struct Ghost {
	clock_t then;
	clock_t now;
	clock_t lastFrameSwap;

	int frame; // Frame at which animation is
	int numOfFrames;

	int size;

	int movingLeft;
	int movingRight;
	int movingDown;
	int movingUp;
	int angle;
	int direction;
	double speedx;
	double speedy;
	double x;
	double y;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture** Textures;
	SDL_Rect rect;
	cell pos;
} Ghost;

int initGhost(Ghost* g, SDL_Window* window, SDL_Renderer* renderer) {
	// Inherit window and renderer
	g->renderer = renderer;
	g->window = window;

	// Set time
	g->then = clock();
	g->now = clock();
	g->lastFrameSwap = clock();

	// Set direction and movement fields
	g->movingLeft = 0;
	g->movingRight = 0;
	g->movingDown = 0;
	g->movingUp = 0;
	g->angle = 0;

	g->x = 10;
	g->y = 10;
	g->speedx = 2 * 100;
	g->speedy = 2 * 100;
	g->direction = 1;


	// Set size
	g->size = 25;

	// Set number of frames and first frame
	g->numOfFrames = 5;
	g->frame = 0;

	// Create surfaces for images to be loaded on
	SDL_Surface* characterSurfaces[g->numOfFrames];
	g->Textures = (SDL_Texture**)malloc(sizeof(SDL_Texture*)*g->numOfFrames);
	char fileDirectory[200] = ""; // Buffer to store the name of the image file

	// Load images and create textures
	for (int i = 0; i < g->numOfFrames; i++) {
		sprintf(fileDirectory, "images/Ghost%d.png", i);
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
		g->Textures[i] = SDL_CreateTextureFromSurface(renderer, characterSurfaces[i]);
		SDL_FreeSurface(characterSurfaces[i]); // We do not need the surface anymore
	}
	return 0;
}


void updateGhostFrame(Ghost* g) {
	// Change the frame
	if ((g->now - g->lastFrameSwap) > 40) {
		g->frame++;
		g->lastFrameSwap = clock();
	}
	if (g->frame >= g->numOfFrames) {
		g->frame = 0;
	}
}

int updateGhostPos(Ghost* g) {
	g->now = clock();
	// Calculate time elapsed since last update
	double dt = (double)((g->now - g->then) / ((double)(CLOCKS_PER_SEC)));
	g->then = clock();

	int left = g->rect.x;
	int right = g->rect.x + g->rect.w;
	int top = g->rect.y;
	int bottom = g->rect.y + g->rect.h;
	int leftLimit = g->pos.rect.x;
	int rightLimit = g->pos.rect.x + g->pos.rect.w;
	int topLimit = g->pos.rect.y;
	int bottomLimit = g->pos.rect.y + g->pos.rect.h;

	if (*g->pos.l_bottom) {
		bottomLimit = 100000000;
	}
	if (*g->pos.l_top) {
		topLimit = -1;
	}
	if (*g->pos.l_right) {
		rightLimit = 100000000;
	}
	if (*g->pos.l_left) {
		leftLimit = -1;
	}

	// Set orientation and Update y and x attributes
	if (g->movingLeft && left - dt * g->speedx > leftLimit) {
		g->angle = 0;
		g->x -= dt * g->speedx;
	} else if (g->movingRight && right + dt * g->speedx < rightLimit) {
		g->angle = 0;
		g->x += dt * g->speedx;
	} else if (g->movingUp && top - dt * g->speedy > topLimit) {
		g->angle = 0;
		g->y -= dt * g->speedy;
	} else if (g->movingDown && bottom + dt * g->speedy < bottomLimit) {
		g->angle = 0;
		g->y += dt * g->speedy;
	}

//	// Set orientation (angle)
//	g->angle = g->angle % 360;
//	if (g->direction == 3 && g->angle % 360 != 180) {
//		for (int i = 0; i < 3*dt * g->speedx; i++) {
//			if (g->angle % 360 != 180) {
//				g->angle++;
//			} else {
//				break;
//			}
//		}
//	} else if (g->direction == 1 && g->angle % 360 != 0) {
//		for (int i = 0; i < 3*dt * g->speedx; i++) {
//			if (g->angle % 360 != 0) {
//				g->angle++;
//			} else {
//				break;
//			}
//		}
//	} else if (g->direction == 0 && g->angle % 360 != 270) {
//		for (int i = 0; i < 3*dt * g->speedx; i++) {
//			if (g->angle % 360 != 270) {
//				g->angle++;
//			} else {
//				break;
//			}
//		}
//	} else if (g->direction == 2 && g->angle % 360 != 90) {
//		for (int i = 0; i < 3*dt * g->speedx; i++) {
//			if (g->angle % 360 != 90) {
//				g->angle++;
//			} else {
//				break;
//			}
//		}
//	}

	if (left < leftLimit + 3) {
		g->x += dt * g->speedx;
	}
	if (right > rightLimit - 3) {
		g->x -= dt * g->speedx;
	}
	if (top < topLimit + 3) {
		g->y += dt * g->speedy;
	}
	if (bottom > bottomLimit - 3) {
		g->y -= dt * g->speedy;
	}
	updateGhostFrame(g);
	return 0;
}


int drawGhost(Ghost* g) {
	SDL_Rect characterRect = { (int)g->x, (int)g->y, g->size * 1.2, g->size * 1.2 * 1.2 };
	g->rect = characterRect;
	SDL_RenderCopyEx(g->renderer, g->Textures[g->frame], NULL, &characterRect, g->angle, NULL, SDL_FLIP_NONE);
//	SDL_RenderCopy(g->renderer, g->Textures[g->frame], NULL, &characterRect);
	return 0;
}
