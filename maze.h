#include "stdio.h"
#include "conio.h"
#include "windows.h"
#include "stdlib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "characters.h"
#include <math.h>


/// Structures
typedef struct {
    int i;
    int j;
} coord;

typedef struct {
    coord* steps; // An array of coordinated
    unsigned int len; // The length of the array of coordinates
} path;

typedef struct {
    /*
    	This is a single cell. A plane is basically a grid of cells
    */
    int x; // Position on screen (but no more needed since we have a rect below)
    int y;
    int foods[5]; // A call can contain 5 foods. Up right down left center

    // To tell if the cell is linked
    int *l_top;
    int *l_left;
    int *l_right;
    int *l_bottom;
    int timesVisited; // The number of times the cell has been visited (Used in path finding algorithms)
    int distance; // The distance (in number of cells) from a certain starting point (Used in path finding algorithms)
    int size;
    int spacing;
    int isColorful;

    coord index;

    SDL_Rect rect;
    SDL_Rect foodrects[5]; // Rects to contain the food textures when drawing the cell
    SDL_Window* window; // The window on which the cell is drawn (Inherits it from the plan in which the cell is)
    SDL_Renderer* renderer; // The renderer on which the cell is drawn (Inherits it from the plan in which the cell is)
} cell;

typedef struct {
    /*
    	A plane is basically a grid of cells
    */
    int sizeX; // The number of cells horizontally
    int sizeY; // The number of cells vertically

    // Info about the edges of the plane (Can be handy sometimes)
    int top;
    int left;
    int right;
    int bottom;

    int cellSize; // Size of cells in the plane
    int cellSpacing; // Spacing size of cells in the plane
    int numFoods; // Total amount of food in all cells in the plane
    int isColorful; // If the maze is colo
    cell **self;
    SDL_Color cellCol;
    SDL_Color borderCol;
    SDL_Texture* foodTexture;
    SDL_Texture* bonusTexture;
    SDL_Renderer* renderer;
} plane;

/// Global variables

/// Function prototypes
void color(int fg, int bg);
plane InitializePlane(int numCellsX, int numCellsY, int topEdge, int leftEdge, int cellSize, int cellSpacing, SDL_Color cellCol, SDL_Color borderCol, SDL_Window* window, SDL_Renderer* renderer);
void DrawPlane(plane p, int showFood);
int existsIsolated(plane p);
void MazifyPlane(plane *p, int showProcess);
void frame(int x1, int x2, int y1, int y2);
void DrawCell(cell c, int size, int cellSpacing, SDL_Color cellCol, SDL_Color borderCol);
void DrawCellBorders(cell c, int size, int cellSpacing, SDL_Color borderCol);
int RandlinkCell(plane *p, int i, int j);
int RandlinkCellToIsolated(plane *p, int i, int j);
int IsCellIsolated(plane p, int i, int j);
int NumLinksAround(plane p, int i, int j);
int NumLinks(plane p, int i, int j);
int NumIsolatedAround(plane p, int i, int j);
int NumVisitedAround(plane p, int i, int j, int numberOfTimesVisited);
path DepthFirstSearch(plane p, coord start, coord end, int numberOfTimesVisited);
double distanceBtw(cell a, cell b);

/// Function declarations
void color(int fg, int bg) {
    HANDLE H = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(H, bg * 16 + fg);
}

int existsIsolated(plane p) {
    for (int i = 0; i < p.sizeX; i++) {
        for (int j = 0; j < p.sizeY; j++) {
            if (IsCellIsolated(p, i, j)) {
                return 1;
            }
        }
    }
    return 0;
}

plane InitializePlane(int numCellsX, int numCellsY, int topEdge, int leftEdge, int cellSize, int cellSpacing, SDL_Color cellCol, SDL_Color borderCol, SDL_Window* window, SDL_Renderer* renderer) {
    plane p;
    p.renderer = renderer;
    p.cellCol = cellCol;
    p.borderCol = borderCol;
    p.cellSpacing = cellSpacing;
    p.cellSize = cellSize;
    p.sizeX = numCellsX;
    p.sizeY = numCellsY;
    p.top = topEdge;
    p.left = leftEdge;
    p.numFoods = numCellsX * numCellsY * 5;
    p.isColorful = 0;

    // Creating food texture
    // Load image on surface
    SDL_Surface* foodSurface = IMG_Load("images/food.png");
    SDL_Surface* bonusSurface = IMG_Load("images/bonus.png");

    if (!foodSurface || !bonusSurface) { // If no surface was created raise an error
        // printf("Error loading image: %s\n", IMG_GetError());
    }

    // Create a texture from the image surface
    p.foodTexture  = SDL_CreateTextureFromSurface(renderer, foodSurface);
    p.bonusTexture  = SDL_CreateTextureFromSurface(renderer, bonusSurface);
    SDL_FreeSurface(foodSurface); // We do not need the surface anymore
    SDL_FreeSurface(bonusSurface);

    p.self = (cell **)malloc(sizeof(cell *) * numCellsX);
    for (int i = 0; i < numCellsX; i++) {
        p.self[i] = (cell *)malloc(sizeof(cell) * numCellsY);
    }

    // Set position of each cell
    for (int i = 0; i < numCellsX; i++) {
        for (int j = 0; j < numCellsY; j++) {
            p.self[i][j].index.i = i;
            p.self[i][j].index.j = j;
            p.self[i][j].x = p.left + i * (cellSize + cellSpacing);
            p.self[i][j].y = p.top + j * (cellSize + cellSpacing);
            p.self[i][j].size = cellSize;
            p.self[i][j].spacing = cellSpacing;

            // Set rect
            SDL_Rect characterRect = { (int)p.self[i][j].x, p.self[i][j].y, p.cellSize, p.cellSize };
            p.self[i][j].rect = characterRect;
            p.self[i][j].window = window;
            p.self[i][j].renderer = renderer;
            p.self[i][j].isColorful = 0;
        }
    }
    p.bottom = p.self[numCellsX - 1][numCellsY - 1].y + p.cellSize + p.cellSpacing * 2 ;
    p.right = p.self[numCellsX - 1][numCellsY - 1].x + p.cellSize + p.cellSpacing * 2;

    // Initialize cell pointers
    float w, h;
    for (int i = 0; i < numCellsX; i++) {
        for (int j = 0; j < numCellsY; j++) {
            w = h = cellSize / 6; // Food rect size
            p.self[i][j].l_bottom = (int*)malloc(sizeof(int));
            p.self[i][j].l_top = (int*)malloc(sizeof(int));
            p.self[i][j].l_left = (int*)malloc(sizeof(int));
            p.self[i][j].l_right = (int*)malloc(sizeof(int));
            *p.self[i][j].l_left = 0;
            *p.self[i][j].l_right = 0;
            *p.self[i][j].l_top = 0;
            *p.self[i][j].l_bottom = 0;
            p.self[i][j].timesVisited = 0;
            p.self[i][j].distance = -1;
            for (int k = 0; k < 5; k++) {
                p.self[i][j].foods[k] = 1;
            }
            float x, y; // Food rect position
            x = (p.self[i][j].x - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - w / 2; // 1/3 along cell rect
            y = (p.self[i][j].y - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 6)) - h / 2; // vertical middle of cell rect
            SDL_Rect fr0 = {.x = (int)x, .y = (int)y, .w = (int)w, .h = (int)h};
            p.self[i][j].foodrects[0] = fr0;

            x = (p.self[i][j].x - cellSpacing / 2.0 + (cellSize + cellSpacing) * (5.0 / 6)) - w / 2; // 1/3 along cell rect
            y = (p.self[i][j].y - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - h / 2; // vertical middle of cell rect
            SDL_Rect fr1 = {.x = (int)x, .y = (int)y, .w = (int)w, .h = (int)h};
            p.self[i][j].foodrects[1] = fr1;

            x = (p.self[i][j].x - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - w / 2; // 1/3 along cell rect
            y = (p.self[i][j].y - cellSpacing / 2.0 + (cellSize + cellSpacing) * (5.0 / 6)) - h / 2; // vertical middle of cell rect
            SDL_Rect fr2 = {.x = (int)x, .y = (int)y, .w = (int)w, .h = (int)h};
            p.self[i][j].foodrects[2] = fr2;

            x = (p.self[i][j].x - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 6)) - w / 2; // 1/3 along cell rect
            y = (p.self[i][j].y - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - h / 2; // vertical middle of cell rect
            SDL_Rect fr3 = {.x = (int)x, .y = (int)y, .w = (int)w, .h = (int)h};
            p.self[i][j].foodrects[3] = fr3;

//			srand(clock() + rand());
            if ((i == 0 || i == p.sizeX - 1) && (j == 2 || j == p.sizeY - 2) ) { // Randomly select a bonus square
                p.self[i][j].foods[4] = 2;
                w = h = cellSize / 3;
            }
            x = (p.self[i][j].x - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - w / 2; // 1/3 along cell rect
            y = (p.self[i][j].y - cellSpacing / 2.0 + (cellSize + cellSpacing) * (1.0 / 2)) - h / 2; // vertical middle of cell rect
            SDL_Rect fr4 = {.x = (int)x, .y = (int)y, .w = (int)w, .h = (int)h};
            p.self[i][j].foodrects[4] = fr4;

        }
    }

    // Link adjecent cells
    for (int i = 0; i < numCellsX; i++) {
        for (int j = 0; j < numCellsY; j++) {
            if (i > 0) {
                p.self[i][j].l_left = p.self[i - 1][j].l_right;
            }
            if (i < numCellsX - 1) {
                p.self[i][j].l_right = p.self[i + 1][j].l_left;
            }
            if (j > 0) {
                p.self[i][j].l_top = p.self[i][j - 1].l_bottom;
            }
            if (j < numCellsY - 1) {
                p.self[i][j].l_bottom = p.self[i][j + 1].l_top;
            }
        }
    }
    return p;
}

void DrawCellBorders(cell c, int size, int cellSpacing, SDL_Color borderCol) {
    int x1, x2, y1, y2, x11, x22, y11, y22;
    int l = cellSpacing / 2;
    x1 = c.x - 1;
    y1 = c.y - 1;
    x2 = c.x + size;
    y2 = c.y + size;
    x11 = x1 - l;
    x22 = x2 + l;
    y11 = y1 - l;
    y22 = y2 + l;


    if (c.isColorful) {
        SDL_SetRenderDrawColor(c.renderer, rand() % 255, rand() % 255, rand() % 255, 0);
    } else {
        SDL_SetRenderDrawColor(c.renderer, borderCol.r, borderCol.g, borderCol.b, borderCol.a);
    }
    if (!*c.l_bottom) {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - l / 2 + i, y2 + i, x2 + l / 2 - i, y2 + i);
        }
    } else {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - i, y2 + l / 2 - i, x1 - i, y22);
            SDL_RenderDrawLine(c.renderer, x2 + i, y2 + l / 2 - i, x2 + i, y22);
        }
    }
    if (!*c.l_top) {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - l / 2 + i, y1 - i, x2 + l / 2 - i, y1 - i);
        }
    } else {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - i, y1 - l / 2 + i, x1 - i, y11);
            SDL_RenderDrawLine(c.renderer, x2 + i, y1 - l / 2 + i, x2 + i, y11);
        }
    }
    if (!*c.l_right) {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x2 + i, y1 - l / 2 + i, x2 + i, y2 + l / 2 - i);
        }
    } else {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x2 + l / 2 - i, y1 - i, x22, y1 - i);
            SDL_RenderDrawLine(c.renderer, x2 + l / 2 - i, y2 + i, x22, y2 + i);
        }
    }
    if (!*c.l_left) {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - i, y1 - l / 2 + i, x1 - i, y2 + l / 2 - i);
        }
    } else {
        for (int i = 0; i < l / 2; i++) {
            SDL_RenderDrawLine(c.renderer, x1 - l / 2 + i, y1 - i, x11, y1 - i);
            SDL_RenderDrawLine(c.renderer, x1 - l / 2 + i, y2 + i, x11, y2 + i);
        }
    }

//	if (rand() % 90 == 0) {
//		if (!*c.l_bottom) {
//			for (int i = 0; i < cellSpacing / 2; i++) {
//				int l = cellSpacing / 2;
//				SDL_RenderDrawLine(c.renderer, x1 - l, y2 + i, x2 + l, y2 + i);
//				//			SDL_RenderDrawLine(c.renderer, x1 - l, y2 + l, x2 + l, y2 + l);
//			}
//		}
//		if (!*c.l_top) {
//			for (int i = 0; i < cellSpacing / 2; i++) {
//				int l = cellSpacing / 2;
//				SDL_RenderDrawLine(c.renderer, x1 - l, y1 - i, x2 + l, y1 - i);
//				//			SDL_RenderDrawLine(c.renderer, x1 - l, y1 - l, x2 + l, y1 - l);
//			}
//		}
//		if (!*c.l_right) {
//			for (int i = 0; i < cellSpacing / 2; i++) {
//				int l = cellSpacing / 2;
//				SDL_RenderDrawLine(c.renderer, x2 + i, y1 - l, x2 + i, y2 + l);
//				//			SDL_RenderDrawLine(c.renderer, x2 + l, y1 - l, x2 + l, y2 + l);
//			}
//		}
//		if (!*c.l_left) {
//			for (int i = 0; i < cellSpacing / 2; i++) {
//				int l = cellSpacing / 2;
//				SDL_RenderDrawLine(c.renderer, x1 - i, y1 - l, x1 - i, y2 + l);
//				//			SDL_RenderDrawLine(c.renderer, x1 - l, y1 - l, x1 - l, y2 + l);
//			}
//		}
//	}
}

void DrawCell(cell c, int size, int cellSpacing, SDL_Color cellCol, SDL_Color borderCol) {
    /*
    This part was removed (commented) because for the PAC MAN game needs only the borders

    //	int x1, x2, y1, y2;
    //	x1 = c.x - 1;
    //	y1 = c.y - 1;
    //	x2 = c.x + size;
    //	y2 = c.y + size;

    //	 Change drawing color
    //	SDL_SetRenderDrawColor(c.renderer, cellCol.r, cellCol.g, cellCol.b, cellCol.a);
    //	SDL_RenderFillRect(c.renderer, &c.rect);

    //	if (*c.l_bottom) {
    //		for (int i = 0; i < cellSpacing / 2; i++) {
    //			SDL_RenderDrawLine(c.renderer, x1 - i, y2 + i, x2 + i, y2 + i);
    //		}
    //	}
    //	if (*c.l_top) {
    //		for (int i = 0; i < cellSpacing / 2; i++) {
    //			SDL_RenderDrawLine(c.renderer, x1 - i, y1 - i, x2 + i, y1 - i);
    //		}
    //	}
    //	if (*c.l_right) {
    //		for (int i = 0; i < cellSpacing / 2; i++) {
    //			SDL_RenderDrawLine(c.renderer, x2 + i, y1 - i, x2 + i, y2 + i);
    //		}
    //	}
    //	if (*c.l_left) {
    //		for (int i = 0; i < cellSpacing / 2; i++) {
    //			SDL_RenderDrawLine(c.renderer, x1 - i, y1 - i, x1 - i, y2 + i);
    //		}
    //	}

    */
    DrawCellBorders(c, size, cellSpacing, borderCol);
}

int IsCellIsolated(plane p, int i, int j) {
    if ((*p.self[i][j].l_bottom) && j != p.sizeY - 1) {
        return 0;
    }
    if ((*p.self[i][j].l_top) && j != 0) {
        return 0;
    }
    if ((*p.self[i][j].l_left) && i != 0) {
        return 0;
    }
    if ((*p.self[i][j].l_right) && i != p.sizeX - 1) {
        return 0;
    }
    return 1;
}

int NumLinksAround(plane p, int i, int j) {
    // Returns the number of possible valid links which can be formed
    int num = 0;
    if (!(*p.self[i][j].l_bottom) && j < p.sizeY - 1) {
        num++;
    }
    if (!(*p.self[i][j].l_top) && j > 0) {
        num++;
    }
    if (!(*p.self[i][j].l_left) && i > 0) {
        num++;
    }
    if (!(*p.self[i][j].l_right) && i < p.sizeX - 1) {
        num++;
    }
    return num;
}

int NumLinks(plane p, int i, int j) {
    // Returns the number links
    int num = 0;
    if (*p.self[i][j].l_bottom) {
        num++;
    }
    if (*p.self[i][j].l_top) {
        num++;
    }
    if (*p.self[i][j].l_left) {
        num++;
    }
    if (*p.self[i][j].l_right) {
        num++;
    }
    return num;
}


int NumIsolatedAround(plane p, int i, int j) {
    // Returns the number of isolated cells around a particular cell
    int num = 0;
    if (j < p.sizeY - 1) {
        if (IsCellIsolated(p, i, j + 1)) {
            num++;
        }
    }
    if (j > 0) {
        if (IsCellIsolated(p, i, j - 1)) {
            num++;
        }
    }
    if (i > 0) {
        if (IsCellIsolated(p, i - 1, j)) {
            num++;
        }
    }
    if (i < p.sizeX - 1) {
        if (IsCellIsolated(p, i + 1, j)) {
            num++;
        }
    }
    return num;
}

int NumVisitedAround(plane p, int i, int j, int numberOfTimesVisited) {
    // Returns the number of cells around which have been visited numberOfTimesVisited times
    int num = 0;
    if (j < p.sizeY - 1) {
        if (p.self[i][j + 1].timesVisited == numberOfTimesVisited) {
            num++;
        }
    }
    if (j > 0) {
        if (p.self[i][j - 1].timesVisited == numberOfTimesVisited) {
            num++;
        }
    }
    if (i > 0) {
        if (p.self[i - 1][j].timesVisited == numberOfTimesVisited) {
            num++;
        }
    }
    if (i < p.sizeX - 1) {
        if (p.self[i + 1][j].timesVisited == numberOfTimesVisited) {
            num++;
        }
    }
    return num;
}

void DrawPlane(plane p, int showFood) {
    /*
    	To draw all the cells in the plane.
    */
    for (int i = 0; i < p.sizeX; i++) {
        for (int j = 0; j < p.sizeY; j++) {
            p.self[i][j].isColorful = p.isColorful; // To inherit the colorfulness of the plane
            int midj = p.sizeY / 2;
            if (j == midj && (i == 0 || i == p.sizeX - 1)) { // To say that the portal cells are colorful all the time
                p.self[i][j].isColorful = 1;
            }
            if (!IsCellIsolated(p, i, j)) { // Not to draw the isolated squares
                DrawCell(p.self[i][j], p.cellSize, p.cellSpacing, p.cellCol, p.borderCol);
            }
            if (showFood) { // To draw the food on the food in a cell
                for (int k = 0; k < 5; k++) {
                    if (p.self[i][j].foods[k] == 1) { // If the food is simple
                        SDL_RenderCopy(p.self[i][j].renderer, p.foodTexture, NULL, &p.self[i][j].foodrects[k]);
                    }
                    if (p.self[i][j].foods[k] == 2) { // If the food is special
                        SDL_RenderCopy(p.self[i][j].renderer, p.bonusTexture, NULL, &p.self[i][j].foodrects[k]);
                    }
                }
            }
        }
    }
}

void frame(int x1, int x2, int y1, int y2) { // Not used in PAC MAN game
    for (int j = y1; j <= y2; j++) {
        gotoxy(x1, j);
        printf("%c", FULL);
        gotoxy(x2, j);
        printf("%c", FULL);
    }
    for (int i = x1; i <= x2; i++) {
        gotoxy(i, y1);
        printf("%c", SQUARE_BOTTOM);
        gotoxy(i, y2);
        printf("%c", SQUARE_TOP);
    }
}

int RandlinkCell(plane *p, int i, int j) {
    /*
    	To link a cell randomly. Takes as parameters:
    	   - The plane
    	   - The coordinates (i,j) of the cell of interest in the plane
    	It returns:
    	   - A number which represents in which direction the link was created
    */
    int numOptions = 0; // The number of options available
    int options[4]; // The options availabel
    randomize();
    // Get the available options
    if (!(*p->self[i][j].l_bottom) && j < p->sizeY - 1) {
        options[numOptions] = DOWN;
        numOptions++;
    }
    if (!(*p->self[i][j].l_top) && j > 0) {
        options[numOptions] = UP;
        numOptions++;
    }
    if (!(*p->self[i][j].l_left) && i > 0) {
        options[numOptions] = LEFT;
        numOptions++;
    }
    if (!(*p->self[i][j].l_right) && i < p->sizeX - 1) {
        options[numOptions] = RIGHT;
        numOptions++;
    }
    if (numOptions != 0) { // If there were options
        srand(clock() + rand());
        int choice = rand() % numOptions; // Choose among options
        switch (options[choice]) { // Link the cell according to the chosed option
            case UP:
                *p->self[i][j].l_top = 1;
                break;
            case DOWN:
                *p->self[i][j].l_bottom = 1;
                break;
            case LEFT:
                *p->self[i][j].l_left = 1;
                break;
            case RIGHT:
                *p->self[i][j].l_right = 1;
                break;
            default:
                // TODO
                break;
        }
        return options[choice]; // Return the option chosed
    }
    return 0; // If there was no option
}

int RandlinkCellToIsolated(plane *p, int i, int j) {
    /*
    	Similar to RandlinkCell but links only to isolated cells
    */
    int numOptions = 0;
    int options[4];
    randomize();
    if (!(*p->self[i][j].l_bottom) && j < p->sizeY - 1) {
        if (IsCellIsolated(*p, i, j + 1)) {
            options[numOptions] = DOWN;
            numOptions++;
        }
    }
    if (!(*p->self[i][j].l_top) && j > 0) {
        if (IsCellIsolated(*p, i, j - 1)) {
            options[numOptions] = UP;
            numOptions++;
        }
    }
    if (!(*p->self[i][j].l_left) && i > 0) {
        if (IsCellIsolated(*p, i - 1, j)) {
            options[numOptions] = LEFT;
            numOptions++;
        }
    }
    if (!(*p->self[i][j].l_right) && i < p->sizeX - 1) {
        if (IsCellIsolated(*p, i + 1, j)) {
            options[numOptions] = RIGHT;
            numOptions++;
        }
    }
    int choice = rand() % numOptions;
    switch (options[choice]) {
        case UP:
            *p->self[i][j].l_top = 1;
            break;
        case DOWN:
            *p->self[i][j].l_bottom = 1;
            break;
        case LEFT:
            *p->self[i][j].l_left = 1;
            break;
        case RIGHT:
            *p->self[i][j].l_right = 1;
            break;
        default:
            // TODO
            break;
    }
    if (numOptions == 0) {
        return 0;
    }
    return options[choice];
}

void ClassicMaze(plane *p) {
    /*
    	To modify a plane and make it similar to the classic PAC MAN maze
    */
    int x = 0;
    int y = 0;

    x = 0;
    y = 0; // Cell with coordinates (0, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 0; // Cell with coordinates (1, 0)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 0; // Cell with coordinates (2, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 0; // Cell with coordinates (3, 0)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 0; // Cell with coordinates (4, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 5;
    y = 0; // Cell with coordinates (5, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 0; // Cell with coordinates (6, 0)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 0; // Cell with coordinates (7, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 0; // Cell with coordinates (8, 0)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 0; // Cell with coordinates (9, 0)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 1; // Cell with coordinates (0, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 1;
    y = 1; // Cell with coordinates (1, 1)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = 1; // Cell with coordinates (2, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = 1; // Cell with coordinates (3, 1)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 4;
    y = 1; // Cell with coordinates (4, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 5;
    y = 1; // Cell with coordinates (5, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 6;
    y = 1; // Cell with coordinates (6, 1)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 7;
    y = 1; // Cell with coordinates (7, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 8;
    y = 1; // Cell with coordinates (8, 1)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 9;
    y = 1; // Cell with coordinates (9, 1)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 2; // Cell with coordinates (0, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 2; // Cell with coordinates (1, 2)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 2; // Cell with coordinates (2, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 2; // Cell with coordinates (3, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 2; // Cell with coordinates (4, 2)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 2; // Cell with coordinates (5, 2)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 2; // Cell with coordinates (6, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 2; // Cell with coordinates (7, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 2; // Cell with coordinates (8, 2)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 2; // Cell with coordinates (9, 2)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 3; // Cell with coordinates (0, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 3; // Cell with coordinates (1, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 3; // Cell with coordinates (2, 3)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = 3; // Cell with coordinates (3, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 3; // Cell with coordinates (4, 3)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 5;
    y = 3; // Cell with coordinates (5, 3)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 3; // Cell with coordinates (6, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 7;
    y = 3; // Cell with coordinates (7, 3)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 3; // Cell with coordinates (8, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 3; // Cell with coordinates (9, 3)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 4; // Cell with coordinates (0, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 4; // Cell with coordinates (1, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = 4; // Cell with coordinates (2, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = 4; // Cell with coordinates (3, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 4; // Cell with coordinates (4, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 4; // Cell with coordinates (5, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 4; // Cell with coordinates (6, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 7;
    y = 4; // Cell with coordinates (7, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 8;
    y = 4; // Cell with coordinates (8, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 4; // Cell with coordinates (9, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 0;
    y = 5; // Cell with coordinates (0, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 5; // Cell with coordinates (1, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 5; // Cell with coordinates (2, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 5; // Cell with coordinates (3, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 5; // Cell with coordinates (4, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 5; // Cell with coordinates (5, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 5; // Cell with coordinates (6, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 5; // Cell with coordinates (7, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 5; // Cell with coordinates (8, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 5; // Cell with coordinates (9, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 6; // Cell with coordinates (0, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 6; // Cell with coordinates (1, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = 6; // Cell with coordinates (2, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = 6; // Cell with coordinates (3, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 6; // Cell with coordinates (4, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 6; // Cell with coordinates (5, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 6; // Cell with coordinates (6, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 7;
    y = 6; // Cell with coordinates (7, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 8;
    y = 6; // Cell with coordinates (8, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 6; // Cell with coordinates (9, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 0;
    y = 7; // Cell with coordinates (0, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 7; // Cell with coordinates (1, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 7; // Cell with coordinates (2, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 7; // Cell with coordinates (3, 7)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 7; // Cell with coordinates (4, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 5;
    y = 7; // Cell with coordinates (5, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 7; // Cell with coordinates (6, 7)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 7; // Cell with coordinates (7, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 7; // Cell with coordinates (8, 7)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 7; // Cell with coordinates (9, 7)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 8; // Cell with coordinates (0, 8)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 8; // Cell with coordinates (1, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = 8; // Cell with coordinates (2, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 8; // Cell with coordinates (3, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 8; // Cell with coordinates (4, 8)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 8; // Cell with coordinates (5, 8)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 8; // Cell with coordinates (6, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 8; // Cell with coordinates (7, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 8;
    y = 8; // Cell with coordinates (8, 8)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 8; // Cell with coordinates (9, 8)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 9; // Cell with coordinates (0, 9)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 9; // Cell with coordinates (1, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 9; // Cell with coordinates (2, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = 9; // Cell with coordinates (3, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 9; // Cell with coordinates (4, 9)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 5;
    y = 9; // Cell with coordinates (5, 9)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 9; // Cell with coordinates (6, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 7;
    y = 9; // Cell with coordinates (7, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 9; // Cell with coordinates (8, 9)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 9; // Cell with coordinates (9, 9)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = 10; // Cell with coordinates (0, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = 10; // Cell with coordinates (1, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = 10; // Cell with coordinates (2, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = 10; // Cell with coordinates (3, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = 10; // Cell with coordinates (4, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = 10; // Cell with coordinates (5, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 6;
    y = 10; // Cell with coordinates (6, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 7;
    y = 10; // Cell with coordinates (7, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 8;
    y = 10; // Cell with coordinates (8, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 9;
    y = 10; // Cell with coordinates (9, 10)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    for (int i = 0; i < p->sizeX; i++) { // Remove food where necessary
        for (int j = 0; j < p->sizeY; j++) {
            if ((i == 0 && (j == 4 || j == 6)) || (i == 1 && (j == 4 || j == 6)) || (i == p->sizeX - 1 && (j == 4 || j == 6)) || (i == p->sizeX - 2 && (j == 4 || j == 6)) || IsCellIsolated(*p, i, j)) {
                for (int k = 0; k < 5; k++) {
                    p->self[i][j].foods[k] = 0;
                    p->numFoods--;
                }
                continue;
            }
            if (!*p->self[i][j].l_top) {
                p->self[i][j].foods[0] = 0;
                p->numFoods--;
            }
            if (!*p->self[i][j].l_right) {
                p->self[i][j].foods[1] = 0;
                p->numFoods--;
            }
            if (!*p->self[i][j].l_bottom) {
                p->self[i][j].foods[2] = 0;
                p->numFoods--;
            }
            if (!*p->self[i][j].l_left) {
                p->self[i][j].foods[3] = 0;
                p->numFoods--;
            }
        }
    }
}

void MazifyPlane(plane *p, int showProcess) {
//	*p->self[midi][midj].l_left = *p->self[midi][midj].l_top = *p->self[midi][midj].l_right = 1;
//	*p->self[midi][midj].l_bottom = 1;
//	*p->self[midi - 1][midj].l_right = *p->self[midi - 1][midj].l_bottom = 1;
//	*p->self[midi - 1][midj].l_left = *p->self[midi - 1][midj].l_top = 0;
//	*p->self[midi + 1][midj].l_left = *p->self[midi + 1][midj].l_bottom = 1;
//	*p->self[midi + 1][midj].l_right = *p->self[midi + 1][midj].l_top = 0;
//	*p->self[midi + 1][midj + 1].l_left = *p->self[midi + 1][midj + 1].l_top = 1;
//	*p->self[midi + 1][midj + 1].l_right = *p->self[midi + 1][midj + 1].l_bottom = 0;
//	*p->self[midi][midj + 1].l_left = *p->self[midi][midj].l_top = *p->self[midi][midj].l_right = 1;
//	*p->self[midi][midj + 1].l_bottom = 0;
//	*p->self[midi - 1][midj + 1].l_right = *p->self[midi + 1][midj + 1].l_top = 1;
//	*p->self[midi - 1][midj + 1].l_left = *p->self[midi - 1][midj + 1].l_bottom = 0;
//	*p->self[midi - 1][midj - 1].l_right = 1;
//	*p->self[midi][midj - 1].l_right = *p->self[midi - 1][midj - 1].l_left = 1;
// Ghost house


    // Below we will link the cells for around the portals. After this, 14 cells will be linked
    int x, y;

    int midi = p->sizeX / 2;
    int midj = (p->sizeY) / 2;
    x = 0;
    y = midj - 1; // Cell with coordinates (0, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj - 1; // Cell with coordinates (1, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

//    x = 2;
//    y = midj - 1; // Cell with coordinates (2, 4)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 0;
//    *p->self[x][y].l_right = 0;

//    x = p->sizeX - 3;
//    y = midj - 1; // Cell with coordinates (7, 4)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 0;
//    *p->self[x][y].l_right = 0;

    x = p->sizeX - 2;
    y = midj - 1; // Cell with coordinates (8, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj - 1; // Cell with coordinates (9, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 0;
    y = midj; // Cell with coordinates (0, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj; // Cell with coordinates (1, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

//    x = 2;
//    y = midj; // Cell with coordinates (2, 5)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 1;
//    *p->self[x][y].l_right = 1;



//    x = p->sizeX - 3;
//    y = midj; // Cell with coordinates (7, 5)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 1;
//    *p->self[x][y].l_right = 1;

    x = p->sizeX - 2;
    y = midj; // Cell with coordinates (8, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj; // Cell with coordinates (9, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = midj + 1; // Cell with coordinates (0, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj + 1; // Cell with coordinates (1, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

//    x = 2;
//    y = midj + 1; // Cell with coordinates (2, 6)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 0;
//    *p->self[x][y].l_right = 0;


//    x = p->sizeX - 3;
//    y = midj + 1; // Cell with coordinates (7, 6)
//    *p->self[x][y].l_bottom = 1;
//    *p->self[x][y].l_top = 1;
//    *p->self[x][y].l_left = 0;
//    *p->self[x][y].l_right = 0;

    x = p->sizeX - 2;
    y = midj + 1; // Cell with coordinates (8, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj + 1; // Cell with coordinates (9, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;


    int total = p->sizeX * p->sizeY;
    coord *stack;
    coord begining;
    stack = (coord *)malloc(sizeof(coord) * total);
    begining.i = p->sizeX - 1;
    randomize();
    begining.j = p->sizeY - 1;
    int pos = 0, lastPos = 0;
    int numVisited = 1; // (number of cells visited)
    stack[pos] = begining;

    SDL_Event ev;
    do {
        // Our event listener to avoid the computer thinking that the program is not responding
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                break;
            }
            if (ev.type == SDL_KEYDOWN) {
                showProcess = 0;
                break;
            }
        }

        int i, j;
        i = stack[pos].i;
        j = stack[pos].j;
        lastPos = pos;
        if (NumIsolatedAround(*p, i, j)) {
            int choice;
            choice = RandlinkCellToIsolated(p, i, j);
            switch (choice) {
                case UP:
                    j--;
                    break;
                case DOWN:
                    j++;
                    break;
                case LEFT:
                    i--;
                    break;
                case RIGHT:
                    i++;
                    break;
                default:
                    // TODO
                    break;
            }
            pos++;
            stack[pos].i = i;
            stack[pos].j = j;
            numVisited++;
//			if (showProcess) {
//            SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 0);
//            SDL_RenderClear(p->renderer);
//            DrawCell(p->self[stack[lastPos].i][stack[lastPos].j], p->cellSize, p->cellSpacing, p->borderCol, p->cellCol);
//            DrawCell(p->self[stack[pos].i][stack[pos].j], p->cellSize, p->cellSpacing, p->cellCol, p->borderCol);
//            DrawPlane(*p);
//            SDL_RenderPresent(p->renderer);
//				SDL_Delay(100);
//			}
        } else {
            pos--;
        }
        if (showProcess != 0) {
            SDL_SetRenderDrawColor(p->renderer, 0, 0, 0, 0);
            SDL_RenderClear(p->renderer);
            SDL_SetRenderDrawColor(p->renderer, 255 / 2, 255 / 2, 255 / 2, 0);
            SDL_RenderFillRect(p->renderer, &p->self[stack[lastPos].i][stack[lastPos].j].rect);
            SDL_SetRenderDrawColor(p->renderer, 255, 255, 255, 0);
            SDL_RenderFillRect(p->renderer, &p->self[stack[pos].i][stack[pos].j].rect);
//        DrawCell(p->self[stack[lastPos].i][stack[lastPos].j], p->cellSize, p->cellSpacing, p->borderCol, p->cellCol);
//        DrawCell(p->self[stack[pos].i][stack[pos].j], p->cellSize, p->cellSpacing, p->cellCol, p->borderCol);
            DrawPlane(*p, 0);
//      SDL_SetRenderDrawColor(p->renderer, 255, 255, 255, 0);
//		SDL_RenderFillRect(p->renderer, &p->self[stack[lastPos].i][stack[lastPos].j].rect);
            SDL_RenderPresent(p->renderer);
            SDL_Delay(50);
        }
//        // printf("%d\n", total - numVisited);
    } while (numVisited != total - 14); // There are 14 cells which were linked by default
    // Ghost house
//    int x, y;

//    int midi = p->sizeX / 2;
//    int midj = (p->sizeY) / 2;


    // Below draw the portals again but now we link them to the rest of the maze
    // and we also draw a particular shape for the middle of the maze

    x = 0;
    y = midj - 1; // Cell with coordinates (0, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj - 1; // Cell with coordinates (1, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = midj - 1; // Cell with coordinates (2, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = midi - 2;
    y = midj - 1; // Cell with coordinates (3, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = midi - 1;
    y = midj - 1; // Cell with coordinates (4, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = midj;
    y = midj - 1; // Cell with coordinates (5, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = midi + 1;
    y = midj - 1; // Cell with coordinates (6, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = p->sizeX - 3;
    y = midj - 1; // Cell with coordinates (7, 4)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = p->sizeX - 2;
    y = midj - 1; // Cell with coordinates (8, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj - 1; // Cell with coordinates (9, 4)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 0;
    y = midj; // Cell with coordinates (0, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj; // Cell with coordinates (1, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 2;
    y = midj; // Cell with coordinates (2, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 3;
    y = midj; // Cell with coordinates (3, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = midj; // Cell with coordinates (4, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = midj; // Cell with coordinates (5, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = midi + 1;
    y = midj; // Cell with coordinates (6, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 3;
    y = midj; // Cell with coordinates (7, 5)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 2;
    y = midj; // Cell with coordinates (8, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj; // Cell with coordinates (9, 5)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 0;
    y = midj + 1; // Cell with coordinates (0, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 1;
    y = midj + 1; // Cell with coordinates (1, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = 2;
    y = midj + 1; // Cell with coordinates (2, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = 3;
    y = midj + 1; // Cell with coordinates (3, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = 4;
    y = midj + 1; // Cell with coordinates (4, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = 5;
    y = midj + 1; // Cell with coordinates (5, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    x = midi + 1;
    y = midj + 1; // Cell with coordinates (6, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 0;

    x = p->sizeX - 3;
    y = midj + 1; // Cell with coordinates (7, 6)
    *p->self[x][y].l_bottom = 1;
    *p->self[x][y].l_top = 1;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 0;

    x = p->sizeX - 2;
    y = midj + 1; // Cell with coordinates (8, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 0;
    *p->self[x][y].l_right = 1;

    x = p->sizeX - 1;
    y = midj + 1; // Cell with coordinates (9, 6)
    *p->self[x][y].l_bottom = 0;
    *p->self[x][y].l_top = 0;
    *p->self[x][y].l_left = 1;
    *p->self[x][y].l_right = 1;

    // Remove dead ends
    // the long condition in the if statements below is to make sure we do not interfare with the
    // portals of the mase and the horrizontal middle of the game map

    // Remove horizontal dead ends
    for (int i = 0; i < p->sizeX; i++) {
        for (int j = 0; j < p->sizeY; j++) {
            if ((NumLinks(*p, i, j) == 1 || IsCellIsolated(*p, i, j)) && !((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 2 && (j == midj - 1 || j == midj + 1)) || j == midj )) {
                if (!(*p->self[i][j].l_left) && i > 0) {
                    *p->self[i][j].l_left = 1;
                }
                if (!(*p->self[i][j].l_right) && i < p->sizeX - 1) {
                    *p->self[i][j].l_right = 1;
                }
            }
        }
    }

    // Remove vertical dead ends. Here we will treat cells above the middle and cells below
    // the middle of the maze differently not to interfere will cells of portals
    for (int i = 0; i < p->sizeX; i++) {
        for (int j = 0; j < p->sizeY; j++) {
            if ((NumLinks(*p, i, j) == 1 || IsCellIsolated(*p, i, j)) && !((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 2 && (j == midj - 1 || j == midj + 1)) /*|| j == midj*/ )) {
                if (!(*p->self[i][j].l_bottom) && j < p->sizeY - 1 && j > midj) {
                    *p->self[i][j].l_bottom = 1;
                    continue;
                }
                if (!(*p->self[i][j].l_top) && j > 0 && j < midj) {
                    *p->self[i][j].l_top = 1;
                    continue;
                }
                if (!(*p->self[i][j].l_top) && j > midj) {
                    *p->self[i][j].l_top = 1;
                    continue;
                }
                if (!(*p->self[i][j].l_bottom) && j < midj) {
                    *p->self[i][j].l_bottom = 1;
                    continue;
                }
            }
        }
    }

    // Remove dead ends and isolated squares which may appear
    for (int i = 3; i < p->sizeX - 4; i++) {
        for (int j = 3; j < p->sizeY - 4; j++) {
            if (NumLinks(*p, i, j) == 1) {
                if (!(*p->self[i][j].l_left)) {
                    *p->self[i][j].l_left = 1;
                }
                if (!(*p->self[i][j].l_right)) {
                    *p->self[i][j].l_right = 1;
                }
                if (!(*p->self[i][j].l_top)) {
                    *p->self[i][j].l_top = 1;
                }
                if (!(*p->self[i][j].l_bottom)) {
                    *p->self[i][j].l_bottom = 1;
                }
            }
            if(IsCellIsolated(*p, i, j)){
				RandlinkCell(p, i, j);
			}
        }
    }

    // Remove food where necessary
    for (int i = 0; i < p->sizeX; i++) {
        for (int j = 0; j < p->sizeY; j++) {
            // Remove all the food in some particular cells or isolated cells if they appear
            if ((i == 0 && (j == midj - 1 || j == midj + 1)) || (i == 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 1 && (j == midj - 1 || j == midj + 1)) || (i == p->sizeX - 2 && (j == midj - 1 || j == midj + 1)) || IsCellIsolated(*p, i, j)) {
                for (int k = 0; k < 5; k++) {
                    p->self[i][j].foods[k] = 0;
                    p->numFoods--;
                }
                continue;
            }
            if (!*p->self[i][j].l_top) { // Remove the top food if not linked up
                p->self[i][j].foods[0] = 0;
                p->numFoods--; // Reduce the amount of food in the maze
            }
            if (!*p->self[i][j].l_right) { // Remove the right food if not linked right
                p->self[i][j].foods[1] = 0;
                p->numFoods--; // Reduce the amount of food in the maze
            }
            if (!*p->self[i][j].l_bottom) { // Remove the down food if not linked down
                p->self[i][j].foods[2] = 0;
                p->numFoods--; // Reduce the amount of food in the maze
            }
            if (!*p->self[i][j].l_left) { // Remove the left food if not linked left
                p->self[i][j].foods[3] = 0;
                p->numFoods--; // Reduce the amount of food in the maze
            }
        }
    }
//	*p->self[midi][midj].l_left = *p->self[midi][midj].l_top = *p->self[midi][midj].l_right = 1;
//		*p->self[midi][midj].l_bottom = 1;
//		*p->self[midi - 1][midj].l_right = *p->self[midi - 1][midj].l_bottom = 1;
//		*p->self[midi - 1][midj].l_left = *p->self[midi - 1][midj].l_top = 0;
//		*p->self[midi + 1][midj].l_left = *p->self[midi + 1][midj].l_bottom = 1;
//		*p->self[midi + 1][midj].l_right = *p->self[midi + 1][midj].l_top = 0;
//		*p->self[midi + 1][midj + 1].l_left = *p->self[midi + 1][midj + 1].l_top = 1;
//		*p->self[midi + 1][midj + 1].l_right = *p->self[midi + 1][midj + 1].l_bottom = 0;
//		*p->self[midi][midj + 1].l_left = *p->self[midi][midj].l_top = *p->self[midi][midj].l_right = 1;
//		*p->self[midi][midj + 1].l_bottom = 0;
//		*p->self[midi - 1][midj + 1].l_right = *p->self[midi + 1][midj + 1].l_top = 1;
//		*p->self[midi - 1][midj + 1].l_left = *p->self[midi - 1][midj + 1].l_bottom = 0;
//		*p->self[midi - 1][midj - 1].l_right = 1;
//		*p->self[midi][midj - 1].l_right = *p->self[midi - 1][midj - 1].l_left = 1;
//
//
//	for (int m = 0; m < 10; m++) {
//		for (int i = 0; i < p->sizeX; i++) {
//			for (int j = 0; j < p->sizeY; j++) {
//				if (NumLinks(*p, i, j) == 1) {
//					RandlinkCell(p, i, j);
//				}
//			}
//		}
//		for (int i = 1; i < p->sizeX; i += 3) {
//			for (int j =  1; j < p->sizeY; j += 4) {
//				if (NumLinks(*p, i, j) == 2) {
//					RandlinkCell(p, i, j);
//				}
//			}
//		}
//
//	}
}

path shortPath(plane p, coord start, coord end) {
    /*
      Implementation of the Breadth first search algorithm.
      Vague explanation:
    		There is an array of coordinates which have been visited called visited. In the begining, it has the starting cell only.
    		In a while loop, we go through all  the cells in
    */
    coord *justVisited;
    coord *visited;
    coord *trip;
    if (start.i == end.i && start.j == end.j) { // If start and end are the same
        trip = (coord *)malloc(sizeof(coord) * 2);
        trip[0].i = start.i;
        trip[0].j = start.j;
        coord t = {.i = -1, .j = -1};
        trip[1] = t;
//		trip[1].i = start.i;
//		trip[1].j = start.j;
        path s;
        s.steps = trip;
        s.len = 1;
        return s;
    }
    justVisited = (coord *)malloc(sizeof(coord) * p.sizeX * p.sizeY); // Coordinated of cells just visited in one iteration. Allocate much memory
    visited = (coord *)malloc(sizeof(coord) * p.sizeX * p.sizeY); // Coordinated of visited. Allocate much memory

    int jv = 0; // length of justVisited
    int v = 0; // length if visited
    int steps = 0; // Number of steps done
    p.self[start.i][start.j].timesVisited = 1; // Say that the start has already been visited
    p.self[start.i][start.j].distance = steps;
    visited[v].i = start.i; // The first cell visited is the start cell
    visited[v].j = start.j;
    v++;
    int flag = 1;

    do {
        jv = 0;
        int i = 0, j = 0;
        for (int m = 0; m < v; m++) {
            i = visited[m].i;
            j = visited[m].j;
            if (i == end.i && j == end.j) {
                flag = 0;
                break;
            }
            if (*p.self[i][j].l_bottom && p.self[i][j + 1].timesVisited == 0) {
                p.self[i][j + 1].timesVisited = 1;
                justVisited[jv].i = i;
                justVisited[jv].j = j + 1;
                jv++;
            }
            if (*p.self[i][j].l_top && p.self[i][j - 1].timesVisited == 0) {
                p.self[i][j - 1].timesVisited = 1;
                justVisited[jv].i = i;
                justVisited[jv].j = j - 1;
                jv++;
            }
            if (*p.self[i][j].l_left && p.self[i - 1][j].timesVisited == 0) {
                p.self[i - 1][j].timesVisited = 1;
                justVisited[jv].i = i - 1;
                justVisited[jv].j = j;
                jv++;
            }
            if (*p.self[i][j].l_right && p.self[i + 1][j].timesVisited == 0) {
                p.self[i + 1][j].timesVisited = 1;
                justVisited[jv].i = i + 1;
                justVisited[jv].j = j;
                jv++;
            }
        }

        v = jv;
        steps++;
        for (int m = 0; m < v; m++) {
            visited[m] = justVisited[m];
            // Draw cells just visited in blue
            p.self[visited[m].i][visited[m].j].distance = steps;
        }
    } while (flag);
    // Show cell distance on each cell
    //	for (int i = 0; i < p.sizeX; i++) {
    //		for (int j = 0; j < p.sizeY; j++) {
    //			gotoxy(p.self[i][j].x + p.cellSize, p.self[i][j].y + p.cellSize / 2);
    //			printf("%d", p.self[i][j].distance);
    //		}
    //	}
    trip = (coord *)malloc(sizeof(coord) * steps);

    int i, j, n = steps, a, b;
    i = a = end.i;
    j = b = end.j;

    do {
        steps--;
        trip[steps].i = i;
        trip[steps].j = j;
        int d = p.self[i][j].distance;

        if (*p.self[i][j].l_top) {
            if (p.self[i][j - 1].distance == d - 1) {
                b--;
                goto end;
            }
        }
        if (*p.self[i][j].l_bottom) {
            if (p.self[i][j + 1].distance == d - 1) {
                b++;
                goto end;
            }
        }
        if (*p.self[i][j].l_left) {
            if (p.self[i - 1][j].distance == d - 1) {
                a--;
                goto end;
            }
        }
        if (*p.self[i][j].l_right) {
            if (p.self[i + 1][j].distance == d - 1) {
                a++;
                goto end;
            }
        }

end:
        i = a;
        j = b;
        //		DrawCell(p.self[i][j], p.cellSize, p.cellSpacing, 1, p.borderCol);
        //		gotoxy(p.self[i][j].x + p.cellSize, p.self[i][j].y + p.cellSize / 2);
        //		// printf("%d", p.self[i][j].distance);
        d--;
    } while (steps > 0);

    for (int i = 0; i < p.sizeX; i++) {
        for (int j = 0; j < p.sizeY; j++) {
            p.self[i][j].timesVisited = 0;
            p.self[i][j].distance = -1;
        }
    }
    path solution;
    solution.steps = trip;
    solution.len = n - 1;
    return solution;
}
path DepthFirstSearch(plane p, coord start, coord end, int numberOfTimesVisited) {
    path trip;
    trip.steps = (coord *)malloc(sizeof(coord) * p.sizeX * p.sizeY);
    SDL_Color col = {0, 100, 250, 0};
    DrawCell(p.self[start.i][start.j], p.cellSize, p.cellSpacing, col, p.borderCol);
    DrawCell(p.self[end.i][end.j], p.cellSize, p.cellSpacing, col, p.borderCol);

    int pos = 0;
    trip.steps[pos] = start;
    p.self[start.i][start.j].timesVisited++;
    do {
        int i = trip.steps[pos].i;
        int j = trip.steps[pos].j;

        if (*p.self[i][j].l_bottom && p.self[i][j + 1].timesVisited == numberOfTimesVisited) {
            p.self[i][j + 1].timesVisited++;
            j = j + 1;
        } else if (*p.self[i][j].l_left && p.self[i - 1][j].timesVisited == numberOfTimesVisited) {
            p.self[i - 1][j].timesVisited++;
            i = i - 1;
        } else if (*p.self[i][j].l_top && p.self[i][j - 1].timesVisited == numberOfTimesVisited) {
            p.self[i][j - 1].timesVisited++;
            j = j - 1;
        } else if (*p.self[i][j].l_right && p.self[i + 1][j].timesVisited == numberOfTimesVisited) {
            p.self[i + 1][j].timesVisited++;
            i = i + 1;
        } else {
            p.self[i][j].timesVisited++;
//			DrawCell(p.self[trip.steps[pos].i][trip.steps[pos].j], p.cellSize, p.cellSpacing, p.cellCol, p.borderCol);
            pos--;
            continue;
        }
//		DrawCell(p.self[i][j], p.cellSize, p.cellSpacing, col, p.borderCol);

        pos++;
        trip.steps[pos].i = i;
        trip.steps[pos].j = j;

    } while (trip.steps[pos].i != end.i || trip.steps[pos].j != end.j);
    for (int i = 0; i < p.sizeX; i++) {
        for (int j = 0; j < p.sizeY; j++) {
            p.self[i][j].timesVisited = numberOfTimesVisited;
        }
    }
    trip.len = pos + 1;
    return trip;
}

double distanceBtw(cell a, cell b) {
    // Uses Pythagoras theorem to calculate the distance between two cells.
    return sqrt((a.rect.x - b.rect.x) * (a.rect.x - b.rect.x) + (a.rect.y - b.rect.y) * (a.rect.y - b.rect.y));
}
