#include "ConsoleInterface.h"

#include <cassert>
#include <cstdio>
#include <utility>

using namespace tictactoe;

static const ConsoleSize kAbsoluteMinimumBufferSize = { 1, 1 };

static WORD sConsoleColorsToAttributes(const ConsoleColor& foregroundColor, const ConsoleColor& backgroundColor);

static COORD sConsoleSizeToCoord(const ConsoleSize& size);
static ConsoleSize sCoordToConsoleSize(const COORD& coord);

static SMALL_RECT sConsoleRectToSmallRect(const ConsoleRect& rect);
static ConsoleRect sSmallRectToConsoleRect(const SMALL_RECT& rect);

ConsoleSize ConsoleRect::GetSize() const
{
	auto width = max(right - left, 0);
	auto height = max(bottom - top, 0);
	return { static_cast<uint16_t>(width), static_cast<uint16_t>(height) };
}

ConsoleInterface::ConsoleInterface() :
	_stdInHandle(GetStdHandle(STD_INPUT_HANDLE)),
	_stdOutHandle(GetStdHandle(STD_OUTPUT_HANDLE)),
	_cachedInfo(),
	_keyEventCallback(nullptr),
	_mouseEventCallback(nullptr),
	_resizeEventCallback(nullptr),
	_minBufferSize(kAbsoluteMinimumBufferSize),
	_currentBufferSize(),
	_currentBufferViewportRect()
{
	assert(_stdInHandle != INVALID_HANDLE_VALUE);
	assert(_stdOutHandle != INVALID_HANDLE_VALUE);

	// Cache the initial console state to be restored in the destructor.
	SaveInitialConsoleState();

	// Change the font to 8x8 Terminal to make our 'pixels' square while keeping the text somewhat legible.
	CONSOLE_FONT_INFOEX fontInfo;
	{
		fontInfo.cbSize = sizeof(fontInfo);
		fontInfo.nFont = 0;
		fontInfo.dwFontSize.X = 8;
		fontInfo.dwFontSize.Y = 8;
		fontInfo.FontFamily = FF_DONTCARE;
		fontInfo.FontWeight = FW_NORMAL;
		wcscpy_s(fontInfo.FaceName, L"Terminal");
	}
	bool result = SetCurrentConsoleFontEx(_stdOutHandle, false, &fontInfo);
	assert(result);

	// Change console mode to allow mouse input.
	result = SetConsoleMode(_stdInHandle, ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
	assert(result);

	// Change console cusor info to hide the blinking cursor.
	CONSOLE_CURSOR_INFO cursorInfo = _cachedInfo.stdOutCursorInfo;
	cursorInfo.bVisible = false;
	result = SetConsoleCursorInfo(_stdOutHandle, &cursorInfo);
	assert(result);

	// Scroll to the top-left of the cursor console buffer.
	SMALL_RECT windowInfo = _cachedInfo.stdOutScreenBufferInfo.srWindow;
	windowInfo.Left = 0;
	windowInfo.Top = 0;
	result = SetConsoleWindowInfo(_stdOutHandle, true, &windowInfo);
	assert(result);

	_currentBufferSize = sCoordToConsoleSize(_cachedInfo.stdOutScreenBufferInfo.dwSize);
	_currentBufferViewportRect = sSmallRectToConsoleRect(_cachedInfo.stdOutScreenBufferInfo.srWindow);
}

ConsoleInterface::~ConsoleInterface()
{
	Clear();

	// Restore the initial console state that was cached in the constructor.
	RestoreInitialConsoleState();
}

void ConsoleInterface::SaveInitialConsoleState()
{
	bool result;

	// Cache the initial console state to be restored in the destructor.
	{
		// IN - Console mode
		result = GetConsoleMode(_stdInHandle, &_cachedInfo.stdInMode);
		assert(result);

		// OUT - Cursor info
		result = GetConsoleCursorInfo(_stdOutHandle, &_cachedInfo.stdOutCursorInfo);
		assert(result);

		// OUT - Screen buffer info
		result = GetConsoleScreenBufferInfo(_stdOutHandle, &_cachedInfo.stdOutScreenBufferInfo);
		assert(result);

		// OUT - Font info
		_cachedInfo.stdOutFontInfo.cbSize = sizeof(_cachedInfo.stdOutFontInfo);
		result = GetCurrentConsoleFontEx(_stdOutHandle, false, &_cachedInfo.stdOutFontInfo);
		assert(result);
	}
}

void ConsoleInterface::RestoreInitialConsoleState()
{
	bool result;

	// OUT - Font info
	result = SetCurrentConsoleFontEx(_stdOutHandle, false, &_cachedInfo.stdOutFontInfo);
	assert(result);

	// OUT - Screen buffer info
	{
		result = SetConsoleTextAttribute(_stdOutHandle, _cachedInfo.stdOutScreenBufferInfo.wAttributes);
		assert(result);

		// HACK: To avoid issues with screen buffer <-> window size dependencies, set the window size to the smallest
		// possible value before setting it back to the original size (see https://stackoverflow.com/a/40634467).
		SMALL_RECT minWindowRect = { 0, 0, 1, 1 };
		result = SetConsoleWindowInfo(_stdOutHandle, true, &minWindowRect);
		assert(result);

		result = SetConsoleScreenBufferSize(_stdOutHandle, _cachedInfo.stdOutScreenBufferInfo.dwSize);
		assert(result);

		result = SetConsoleWindowInfo(_stdOutHandle, true, &_cachedInfo.stdOutScreenBufferInfo.srWindow);
		assert(result);
	}

	// OUT - Cursor info
	result = SetConsoleCursorInfo(_stdOutHandle, &_cachedInfo.stdOutCursorInfo);
	assert(result);

	result = SetConsoleCursorPosition(_stdOutHandle, { 0, 0 });
	assert(result);

	// IN - Console mode
	result = SetConsoleMode(_stdInHandle, _cachedInfo.stdInMode);
	assert(result);
}

void ConsoleInterface::SetCallbacks(KeyEventCallback keyCb, MouseEventCallback mouseCb, ResizeEventCallback resizeCb)
{
	_keyEventCallback = std::move(keyCb);
	_mouseEventCallback = std::move(mouseCb);
	_resizeEventCallback = std::move(resizeCb);
}

void ConsoleInterface::Update()
{
	bool receivedResizeEvent = false;
	DWORD inputEventCount;
	if (GetNumberOfConsoleInputEvents(_stdInHandle, &inputEventCount) &&
		inputEventCount > 0)
	{
		const uint16_t eventBufferSize = 32;
		INPUT_RECORD eventBuffer[eventBufferSize];

		DWORD eventsReadCount;
		if (ReadConsoleInput(_stdInHandle, eventBuffer, eventBufferSize, &eventsReadCount))
		{
			for (uint16_t eventIndex = 0; eventIndex < eventsReadCount; eventIndex++)
			{
				const INPUT_RECORD& eventRecord = eventBuffer[eventIndex];
				switch (eventRecord.EventType)
				{
				case KEY_EVENT:
					_keyEventCallback(eventRecord.Event.KeyEvent);
					break;
				case MOUSE_EVENT:
					_mouseEventCallback(eventRecord.Event.MouseEvent);
					break;
				case WINDOW_BUFFER_SIZE_EVENT:
					receivedResizeEvent = true;
					break;
				default:
					// Do nothing.
					break;
				}
			}
		}
	}

	CONSOLE_SCREEN_BUFFER_INFO sbInfo;
	if (GetConsoleScreenBufferInfo(_stdOutHandle, &sbInfo))
	{
		_currentBufferViewportRect = sSmallRectToConsoleRect(sbInfo.srWindow);

		if (receivedResizeEvent)
		{
			uint16_t w = max(sbInfo.srWindow.Right - sbInfo.srWindow.Left, 0) + 1;
			uint16_t h = max(sbInfo.srWindow.Bottom - sbInfo.srWindow.Top, 0) + 1;
			if (w != _currentBufferSize.width ||
				h != _currentBufferSize.height)
			{
				ResizeBuffer(w, h);
				_resizeEventCallback(_currentBufferSize);
			}
		}
	}
}

void ConsoleInterface::SetMinBufferSize(const ConsoleSize& size)
{
	_minBufferSize.width = max(size.width, kAbsoluteMinimumBufferSize.width);
	_minBufferSize.height = max(size.height, kAbsoluteMinimumBufferSize.height);

	uint16_t w = max(_currentBufferSize.width, _minBufferSize.width);
	uint16_t h = max(_currentBufferSize.height, _minBufferSize.height);
	if (w != _currentBufferSize.width ||
		h != _currentBufferSize.height)
	{
		ResizeBuffer(w, h);
	}
}

void ConsoleInterface::ResizeBuffer(uint16_t w, uint16_t h)
{
	w = max(w, _minBufferSize.width);
	h = max(h, _minBufferSize.height);

	_currentBufferSize = { w, h };
	SetConsoleScreenBufferSize(_stdOutHandle, sConsoleSizeToCoord(_currentBufferSize));
}

void ConsoleInterface::DrawChar(char c, uint16_t x, uint16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor)
{
	COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
	WORD attributes = sConsoleColorsToAttributes(color, backgroundColor);
	DWORD dummy;
	WriteConsoleOutputAttribute(_stdOutHandle, &attributes, 1, coord, &dummy);
	WriteConsoleOutputCharacterA(_stdOutHandle, &c, 1, coord, &dummy);
}

void ConsoleInterface::DrawString(const char* str, uint16_t x, uint16_t y, const ConsoleColor& color, const ConsoleColor& backgroundColor)
{
	while (*str != NULL &&
		(x >= 0 && x < _currentBufferSize.width) &&
		(y >= 0 && y < _currentBufferSize.height))
	{
		DrawChar(*str, x, y, color, backgroundColor);
		x++;
		str++;
	}
}

void ConsoleInterface::DrawPixel(uint16_t x, uint16_t y, const ConsoleColor& color)
{
	DrawChar(' ', x, y, color, color);
}

// Reference: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void ConsoleInterface::DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const ConsoleColor& color)
{
	bool isVertical = abs(y1 - y0) > abs(x1 - x0);

	// The base algorithm assumes horizontal-ish lines; swap the x/y values if it's more vertical-ish.
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

	int16_t yInc = 1;
	if (y0 > y1)
	{
		yInc = -1;
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
			y += yInc;
			decision -= 2 * deltaX;
		}
		decision += 2 * deltaY;

		x++;
	}
}

// Reference: https://www.thecrazyprogrammer.com/2016/12/bresenhams-midpoint-circle-algorithm-c-c.html
void ConsoleInterface::DrawCircle(uint16_t x, uint16_t y, uint16_t r, const ConsoleColor& color)
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

void ConsoleInterface::DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const ConsoleColor& color, const ConsoleColor& fillColor)
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
	COORD topLeft = { 0, 0 };
	DWORD bufferSize = _currentBufferSize.width * _currentBufferSize.height;
	DWORD dummy;
	FillConsoleOutputCharacterA(_stdOutHandle, ' ', bufferSize, topLeft, &dummy);
	FillConsoleOutputAttribute(_stdOutHandle, 0, bufferSize, topLeft, &dummy);
}

static WORD sConsoleColorsToAttributes(const ConsoleColor& foregroundColor, const ConsoleColor& backgroundColor)
{
	WORD foreground = static_cast<WORD>(foregroundColor);
	WORD background = static_cast<WORD>(backgroundColor);
	return foreground + (background * 16);
}

static COORD sConsoleSizeToCoord(const ConsoleSize& size)
{
	return {
		static_cast<SHORT>(size.width),
		static_cast<SHORT>(size.height)
	};
}

static ConsoleSize sCoordToConsoleSize(const COORD& coord)
{
	return {
		static_cast<uint16_t>(coord.X),
		static_cast<uint16_t>(coord.Y)
	};
}

static SMALL_RECT sConsoleRectToSmallRect(const ConsoleRect& rect)
{
	return {
		static_cast<SHORT>(rect.left),
		static_cast<SHORT>(rect.top),
		static_cast<SHORT>(rect.right),
		static_cast<SHORT>(rect.bottom)
	};
}

static ConsoleRect sSmallRectToConsoleRect(const SMALL_RECT& rect)
{
	return {
		static_cast<uint16_t>(rect.Left),
		static_cast<uint16_t>(rect.Top),
		static_cast<uint16_t>(rect.Right),
		static_cast<uint16_t>(rect.Bottom)
	};
}
