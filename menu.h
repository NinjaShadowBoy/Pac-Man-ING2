#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#include <pac.h>
#include <button.h>

typedef struct menu {
    /*
    A menu is a set of buttons:
        - The title button
        - The menu options' buttons
    Among those buttons, one is highlighted
    It also has a back ground music and a click sound (A sound to be played when a key is pressed while in menu)
    */
    char **menutext;        // Set of strings of the menu options
    char *menuTitle;        // Text of menu title
    int numOptions;         // NUmber of options in a menu
    int mx;                 // x coordinate of the tile button
    int my;                 // y coordinate of the tile button
    int indent;             // Indent of the menu optionn's buttons with respect to the menu title button
    int fontSize;           // Font size of the menu options' buttons
    button **menuButtons;   // Menu options buttons
    button menuTitleButton; // Menu title button
    int bg_Channel;         // Channel of the bg music
    int click_channel;      // Channel of the click sound
    int choice;             // The option currently selected in the menu
    int isrunning;          // A flag which tells if the menu is running or not
    Mix_Chunk *bgSound;     // Back ground music
    Mix_Chunk *clickSound;  // Click sound effect
    SDL_Renderer *renderer; // Renderer
    SDL_Color seleColor;    // Highlighted button's and menu title's font color
    SDL_Color idleColor;    // Other buttons' font color
} menu;

menu newMenu(SDL_Renderer *ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk *bg_sound, int click_channel,
             Mix_Chunk *clickSound);
int showMenu(menu m);
menu newMenu(SDL_Renderer *ren,
             char *menuTitle,
             char **menutext, int numOptions,
             int fontSize, int indent, int mx,
             int my, SDL_Color seleColor, SDL_Color idleColor,
             int bg_channel,
             Mix_Chunk *bg_sound, int click_channel,
             Mix_Chunk *clickSound) {
    /*
    Creates a new button and ruturns it
    */
    menu m;
    m.menutext = menutext;           // Array of strings which are the menu options
    m.numOptions = numOptions;       // Number of options
    m.mx = mx;                       // x position of the menu title button
    m.my = my;                       // y position of the menu title button
    m.bg_Channel = bg_channel;       // Channel of the background music
    m.bgSound = bg_sound;            // Back ground music to be playing while in menu
    m.click_channel = click_channel; // Channel of the click sound effect
    m.clickSound = clickSound;       // Click sound of the menu
    m.renderer = ren;                // Renderer on which to draw the buttons
    m.menuTitle = menuTitle;         // Taxt for the menu title
    m.fontSize = fontSize;           // Font size of the menu options (That of the title will be a multiple of it)
    m.indent = indent;               // Indentation of the menu options with respect to the menu title
    m.seleColor = seleColor;         // Color of the highlighted optionin the menu
    m.idleColor = idleColor;         // Color of other options in the menu

    // Allocate mamory to store the menu options' buttons
    m.menuButtons = (button **)malloc(m.numOptions * sizeof(button *));
    for (int i = 0; i < m.numOptions; i++) {
        // For each color mode
        m.menuButtons[i] = (button *)malloc(2 * sizeof(button));
    }

    return m; // return the menu created
}

void checkMenuEvents(menu* m) {
    // Manage key presses in the menu
    SDL_Event menuEvent;
    SDL_Point mouse;
    while (SDL_PollEvent(&menuEvent)) {
        if (menuEvent.type == SDL_QUIT) {
			Global_quit = 1;
            m->isrunning = 0;
            m->choice = m->numOptions - 1;
            break;
        }
        if (menuEvent.type == SDL_KEYDOWN) {
            if (menuEvent.key.keysym.sym == SDLK_DOWN) {
                // If up key is pressed
                m->choice++;
                if (m->choice == m->numOptions) {
                    m->choice = 0;
                }
            }
            if (menuEvent.key.keysym.sym == SDLK_UP) {
                // If down key is pressed
                m->choice--;
                if (m->choice == -1) {
                    m->choice = m->numOptions - 1;
                }
            }
            if (menuEvent.key.keysym.sym == SDLK_RETURN) {
                // If an option is selected
                m->isrunning = 0;
            }
            Mix_PlayChannel(m->click_channel, m->clickSound, 0); // Play the click sound
        }
        if (menuEvent.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&mouse.x, &mouse.y);
            for (int i = 0; i < m->numOptions; i++) {
                if (SDL_PointInRect(&mouse, &m->menuButtons[i][0].rect) && i != m->choice) {
                    m->choice = i;
                    Mix_PlayChannel(m->click_channel, m->clickSound, 0); // Play the click sound
                }
            }
        }
        if (menuEvent.type == SDL_MOUSEBUTTONDOWN) {
            SDL_GetMouseState(&mouse.x, &mouse.y);
            for (int i = 0; i < m->numOptions; i++) {
                if (SDL_PointInRect(&mouse, &m->menuButtons[i][0].rect)) {
                    Mix_PlayChannel(m->click_channel, m->clickSound, 0); // Play the click sound
                    m->isrunning = 0; // Stop the menu
                }
            }
        }
    }
}

int showMenu(menu m) {
    /*
    This functions indefinitely displays the menu passed as parameter until a choice is made
    */
    m.isrunning = 1;
    m.choice = 0; // The number indicating the highlighted option.
    // By default it is the first option.

    // Play the background music indefinitely
    Mix_PlayChannel(m.bg_Channel, m.bgSound, -1);

    // Create the menu title button fron a font
    m.menuTitleButton = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize * 2, m.seleColor, m.menuTitle);
    // Set menu title's position
    m.menuTitleButton.rect.x = m.mx;
    m.menuTitleButton.rect.y = m.my;
    // Create a button for each option in menu
    for (int i = 0; i < m.numOptions; i++) {
        // If option is the highlighed option, create it with special font color
        m.menuButtons[i][1] = newButton(m.renderer, "fonts/Crackman.otf", m.fontSize, m.seleColor, m.menutext[i]);
        // If option is not highlighted
        m.menuButtons[i][0] = newButton(m.renderer, "fonts/Crackman Back.otf", m.fontSize, m.idleColor, m.menutext[i]);

        m.menuButtons[i][0].rect.x = m.mx + m.indent;
        m.menuButtons[i][1].rect.x = m.mx + m.indent + 20; // Indent the highlighted buttons by 20px

        m.menuButtons[i][0].rect.y = m.my + i * m.menuButtons[i][0].rect.h + m.menuTitleButton.rect.h * 2;
        m.menuButtons[i][1].rect.y = m.my + i * m.menuButtons[i][0].rect.h + m.menuTitleButton.rect.h * 2;
    }

    while (m.isrunning) {
        checkMenuEvents(&m);
        // Set bg color and fill the screen with it
        SDL_SetRenderDrawColor(m.renderer, 0, 0, 30, 0);
        SDL_RenderClear(m.renderer);

        drawButton(m.menuTitleButton);
        // All menu options draw them
        for (int i = 0; i < m.numOptions; i++) {
            if (i == m.choice) {
                // If button is highlighted, indent it by 20 pixels
                drawButton(m.menuButtons[i][1]); // Draw the button to the renderer
            } else
                drawButton(m.menuButtons[i][0]); // Draw the button to the renderer
        }

        SDL_RenderPresent(m.renderer); // Show the renderer
        SDL_Delay(50);                 // Little optional delay
    }
    Mix_HaltChannel(m.bg_Channel); // Stop the backgroung music
    return m.choice == m.numOptions - 1? -1: m.choice; // Return position of the highlighted option
}
