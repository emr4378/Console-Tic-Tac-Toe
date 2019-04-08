#include "FancyGame.h"

using namespace tictactoe;

// TODO: These defines could/should probably be const variables instead (constexpr?)
#define MARK_SIZE	5
#define PAD_SIZE	1
#define BORDER_SIZE	1
#define CELL_SIZE	(MARK_SIZE + (2 * PAD_SIZE) + BORDER_SIZE)

#define INFO_AREA_SIZE	1

#define VK_Y	0x59
#define VK_Z	0x5A

static bool sConsoleRectIntersect(const ConsoleRect& a, const ConsoleRect& b);

static const char* sGetPlayerName(PlayerID playerID);
static char sGetPlayerChar(PlayerID playerID);
static ConsoleColor sGetPlayerColor(PlayerID playerID);

FancyGame::FancyGame(uint32_t m, uint32_t n, uint32_t k) :
	GameSimulation(m, n, k),
	_consoleInterface(),
	_isGameAreaDirty(true),
	_isInfoPanelDirty(true),
	_currentMouseCell(),
	_prevMouseCell(),
	_prevViewportRect()
{
	_consoleInterface.SetCallbacks(
		[=](const KEY_EVENT_RECORD& event) { this->OnKeyEvent(event); },
		[=](const MOUSE_EVENT_RECORD& event) { this->OnMouseEvent(event); },
		[=](const ConsoleSize& newSize) { this->OnResizeEvent(newSize); });

	uint16_t minBufferWidth = (CELL_SIZE * m);
	uint16_t minBufferHeight = (CELL_SIZE * n) + INFO_AREA_SIZE;
	_consoleInterface.SetMinBufferSize({ minBufferWidth, minBufferHeight });
}

FancyGame::~FancyGame()
{
}

void FancyGame::Update()
{
	_consoleInterface.Update();

	// If the viewport has been scrolled the game area will need to be redrawn.
	const ConsoleRect& viewportRect = _consoleInterface.GetCurrentBufferViewportRect();
	if (viewportRect.left != _prevViewportRect.left ||
		viewportRect.right != _prevViewportRect.right ||
		viewportRect.top != _prevViewportRect.top ||
		viewportRect.bottom != _prevViewportRect.bottom)
	{
		_isGameAreaDirty = true;
	}

	// If the mouse has changed the cell it's over the info panel will need to be redrawn.
	if (_prevMouseCell.x != _currentMouseCell.x ||
		_prevMouseCell.y != _currentMouseCell.y)
	{
		_isInfoPanelDirty = true;
	}

	// Draw the game board
	if (_isGameAreaDirty)
	{
		_consoleInterface.Clear();
		_isInfoPanelDirty = true;

		const ConsoleSize minBufferSize = _consoleInterface.GetMinBufferSize();
		const uint16_t numColumns = minBufferSize.width / CELL_SIZE;
		const uint16_t numRows = minBufferSize.height / CELL_SIZE;
		for (uint16_t r = 0; r < numRows; r++)
		{
			for (uint16_t c = 0; c < numColumns; c++)
			{
				ConsoleRect borderRect;
				borderRect.left = (c + 0) * CELL_SIZE;
				borderRect.top = (r + 0) * CELL_SIZE + INFO_AREA_SIZE;
				borderRect.right = (c + 1) * CELL_SIZE;
				borderRect.bottom = (r + 1) * CELL_SIZE + INFO_AREA_SIZE;

				if (sConsoleRectIntersect(borderRect, viewportRect))
				{
					ConsoleRect markRect = borderRect;
					markRect.left += BORDER_SIZE + PAD_SIZE;
					markRect.top += BORDER_SIZE + PAD_SIZE;
					markRect.right -= BORDER_SIZE + PAD_SIZE;
					markRect.bottom -= BORDER_SIZE + PAD_SIZE;

					// Draw player marker
					PlayerID playerID = _gameBoard.GetMarker({ c, r });
					switch (playerID)
					{
					case kInvalidPlayerID:
						// Do nothing - empty square.
						break;

					case 0:
						_consoleInterface.DrawLine(
							markRect.left, markRect.top,
							markRect.right, markRect.bottom,
							sGetPlayerColor(playerID));

						_consoleInterface.DrawLine(
							markRect.right, markRect.top,
							markRect.left, markRect.bottom,
							sGetPlayerColor(playerID));
						break;

					case 1:
						_consoleInterface.DrawCircle(
							(markRect.right + markRect.left) / 2,
							(markRect.top + markRect.bottom) / 2,
							MARK_SIZE / 2,
							sGetPlayerColor(playerID));
						break;

					default:
						assert(false);
						break;
					}
					// TODO: static_assert(NUM_PLAYERS == 2);

					// Draw right-side border
					if (c < numColumns - 1)
					{
						_consoleInterface.DrawLine(
							borderRect.right,
							borderRect.top + BORDER_SIZE,
							borderRect.right,
							borderRect.bottom - BORDER_SIZE,
							ConsoleColor::LightGray);
						static_assert(BORDER_SIZE == 1, "Border size assumed to be 1 here.");
					}

					// Draw bottom-side border
					if (r < numRows - 1)
					{
						_consoleInterface.DrawLine(
							borderRect.left + BORDER_SIZE,
							borderRect.bottom,
							borderRect.right - BORDER_SIZE,
							borderRect.bottom,
							ConsoleColor::LightGray);
						static_assert(BORDER_SIZE == 1, "Border size assumed to be 1 here.");
					}
				}
			}
		}

		_isGameAreaDirty = false;
	}

	// Draw the info panel
	if (_isInfoPanelDirty)
	{
		const ConsoleSize viewportSize = viewportRect.GetSize();

		_consoleInterface.DrawRectangle(
			viewportRect.left, viewportRect.top,
			viewportRect.right + 1, viewportRect.top + INFO_AREA_SIZE,
			ConsoleColor::Black,
			ConsoleColor::Black);

		char buffer[32];
		auto strLength = sprintf_s(
			buffer,
			"%s's turn (%c)",
			sGetPlayerName(_activePlayer),
			sGetPlayerChar(_activePlayer));
		_consoleInterface.DrawString(
			buffer,
			viewportRect.left + (viewportSize.width / 2) - (strLength / 2),
			viewportRect.top,
			ConsoleColor::Black,
			ConsoleColor::White);

		strLength = sprintf_s(buffer, "X: %u, Y: %u\0", _currentMouseCell.x, _currentMouseCell.y);
		_consoleInterface.DrawString(
			buffer,
			viewportRect.right - strLength,
			viewportRect.top,
			ConsoleColor::Black,
			ConsoleColor::White);

		_isInfoPanelDirty = false;
	}

	_prevMouseCell = _currentMouseCell;
	_prevViewportRect = viewportRect;
}

void FancyGame::OnKeyEvent(const KEY_EVENT_RECORD& event)
{
	if (event.bKeyDown)
	{
		bool isCtrlPressed =
			event.dwControlKeyState & LEFT_CTRL_PRESSED ||
			event.dwControlKeyState & RIGHT_CTRL_PRESSED;

		switch (event.wVirtualKeyCode)
		{
			case VK_SPACE:
				if (isCtrlPressed)
				{
					// HACK: Force a redraw
					_isGameAreaDirty = true;
				}
				break;

			case VK_ESCAPE:
				// TODO: Quit the game
				break;

			case VK_Y:
				if (isCtrlPressed)
				{
					// TODO: Redo should return success, only mark game area as dirty if successful
					Redo();
					_isGameAreaDirty = true;
				}
				break;

			case VK_Z:
				if (isCtrlPressed)
				{
					// TODO: Undo should return success, only mark game area as dirty if successful
					Undo();
					_isGameAreaDirty = true;
				}
				break;

			default:
				// Do nothing
				break;
		}
	}
}

void FancyGame::OnMouseEvent(const MOUSE_EVENT_RECORD& event)
{
	auto mouseColumn = static_cast<uint16_t>(event.dwMousePosition.X / CELL_SIZE);
	auto mouseRow = static_cast<uint16_t>(event.dwMousePosition.Y / CELL_SIZE);
	_currentMouseCell = { mouseColumn, mouseRow };

	if (event.dwEventFlags == 0 &&
		event.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		MarkResult result = Mark(_currentMouseCell);
		if (result == MarkResult::Success)
		{
			_isGameAreaDirty = true;
			_isInfoPanelDirty = true;
		}
		else
		{
			// TODO: Some sort of error handling
		}
	}
}

void FancyGame::OnResizeEvent(const ConsoleSize& newSize)
{
	_isGameAreaDirty = true;
	_isInfoPanelDirty = true;
}

static bool sConsoleRectIntersect(const ConsoleRect& a, const ConsoleRect& b)
{
	return !(b.left > a.right ||
		b.right < a.left ||
		b.top > a.bottom ||
		b.bottom < a.top);
}

static const char* sGetPlayerName(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return "Player 1";
		case 1:		return "Player 2";
		default:	return nullptr;
	}
	// TODO: static_assert(NUM_PLAYERS == 2);
}

static char sGetPlayerChar(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return 'X';
		case 1:		return 'O';
		default:	return ' ';
	}
	// TODO: static_assert(NUM_PLAYERS == 2);
}

static ConsoleColor sGetPlayerColor(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return ConsoleColor::LightRed;
		case 1:		return ConsoleColor::LightBlue;
		default:	return ConsoleColor::Black;
	}
	// TODO: static_assert(NUM_PLAYERS == 2);
}
