#include "characters.h"

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
