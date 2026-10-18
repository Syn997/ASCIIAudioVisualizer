#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h> // Required for C99 - C17
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

//default bar count and height
#define BAR_COUNT  32
#define BAR_HEIGHT 12


// Shared data between audio capture and renderer.
static float audio_levels[BAR_COUNT];

//return color and intensity based on audio feedback loop
static WORD get_color(ULONGLONG ms)
{
	switch ((ms / 200) % 6)
	{
	case 0: return FOREGROUND_RED | FOREGROUND_INTENSITY;
	case 1: return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	case 2: return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	case 3: return FOREGROUND_RED | FOREGROUND_GREEN |
		FOREGROUND_INTENSITY;
	case 4: return FOREGROUND_RED | FOREGROUND_BLUE |
		FOREGROUND_INTENSITY;
	default: return FOREGROUND_GREEN | FOREGROUND_BLUE |
		FOREGROUND_INTENSITY;
	}
}

//called by the renderer in main
static void draw_visualizer(ULONGLONG ms)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (console == INVALID_HANDLE_VALUE || console == NULL)
		return;

	COORD origin = { 0, 0 };
	SetConsoleCursorPosition(console, origin);
	SetConsoleTextAttribute(console, get_color(ms));

	for (int row = BAR_HEIGHT; row >= 1; --row)
	{
		for (int col = 0; col < BAR_COUNT; ++col)
		{
			int height = (int)(audio_levels[col] * BAR_HEIGHT);

			if (height > BAR_HEIGHT) height = BAR_HEIGHT;
			if (height < 0) height = 0;

			putchar(height >= row ? '#' : ' ');
			putchar(' ');
		}
		putchar('\n');
	}

	SetConsoleTextAttribute(
		console,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);

	printf("Elapsed: %llu ms\n",
		(unsigned long long)ms);

	fflush(stdout);
}

int main(void)
{
	// Disable console QuickEdit mode
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	GetConsoleMode(hInput, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	mode |= ENABLE_EXTENDED_FLAGS;
	SetConsoleMode(hInput, mode);

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD written;
	COORD origin = { 0, 0 };
	FillConsoleOutputCharacterA(
		console, ' ', 2000, origin, &written);

	while (1)
	{
		draw_visualizer();
		Sleep(30);
	}

	return 0;
}