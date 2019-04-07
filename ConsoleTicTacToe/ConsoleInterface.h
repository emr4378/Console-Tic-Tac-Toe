#pragma once

#include <cstdint>
#include <Windows.h>

namespace tictactoe
{
	enum class ConsoleColor
	{
		Black =			0,

		DarkBlue =		FOREGROUND_BLUE,
		DarkGreen =		FOREGROUND_GREEN,
		DarkCyan =		DarkGreen | DarkBlue,
		DarkRed =		FOREGROUND_RED,
		DarkMagenta =	DarkRed | DarkBlue,
		DarkYellow =	DarkRed | DarkGreen,
		DarkGray =		DarkRed | DarkGreen | DarkBlue,

		LightGray =		FOREGROUND_INTENSITY,
		LightBlue =		LightGray | DarkBlue,
		LightGreen =	LightGray | DarkGreen,
		LightCyan =		LightGray | DarkCyan,
		LightRed =		LightGray | DarkRed,
		LightMagenta =	LightGray | DarkMagenta,
		LightYellow =	LightGray | DarkYellow,

		White =			LightGray | DarkGray
	};

	class ConsoleInterface
	{
	public:
		ConsoleInterface();

		void Initialize();
		void Terminate();
		void Update();

		void DrawChar(char c, int16_t x, int16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor);
		void DrawString(const char* str, int16_t x, int16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor);
		void DrawPixel(int16_t x, int16_t y, const ConsoleColor& color);
		void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const ConsoleColor& color);
		void DrawCircle(int16_t x, int16_t y, int16_t r, const ConsoleColor& color);
		void DrawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const ConsoleColor& color, const ConsoleColor& fillColor);
		void Clear();

		int16_t GetWidth() const { return _currentWidth; }
		int16_t GetHeight() const { return _currentHeight; }

	private:
		struct CachedInfo
		{
			DWORD stdInMode;

			CONSOLE_SCREEN_BUFFER_INFO stdOutScreenBufferInfo;
			CONSOLE_FONT_INFOEX stdOutFontInfo;
		};

		bool _isInitialized;

		HANDLE _stdInHandle;
		HANDLE _stdOutHandle;
		CachedInfo _cachedInfo;

		int16_t _currentWidth;
		int16_t _currentHeight;
	};
}
