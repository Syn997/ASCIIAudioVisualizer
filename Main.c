#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h> // Required for C99 - C17
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#define BAR_COUNT  32
#define BAR_HEIGHT 12



int main(void)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD written;
	COORD origin = { 0, 0 };
	FillConsoleOutputCharacterA(
		console, ' ', 2000, origin, &written);

	while (1)
	{
		printf("test");
		//draw_visualizer();
		Sleep(30);
	}

	return 0;
}