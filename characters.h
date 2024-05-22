#include "stdio.h"
#include "conio.h"
#include "windows.h"
#include "stdlib.h"
#include "time.h"

#define TOP_THIN 196
#define TOP_DOUBLE 205

#define TOP_MID_THIN 194
#define TOP_MID_DOUBLE 203

#define RIGHT_MID_THIN 180
#define RIGHT_MID_DOUBLE 185

#define LEFT_MID_THIN 195
#define LEFT_MID_DOUBLE 204

#define BOTTOM_THIN 196
#define BOTTOM_DOUBLE 205

#define BOTTOM_MID_THIN 193
#define BOTTOM_MID_DOUBLE 202

#define RIGHT_TOP_THIN 191
#define RIGHT_TOP_DOUBLE 187

#define TOP_LEFT_THIN 218
#define TOP_LEFT_DOUBLE 201

#define BOTTOM_LEFT_THIN 192
#define BOTTOM_LEFT_DOUBLE 200

#define BOTTOM_RIGHT_THIN 192
#define BOTTOM_RIGHT_DOUBLE 217

#define CROSS_THIN 197
#define CROSS_DOUBLE 206

#define VERT_THIN 179
#define VERT_DOUBLE 186

#define SQUARE_MID 254
#define SQUARE_TOP 223
#define SQUARE_BOTTOM 220
#define FULL 219
#define EMPTY 32

#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

time_t ref;


void randomize() {
	if(rand()%2){
		ref += time(NULL) +  rand();
	}else{
		ref += time(NULL) -  rand();
	}
	srand(ref);
}


void gotoxy(int x, int y) {
	HANDLE hConsoleOutput;
	COORD dwCursorPosition;

	dwCursorPosition.X = x;
	dwCursorPosition.Y = y;
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);

	randomize();
}
