#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


typedef struct button {
	char* text;
	char* fontDirectory;
	SDL_Color color;
	SDL_Renderer* renderer;
	SDL_Rect rect;
	SDL_Texture* texture;
	int fontSize;
} button;

typedef struct menu {
	char **menutext;
	char *menuTitle;
	int numOptions;
	int mx;
	int my;
	int indent;
	int fontSize;
	button* menuButtons;
	button menuTitleButton;
	int bg_Channel;
	int click_channel;
	Mix_Chunk* bgSound;
	Mix_Chunk* clickSound;
	SDL_Renderer* renderer;
	SDL_Color seleColor;
	SDL_Color idleColor;
} menu;

void createButtonTexture(button* b) {
	TTF_Font* font = TTF_OpenFont(b->fontDirectory, b->fontSize); // open font file with specific fontSize
	if (font == NULL) { // Error handling
		printf("\nError opening font file at %s", b->fontDirectory);
	}
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, b->text, b->color); //Create  surface
	b->texture = SDL_CreateTextureFromSurface(b->renderer, surface); // Create texture
	SDL_FreeSurface(surface); // We do not need the surface anymore
	SDL_QueryTexture(b->texture, NULL, NULL, &b->rect.w, &b->rect.h);
	TTF_CloseFont(font);
}

button newButton(SDL_Renderer* ren, char* fontDirectory, int fontSize, SDL_Color txtColor, char* txt) {
	button b;
	b.renderer = ren; // Set renderer
	b.rect.x = b.rect.y = 0; // Position of rect
	b.color = txtColor; // Set text color
	b.fontSize = fontSize;
	b.fontDirectory = fontDirectory;
	b.text = txt;

	createButtonTexture(&b);

	return b;
}

int drawButton(button b) {
	return SDL_RenderCopy(b.renderer, b.texture, NULL, &b.rect);
}

void setButtonText(button* b, char* txt, int fontSize) {
	b->text = txt;
	b->fontSize = fontSize;
	createButtonTexture(b);
}


SDL_Window* win;
SDL_Renderer* ren;


menu newMenu(SDL_Renderer* ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk* bg_sound, int click_channel,
             Mix_Chunk* clickSound) {
	menu m;
	m.menutext = menutext;
	m.numOptions = numOptions;
	m.mx = mx;
	m.my = my;
	m.bg_Channel = bg_channel;
	m.bgSound = bg_sound;
	m.click_channel = click_channel;
	m.clickSound = clickSound;
	m.renderer = ren;
	m.menuTitle = menuTitle;
	m.fontSize = fontSize;
	m.indent = indent;
	m.seleColor = seleColor;
	m.idleColor = idleColor;
	m.menuButtons = (button*)malloc(m.numOptions*sizeof(button));


	return m;
}


int showMenu(menu m) {
	int isrunning = 1;
	int choice = 0;
	SDL_Event menuEvent;
	void checkMenuEvents() {
		while (SDL_PollEvent(&menuEvent)) {
			if (menuEvent.type == SDL_QUIT) {
				isrunning = 0;
				break;
			}
			if (menuEvent.type == SDL_KEYDOWN) {
				if (menuEvent.key.keysym.sym == SDLK_DOWN) {
					choice++;
					if (choice == m.numOptions) {
						choice = 0;
					}
				}
				if (menuEvent.key.keysym.sym == SDLK_UP) {
					choice--;
					if (choice == -1) {
						choice = m.numOptions - 1;
					}
				}
				if (menuEvent.key.keysym.sym == SDLK_RETURN) {
					isrunning = 0;
					Mix_HaltChannel(m.bg_Channel);
				}
				Mix_PlayChannel(m.click_channel, m.clickSound, 0);
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
	Mix_PlayChannel(m.bg_Channel, m.bgSound, -1);
	while (isrunning) {
		checkMenuEvents();
		SDL_SetRenderDrawColor(m.renderer, 0, 0, 15, 0);
		SDL_RenderClear(m.renderer);
		
		m.menuTitleButton = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize * 1.5, m.seleColor, m.menuTitle);
		for (int i = 0; i < m.numOptions; i++) {
			if (i == choice) {
				m.menuButtons[i] = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize, m.seleColor, m.menutext[i]);
				continue;
			}
			m.menuButtons[i] = newButton(m.renderer, "fonts/Crackman Back.otf", m.fontSize, m.idleColor, m.menutext[i]);
		}
		m.menuTitleButton.rect.x = m.mx;
		m.menuTitleButton.rect.y = m.my;
		for (int i = 0; i < m.numOptions; i++) {
			m.menuButtons[i].rect.x = m.mx + m.indent;
			if (i == choice) {
				m.menuButtons[i].rect.x += 20;
			}
			m.menuButtons[i].rect.y += i * m.menuButtons[i].rect.h + m.menuTitleButton.rect.h * 2;
			drawButton(m.menuButtons[i]);
			SDL_DestroyTexture(m.menuButtons[i].texture);
		}
		drawButton(m.menuTitleButton);
		SDL_DestroyTexture(m.menuTitleButton.texture);
		SDL_RenderPresent(m.renderer);
		SDL_Delay(50);
	}
	return choice;
}
