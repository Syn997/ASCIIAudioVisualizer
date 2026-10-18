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

// Called by the renderer:
void draw_visualizer(void)
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		int height = (int)(audio_levels[i] * BAR_HEIGHT);

		if (height > BAR_HEIGHT)
			height = BAR_HEIGHT;
		if (height < 0)
			height = 0;

		// Render this bar using your existing console code.

		static ULONGLONG start = 0;

		if (start == 0)
			start = GetTickCount64();

		ULONGLONG ms = GetTickCount64() - start;

		// Change color every 200 milliseconds.
		WORD color = (WORD)(FOREGROUND_RED |
			FOREGROUND_INTENSITY);

		switch ((ms / 200) % 6)
		{
		case 0: color = FOREGROUND_RED |
			FOREGROUND_INTENSITY; break;
		case 1: color = FOREGROUND_GREEN |
			FOREGROUND_INTENSITY; break;
		case 2: color = FOREGROUND_BLUE |
			FOREGROUND_INTENSITY; break;
		case 3: color = FOREGROUND_RED |
			FOREGROUND_GREEN |
			FOREGROUND_INTENSITY; break;
		case 4: color = FOREGROUND_RED |
			FOREGROUND_BLUE |
			FOREGROUND_INTENSITY; break;
		case 5: color = FOREGROUND_GREEN |
			FOREGROUND_BLUE |
			FOREGROUND_INTENSITY; break;
		}

		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (console == INVALID_HANDLE_VALUE ||
			console == NULL)
			return;

		SetConsoleTextAttribute(console, color);

		COORD origin = { 0, 0 };
		SetConsoleCursorPosition(console, origin);

		for (int row = BAR_HEIGHT; row > 0; --row)
		{
			for (int col = 0; col < BAR_COUNT; ++col)
			{
				// Simple deterministic animation.
				int phase = (int)((ms / 80 + col * 3) % 12);
				int height = phase;

				putchar(height >= row ? '#' : ' ');
				putchar(' ');
			}

			putchar('\n');
		}

		SetConsoleTextAttribute(console,
			FOREGROUND_RED | FOREGROUND_GREEN |
			FOREGROUND_BLUE);

		printf("\r\nElapsed: %llu ms\n",
			(unsigned long long)ms);

		fflush(stdout);
	}
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