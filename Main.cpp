#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <ksmedia.h>
#include <stdio.h>
#include <psapi.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h> // Required for C99 - C17

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

#define BAR_COUNT  48
#define BAR_HEIGHT 24

#include <stdlib.h>
#include <time.h>

ULONGLONG start_time = GetTickCount64();

void draw_stats(double elapsed_seconds, int bpm)
{
	PROCESS_MEMORY_COUNTERS pmc = {};
	pmc.cb = sizeof(pmc);

	SIZE_T memory_kb = 0;

	if (GetProcessMemoryInfo(
		GetCurrentProcess(), &pmc, sizeof(pmc)))
	{
		memory_kb = pmc.WorkingSetSize / 1024;
	}

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { 0, 0 };
	SetConsoleCursorPosition(console, pos);

	SetConsoleTextAttribute(
		console,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	printf("Elapsed: %02d:%02d:%02d\n",
		(int)(elapsed_seconds / 3600),
		((int)elapsed_seconds / 60) % 60,
		(int)elapsed_seconds % 60);

	printf("Memory:  %zu KB\n", (size_t)memory_kb / 10);
	printf("BPM:     %d\n", bpm);
}

volatile int current_bpm = 10;

void update_bpm(float level)
{
	static float envelope = 0.0f;
	static float previous_envelope = 0.0f;
	static float noise_floor = 0.0f;

	static ULONGLONG last_beat = 0;
	static ULONGLONG last_candidate = 0;

	ULONGLONG now = GetTickCount64();

	// Track the changing audio level.
	envelope = envelope * 0.7f + level * 0.3f;

	// Adapt to the background level.
	noise_floor = noise_floor * 0.995f + envelope * 0.005f;

	// Detect a local rise through an adaptive threshold.
	float threshold = noise_floor + 0.04f;

	bool rising_peak =
		envelope > threshold &&
		previous_envelope <= threshold;

	previous_envelope = envelope;

	if (!rising_peak || now - last_candidate < 250)
		return;

	last_candidate = now;

	if (last_beat != 0)
	{
		ULONGLONG interval = now - last_beat;

		// Accept approximately 50-200 BPM.
		if (interval >= 300 && interval <= 1200)
		{
			int detected_bpm =
				(int)(60000.0 / interval + 0.5);

			// Smooth the estimate.
			if (current_bpm == 120)
				current_bpm = detected_bpm;
			else
				current_bpm =
				(current_bpm * 3 + detected_bpm) / 4;
		}
	}

	last_beat = now;
}

const char* bar_chars = "░▒▓█@$%&*+=xX";

void init_visualizer()
{
	srand((unsigned int)time(NULL));
}

static void setup_console(void)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (console == INVALID_HANDLE_VALUE || console == NULL)
		return;

	// Ensure enough buffer height for 24 rows + status.
	CONSOLE_SCREEN_BUFFER_INFO info;

	if (GetConsoleScreenBufferInfo(console, &info))
	{
		COORD size = info.dwSize;

		if (size.Y < 30)
			size.Y = 30;

		if (size.X < BAR_COUNT * 2)
			size.X = BAR_COUNT * 2;

		SetConsoleScreenBufferSize(console, size);
	}

	// Hide the cursor so it doesn't look like a stray bar.
	CONSOLE_CURSOR_INFO cursorInfo;

	if (GetConsoleCursorInfo(console, &cursorInfo))
	{
		cursorInfo.bVisible = FALSE;
		SetConsoleCursorInfo(console, &cursorInfo);
	}
}


static float audio_levels[BAR_COUNT] = { 0 };
//return color and intensity based on audio feedback loop
static WORD get_color(ULONGLONG ms, int bpm)
{
	if (bpm < 1)
		bpm = 1;

	// Higher BPM = faster color changes
	// 20 BPM  -> change every 2000 ms
	// 40 BPM  -> change every 1000 ms
	// 80 BPM  -> change every  500 ms
	// 160 BPM -> change every  250 ms
	ULONGLONG interval = 60000 / bpm;

	switch ((ms / interval) % 6)
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
	// Reserve rows 0-3 for stats.
	COORD pos = { 0, 5 };
	SetConsoleCursorPosition(console, pos);

	if (console == INVALID_HANDLE_VALUE || console == NULL)
		return;

	// Position cursor at the top-left without scrolling.
	COORD origin = { 0, 0 };
	SetConsoleCursorPosition(console, origin);

	//set color based on ms and calculated bpm 
	SetConsoleTextAttribute(console, get_color(ms, current_bpm));

	// Draw bars from top to bottom.
	for (int row = BAR_HEIGHT; row >= 1; --row)
	{
		for (int col = 0; col < BAR_COUNT; ++col)
		{
			float level = audio_levels[col];

			if (level < 0.0f) level = 0.0f;
			if (level > 1.0f) level = 1.0f;


			//calculate height of the bars based on constants
			int height = (int)(audio_levels[col] * BAR_HEIGHT + 0.5f);

			if (height > BAR_HEIGHT) height = BAR_HEIGHT;
			if (height < 0) height = 0;

			// Solid characters make the bar height visible.
			//use random character from bar_chars instead of #
			//putchar(bar_chars[rand() % 14]);
			putchar(height >= row ? bar_chars[rand() % 14] : ' ');
			putchar(' ');
		}

		putchar('\n');
	}

	// Reset color and clear any leftovers from the previous frame.
	SetConsoleTextAttribute(
		console,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);


	//printf("\r\nElapsed: %llu ms", (unsigned long long)ms);
	// Erase remnants of the previous status line.
	printf("                                      \n");
	fflush(stdout);
}


// Read Audio Feedback Loop using common Windows mix formats: float32 and PCM16.
static float read_sample(
	const BYTE* data,
	UINT32 index,
	bool is_float)
{
	if (is_float)
	{
		float sample;
		memcpy(&sample, data + index * sizeof(float), sizeof(float));

		if (!isfinite(sample))
			return 0.0f;

		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		return sample;
	}

	int16_t sample;
	memcpy(&sample, data + index * sizeof(int16_t), sizeof(int16_t));
	return (float)sample / 32768.0f;
}

int main(void)
{
	setup_console();

	init_visualizer();

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (FAILED(hr))
	{
		printf("CoInitializeEx failed: 0x%08lX\n",
			(unsigned long)hr);
		return 1;
	}

	IMMDeviceEnumerator* enumerator = NULL;
	IMMDevice* device = NULL;
	IAudioClient* client = NULL;
	IAudioCaptureClient* capture = NULL;
	WAVEFORMATEX* format = NULL;
	bool is_float = false;

	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void**)&enumerator);

	if (FAILED(hr)) goto cleanup;

	hr = enumerator->GetDefaultAudioEndpoint(
		eRender, eConsole, &device);

	if (FAILED(hr)) goto cleanup;

	hr = device->Activate(
		__uuidof(IAudioClient), CLSCTX_ALL, NULL,
		(void**)&client);

	if (FAILED(hr)) goto cleanup;

	hr = client->GetMixFormat(&format);
	if (FAILED(hr)) goto cleanup;

	if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
		is_float = (format->wBitsPerSample == 32);
	}
	else if (format->wFormatTag == WAVE_FORMAT_PCM)
	{
		is_float = false;
	}
	else if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE* ext =
			(WAVEFORMATEXTENSIBLE*)format;

		if (IsEqualGUID(ext->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
			is_float = true;
		else if (IsEqualGUID(ext->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
			is_float = false;
		else
		{
			printf("Unsupported audio subtype.\n");
			goto cleanup;
		}
	}
	else
	{
		printf("Unsupported audio format.\n");
		goto cleanup;
	}

	if ((is_float && format->wBitsPerSample != 32) ||
		(!is_float && format->wBitsPerSample != 16))
	{
		printf("This example supports float32 or PCM16 only.\n");
		goto cleanup;
	}

	hr = client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
		1000000, // 100 ms buffer duration, in 100-ns units
		0,
		format,
		NULL);

	if (FAILED(hr)) goto cleanup;

	hr = client->GetService(
		__uuidof(IAudioCaptureClient),
		(void**)&capture);

	if (FAILED(hr)) goto cleanup;

	// Clear the screen once.
	printf("\033[2J");
	SetConsoleTitleA("ASCII Audio Visualizer");

	hr = client->Start();
	if (FAILED(hr)) goto cleanup;

	{
		ULONGLONG start = GetTickCount64();
		bool running = true;

		while (running)
		{
			UINT32 packet_frames = 0;

			hr = capture->GetNextPacketSize(&packet_frames);
			if (FAILED(hr)) break;

			while (packet_frames > 0)
			{
				BYTE* data = NULL;
				UINT32 frames = 0;
				DWORD flags = 0;

				hr = capture->GetBuffer(
					&data, &frames, &flags, NULL, NULL);

				if (FAILED(hr))
				{
					running = false;
					break;
				}

				double sum = 0.0;
				double peak = 0.0;
				UINT32 samples = frames * format->nChannels;

				if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && data)
				{
					for (UINT32 i = 0; i < samples; ++i)
					{
						float s = read_sample(data, i, is_float);
						double a = fabs((double)s);

						sum += a * a;
						if (a > peak) peak = a;
					}
				}

				float rms = samples > 0
					? (float)sqrt(sum / samples)
					: 0.0f;

				// Lower sensitivity
				float level = rms * 4.0f;

				// Keep quiet sounds visible, without amplifying everything into oblivion
				level = sqrtf(level);
				if (level > 1.0f)
					level = 1.0f;

				// Smooth movement
				static float smoothed_level = 0.0f;

				if (level > smoothed_level)
					smoothed_level = smoothed_level * 0.65f + level * 0.35f;
				else
					smoothed_level = smoothed_level * 0.90f + level * 0.10f;

				level = smoothed_level;

				//update the bpm level
				update_bpm(level);

				// Shift history and append the new level.
				for (int i = 0; i < BAR_COUNT - 1; ++i)
					audio_levels[i] = audio_levels[i + 1];

				audio_levels[BAR_COUNT - 1] = level;


				(void)peak; // Available for a future peak-hold display.

				hr = capture->ReleaseBuffer(frames);
				if (FAILED(hr))
				{
					running = false;
					break;
				}

				hr = capture->GetNextPacketSize(&packet_frames);
				if (FAILED(hr))
				{
					running = false;
					break;
				}
			}



			draw_visualizer(GetTickCount64() - start);

			//draw todo
			draw_stats(
				(GetTickCount64() - start_time) / 1000.0,
				current_bpm
			);


			Sleep(10);

			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				running = false;
		}

		client->Stop();
	}

cleanup:
	if (FAILED(hr))
		printf("\nWASAPI error: 0x%08lX\n", (unsigned long)hr);

	if (format) CoTaskMemFree(format);
	if (capture) capture->Release();
	if (client) client->Release();
	if (device) device->Release();
	if (enumerator) enumerator->Release();

	CoUninitialize();

	printf("\nVisualizer stopped.\n");
	return FAILED(hr) ? 1 : 0;
}