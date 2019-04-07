#include "ConsoleInterface.h"

#include "Common.h"

#include <utility>
#include <cassert>
#include <cstdio>
#include <Windows.h>

using namespace tictactoe;

WORD sConsoleColorsToAttributes(ConsoleColor foregroundColor, ConsoleColor backgroundColor)
{
	WORD foreground = static_cast<WORD>(foregroundColor);
	WORD background = static_cast<WORD>(backgroundColor);
	return foreground + (background * 16);
}

ConsoleInterface::ConsoleInterface() :
	_isInitialized(false),
	_stdInHandle(INVALID_HANDLE_VALUE),
	_stdOutHandle(INVALID_HANDLE_VALUE),
	_cachedInfo()
{
}

void ConsoleInterface::Initialize()
{
	if (!_isInitialized)
	{
		_stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
		assert(_stdInHandle != INVALID_HANDLE_VALUE);

		_stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		assert(_stdOutHandle != INVALID_HANDLE_VALUE);

		bool result;

		// Cache the initial console state to be restored on Terminate().
		{
			result = GetConsoleMode(_stdInHandle, &_cachedInfo.stdInMode);
			assert(result);

			result = GetConsoleScreenBufferInfo(_stdOutHandle, &_cachedInfo.stdOutScreenBufferInfo);
			assert(result);

			_cachedInfo.stdOutFontInfo.cbSize = sizeof(_cachedInfo.stdOutFontInfo);
			result = GetCurrentConsoleFontEx(_stdOutHandle, false, &_cachedInfo.stdOutFontInfo);
			assert(result);
		}

		// Change the font to 8x8 Terminal to make our 'pixels' square while keeping the text somewhat legible.
		CONSOLE_FONT_INFOEX fontInfo;
		fontInfo.cbSize = sizeof(fontInfo);
		fontInfo.nFont = 0;
		fontInfo.dwFontSize.X = 8;
		fontInfo.dwFontSize.Y = 8;
		fontInfo.FontFamily = FF_DONTCARE;
		fontInfo.FontWeight = FW_NORMAL;
		wcscpy_s(fontInfo.FaceName, L"Terminal");
		result = SetCurrentConsoleFontEx(_stdOutHandle, false, &fontInfo);
		assert(result);

		// Change STDIN console mode to allow mouse input.
		result = SetConsoleMode(_stdInHandle, ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
		assert(result);

		_isInitialized = true;
	}
}

void ConsoleInterface::Terminate()
{
	if (_isInitialized)
	{
		Clear();

		// Restore the initial console state that was cached in Initialize().
		{
			bool result = SetConsoleMode(_stdInHandle, _cachedInfo.stdInMode);
			assert(result);

			result = SetConsoleTextAttribute(_stdOutHandle, _cachedInfo.stdOutScreenBufferInfo.wAttributes);
			assert(result);

			result = SetConsoleScreenBufferSize(_stdOutHandle, _cachedInfo.stdOutScreenBufferInfo.dwSize);
			assert(result);

			result = SetCurrentConsoleFontEx(_stdOutHandle, false, &_cachedInfo.stdOutFontInfo);
			assert(result);
		}

		_cachedInfo = {};
		_stdOutHandle = INVALID_HANDLE_VALUE;
		_stdInHandle = INVALID_HANDLE_VALUE;
		_isInitialized = false;
	}
}

void ConsoleInterface::Update()
{
	assert(_isInitialized);

	CONSOLE_SCREEN_BUFFER_INFO sbInfo;
	if (GetConsoleScreenBufferInfo(_stdOutHandle, &sbInfo))
	{
		int16_t newWidth = max(sbInfo.srWindow.Right - sbInfo.srWindow.Left, 0) + 1;
		int16_t newHeight = max(sbInfo.srWindow.Bottom - sbInfo.srWindow.Top, 0) + 1;
		if (newWidth != _currentWidth ||
			newHeight != _currentHeight)
		{
			_currentWidth = newWidth;
			_currentHeight = newHeight;
			SetConsoleScreenBufferSize(_stdOutHandle, { _currentWidth, _currentHeight });
		}
	}

	// TODO: Pass in callback functions for onMouseEvent, onKeyboardEvent, onResizeEvent, and invoke them here
	//bool hasInputEvents = false;
	//{
	//	DWORD inputEventCount;
	//	if (GetNumberOfConsoleInputEvents(_stdInHandle, &inputEventCount))
	//	{
	//		hasInputEvents = (inputEventCount > 0);
	//	}
	//}

	//const uint32_t eventBufferSize = 32;
	//INPUT_RECORD eventBuffer[eventBufferSize];

	//DWORD eventsReadCount;
	//if (ReadConsoleInput(_stdInHandle, eventBuffer, eventBufferSize, &eventsReadCount))
	//{
	//	for (uint32_t eventIndex = 0; eventIndex < eventsReadCount; eventIndex++)
	//	{
	//		const INPUT_RECORD& eventRecord = eventBuffer[eventIndex];
	//		switch (eventRecord.EventType)
	//		{
	//		case KEY_EVENT:
	//			printf("Key Event\n");
	//			break;
	//		case MOUSE_EVENT:
	//			printf("Mouse Event\n");
	//			break;
	//		case WINDOW_BUFFER_SIZE_EVENT:
	//			printf("Window Buffer Size Event\n");
	//			break;
	//		case MENU_EVENT:
	//			printf("Menu Event\n");
	//			break;
	//		case FOCUS_EVENT:
	//			printf("Focus Event\n");
	//			break;
	//		default:
	//			printf("Unknown Event\n");
	//			break;
	//		}
	//	}
	//}
}

void ConsoleInterface::DrawChar(char c, int16_t x, int16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor)
{
	COORD coord = { x, y };
	WORD attributes = sConsoleColorsToAttributes(color, backgroundColor);
	DWORD dummy;
	WriteConsoleOutputAttribute(_stdOutHandle, &attributes, 1, coord, &dummy);
	WriteConsoleOutputCharacterA(_stdOutHandle, &c, 1, coord, &dummy);
}

void ConsoleInterface::DrawString(const char* str, int16_t x, int16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor)
{
	while (*str != NULL &&
		(x >= 0 && x < _currentWidth) &&
		(y >= 0 && y < _currentHeight))
	{
		DrawChar(*str, x, y, color, backgroundColor);
		x++;
		str++;
	}
}

void ConsoleInterface::DrawPixel(int16_t x, int16_t y, const ConsoleColor& color)
{
	DrawChar(' ', x, y, color, color);
}

// Source: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void ConsoleInterface::DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const ConsoleColor& color)
{
	bool isVertical = abs(y1 - y0) > abs(x1 - x0);

	// The base algorithm assumes horizontal lines; swap the x/y values if it's more vertical.
	if (isVertical)
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	// The base algorithm assumes left-to-right lines; swap the start and end points if it's right-to-left.
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int16_t deltaX = abs(x1 - x0);
	int16_t deltaY = abs(y1 - y0);
	int16_t decision = (2 * deltaY) - deltaX;

	int16_t x = x0;
	int16_t y = y0;
	while (x <= x1)
	{
		// If isVertical is true the x/y were swapped; swap them back before drawing.
		if (isVertical) DrawPixel(y, x , color);
		else DrawPixel(x, y , color);

		if (decision > 0)
		{
			y++;
			decision -= 2 * deltaX;
		}
		decision += 2 * deltaY;

		x++;
	}
}

// Source: https://www.thecrazyprogrammer.com/2016/12/bresenhams-midpoint-circle-algorithm-c-c.html
void ConsoleInterface::DrawCircle(int16_t x, int16_t y, int16_t r, const ConsoleColor& color)
{
	assert(r > 0);

	int16_t xOffset = r;
	int16_t yOffset = 0;
	int16_t decision = 0;

	while (xOffset >= yOffset)
	{
		// The algorithm assumes symmetry in all 8 octants, so draw a point in each.
		DrawPixel(x + xOffset, y + yOffset, color);
		DrawPixel(x + xOffset, y - yOffset, color);
		DrawPixel(x - xOffset, y + yOffset, color);
		DrawPixel(x - xOffset, y - yOffset, color);

		DrawPixel(x + yOffset, y + xOffset, color);
		DrawPixel(x + yOffset, y - xOffset, color);
		DrawPixel(x - yOffset, y + xOffset, color);
		DrawPixel(x - yOffset, y - xOffset, color);

		// Decide if we should move up one, or to the left one.
		if (decision <= 0)
		{
			yOffset++;
			decision += (2 * yOffset) + 1;
		}
		else
		{
			xOffset--;
			decision -= (2 * xOffset) + 1;
		}
	}
}

void ConsoleInterface::DrawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const ConsoleColor& color, const ConsoleColor& fillColor)
{
	assert(x0 < x1);
	assert(y0 < y1);

	DrawLine(x0, y0, x1, y0, color);
	for (int16_t y = y0 + 1; y <= y1 - 1; y++)
	{
		DrawPixel(x0, y, color);
		DrawLine(x0 + 1, y, x1 - 1, y, fillColor);
		DrawPixel(x1, y, color);
	}
	DrawLine(x0, y1, x1, y1, color);
}

void ConsoleInterface::Clear()
{
	assert(_isInitialized);

	COORD topLeft = { 0, 0 };
	DWORD bufferSize = _currentWidth * _currentHeight;
	DWORD dummy;
	FillConsoleOutputCharacterA(_stdOutHandle, ' ', bufferSize, topLeft, &dummy);
	FillConsoleOutputAttribute(_stdOutHandle, 0, bufferSize, topLeft, &dummy);
}
