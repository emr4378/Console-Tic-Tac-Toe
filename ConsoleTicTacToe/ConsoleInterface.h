#pragma once

#include <cstdint>
#include <functional>

#define WIN32_LEAN_AND_MEAN
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

	struct ConsoleCoord
	{
		uint16_t x;
		uint16_t y;
	};

	struct ConsoleSize
	{
		uint16_t width;
		uint16_t height;
	};

	struct ConsoleRect
	{
		uint16_t left;
		uint16_t top;
		uint16_t right;
		uint16_t bottom;

		ConsoleSize GetSize() const;
	};

	// Reference: https://docs.microsoft.com/en-us/windows/console/
	class ConsoleInterface
	{
	public:
		typedef std::function<void(const KEY_EVENT_RECORD&)> KeyEventCallback;
		typedef std::function<void(const MOUSE_EVENT_RECORD&)> MouseEventCallback;
		typedef std::function<void(const ConsoleSize&)> ResizeEventCallback;

		ConsoleInterface();
		virtual ~ConsoleInterface();

		void SetCallbacks(KeyEventCallback keyCb, MouseEventCallback mouseCb, ResizeEventCallback resizeCb);

		void Update();

		void SetMinBufferSize(const ConsoleSize& size);
		const ConsoleSize& GetMinBufferSize() const { return _minBufferSize; }
		const ConsoleSize& GetCurrentBufferSize() const { return _currentBufferSize; }
		const ConsoleRect& GetCurrentBufferViewportRect() const { return _currentBufferViewportRect; }

		void DrawChar(char c, uint16_t x, uint16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor);
		void DrawString(const char* str, uint16_t x, uint16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor);
		void DrawPixel(uint16_t x, uint16_t y, const ConsoleColor& color);
		void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const ConsoleColor& color);
		void DrawCircle(uint16_t x, uint16_t y, uint16_t r, const ConsoleColor& color);
		void DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const ConsoleColor& color, const ConsoleColor& fillColor);
		void Clear();

	private:
		void ResizeBuffer(uint16_t w, uint16_t h);

	private:
		struct CachedInfo
		{
			DWORD stdInMode;

			CONSOLE_SCREEN_BUFFER_INFO stdOutScreenBufferInfo;
			CONSOLE_FONT_INFOEX stdOutFontInfo;
		};
		void SaveInitialConsoleState();
		void RestoreInitialConsoleState();

		HANDLE _stdInHandle;
		HANDLE _stdOutHandle;
		CachedInfo _cachedInfo;
		
		KeyEventCallback _keyEventCallback;
		MouseEventCallback _mouseEventCallback;
		ResizeEventCallback _resizeEventCallback;

		ConsoleSize _minBufferSize;
		ConsoleSize _currentBufferSize;
		ConsoleRect _currentBufferViewportRect;
	};
}
