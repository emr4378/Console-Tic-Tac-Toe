#include "FancyGame.h"

using namespace tictactoe;

#define MARK_SIZE	5
#define PAD_SIZE	1
#define BORDER_SIZE	1
#define CELL_SIZE	(MARK_SIZE + (2 * PAD_SIZE) + BORDER_SIZE)

#define INFO_AREA_SIZE	2

#define VK_Y	0x59
#define VK_Z	0x5A

static BoardPosition sGetBoardPosition(uint16_t x, uint16_t y);
static ConsoleRect sGetBorderRect(uint16_t r, uint16_t c);
static ConsoleRect sGetMarkerRect(uint16_t r, uint16_t c);
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

bool FancyGame::Update()
{
	if (_isQuitRequested)
	{
		return false;
	}

	_consoleInterface.Update();

	const ConsoleSize minBufferSize = _consoleInterface.GetMinBufferSize();
	const uint16_t numColumns = minBufferSize.width / CELL_SIZE;
	const uint16_t numRows = minBufferSize.height / CELL_SIZE;

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

		if (GetGameStatus() == GameStatus::Active)
		{
			// Draw a temporary marker in the current mouse cell.
			if (_gameBoard.IsValidPosition(_currentMouseCell) &&
				_gameBoard.GetMarker(_currentMouseCell) == kInvalidPlayerID)
			{
				DrawPlayerMarker(sGetMarkerRect(_currentMouseCell.y, _currentMouseCell.x), GetActivePlayer(), ConsoleColor::DarkGray);
			}

			// Cleanup any temporary marker in the previous mouse cell.
			if (_gameBoard.IsValidPosition(_prevMouseCell) &&
				_gameBoard.GetMarker(_prevMouseCell) == kInvalidPlayerID)
			{
				DrawPlayerMarker(sGetMarkerRect(_prevMouseCell.y, _prevMouseCell.x), GetActivePlayer(), ConsoleColor::Black);
			}
		}
	}

	// Draw the game area.
	if (_isGameAreaDirty)
	{
		_consoleInterface.Clear();
		_isInfoPanelDirty = true;

		// If a player has won, highlight the backgrounds of the winning cells.
		if (GetGameStatus() == GameStatus::Won)
		{
			const auto& positionList = _gameBoard.GetWinPositionList();
			for (auto iter = positionList.begin(); iter != positionList.end(); iter++)
			{
				ConsoleRect markerRect = sGetMarkerRect(iter->y, iter->x);
				DrawPlayerMarkerWinBackground(markerRect);
			}
		}

		// Draw the current state of every cell in the gameboard and its borders.
		for (uint16_t r = 0; r < numRows; r++)
		{
			for (uint16_t c = 0; c < numColumns; c++)
			{
				ConsoleRect borderRect = sGetBorderRect(r, c);
				if (sConsoleRectIntersect(borderRect, viewportRect))
				{
					if (c < numColumns - 1)
					{
						DrawCellBorderRightSide(borderRect);
					}

					if (r < numRows - 1)
					{
						DrawCellBorderBottomSide(borderRect);
					}

					ConsoleRect markerRect = sGetMarkerRect(r, c);
					DrawPlayerMarker(markerRect, _gameBoard.GetMarker({ c, r }));
				}
			}
		}

		_isGameAreaDirty = false;
	}

	// Draw the info panel.
	if (_isInfoPanelDirty)
	{
		const ConsoleSize viewportSize = viewportRect.GetSize();

		_consoleInterface.DrawRectangle(
			viewportRect.left, viewportRect.top,
			viewportRect.right, viewportRect.top + INFO_AREA_SIZE,
			ConsoleColor::Black,
			ConsoleColor::Black);

		char buffer[32];
		int32_t bufferCharCount;

		// Print the current turn information.
		{
			ConsoleColor foreground;
			ConsoleColor background;
			if (GetGameStatus() == GameStatus::Won)
			{
				auto winningPlayerID = _gameBoard.GetWinningPlayerID();
				bufferCharCount = sprintf_s(
					buffer,
					"-- %s (%c) wins! --",
					sGetPlayerName(winningPlayerID),
					sGetPlayerChar(winningPlayerID));
				foreground = sGetPlayerColor(winningPlayerID);
				background = ConsoleColor::DarkGray;
			}
			else if (GetGameStatus() == GameStatus::Draw)
			{
				bufferCharCount = sprintf_s(buffer, "- No spaces left: draw! -");
				foreground = ConsoleColor::LightGray;
				background = ConsoleColor::DarkGray;
			}
			else
			{
				bufferCharCount = sprintf_s(
					buffer,
					"%s's turn (%c)",
					sGetPlayerName(GetActivePlayer()),
					sGetPlayerChar(GetActivePlayer()));
				foreground = ConsoleColor::Black;
				background = ConsoleColor::White;
			}

			_consoleInterface.DrawString(
				buffer,
				//viewportRect.left + (viewportSize.width / 2) - (bufferCharCount / 2),
				viewportRect.left,
				viewportRect.top,
				foreground,
				background);
		}

		// Print the current mouse cell information.
		{
			if (_currentMouseCell.x < numColumns &&
				_currentMouseCell.y < numRows)
			{
				bufferCharCount = sprintf_s(
					buffer,
					"X: %u, Y: %u\0",
					_currentMouseCell.x,
					_currentMouseCell.y);
			}
			else
			{
				bufferCharCount = sprintf_s(buffer, "X: --, Y: --");
			}
			_consoleInterface.DrawString(
				buffer,
				viewportRect.left,
				viewportRect.top + 1,
				ConsoleColor::Black,
				ConsoleColor::White);
		}

		_isInfoPanelDirty = false;
	}

	_prevMouseCell = _currentMouseCell;
	_prevViewportRect = viewportRect;

	return true;
}

void FancyGame::Reset()
{
	GameSimulation::Reset();

	_consoleInterface.Clear();
	_isGameAreaDirty = true;
	_isInfoPanelDirty = true;
	_prevMouseCell = {};
	_currentMouseCell = {};
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
				Reset();
				break;

			case VK_ESCAPE:
				_isQuitRequested = true;
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
	_currentMouseCell = sGetBoardPosition(event.dwMousePosition.X, event.dwMousePosition.Y);

	if (event.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		if (GetGameStatus() != GameStatus::Active &&
			event.dwEventFlags & DOUBLE_CLICK)
		{
			Reset();
		}

		if (event.dwEventFlags == 0)
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
}

void FancyGame::OnResizeEvent(const ConsoleSize& newSize)
{
	_isGameAreaDirty = true;
	_isInfoPanelDirty = true;
}

void FancyGame::DrawCellBorderRightSide(const ConsoleRect& borderRect)
{
	_consoleInterface.DrawLine(
		borderRect.right,
		borderRect.top + BORDER_SIZE,
		borderRect.right,
		borderRect.bottom - BORDER_SIZE,
		ConsoleColor::LightGray);
	static_assert(BORDER_SIZE == 1, "Border size assumed to be 1 here.");
}

void FancyGame::DrawCellBorderBottomSide(const ConsoleRect& borderRect)
{
	_consoleInterface.DrawLine(
		borderRect.left + BORDER_SIZE,
		borderRect.bottom,
		borderRect.right - BORDER_SIZE,
		borderRect.bottom,
		ConsoleColor::LightGray);
	static_assert(BORDER_SIZE == 1, "Border size assumed to be 1 here.");
}

void FancyGame::DrawPlayerMarker(const ConsoleRect& markerRect, PlayerID playerID)
{
	DrawPlayerMarker(markerRect, playerID, sGetPlayerColor(playerID));
}

void FancyGame::DrawPlayerMarker(const ConsoleRect& markerRect, PlayerID playerID, ConsoleColor color)
{
	switch (playerID)
	{
		case kInvalidPlayerID:
			// Do nothing - empty square.
			break;

		case 0:
			_consoleInterface.DrawLine(
				markerRect.left, markerRect.top,
				markerRect.right, markerRect.bottom,
				color);

			_consoleInterface.DrawLine(
				markerRect.right, markerRect.top,
				markerRect.left, markerRect.bottom,
				color);
			break;

		case 1:
			_consoleInterface.DrawCircle(
				(markerRect.right + markerRect.left) / 2,
				(markerRect.top + markerRect.bottom) / 2,
				MARK_SIZE / 2,
				color);
			break;

		default:
			assert(false);
			break;
	}
	static_assert(GameSimulation::kNumPlayers == 2, "FancyGame::DrawPlayerMarker() needs updating.");
}

void FancyGame::DrawPlayerMarkerWinBackground(const ConsoleRect& markerRect)
{
	_consoleInterface.DrawRectangle(
		markerRect.left, markerRect.top,
		markerRect.right, markerRect.bottom,
		ConsoleColor::DarkGreen,
		ConsoleColor::LightGreen);
}

static BoardPosition sGetBoardPosition(uint16_t x, uint16_t y)
{
	return {
		static_cast<uint32_t>(x / CELL_SIZE),
		static_cast<uint32_t>((y - INFO_AREA_SIZE) / CELL_SIZE)
	};
}

static ConsoleRect sGetBorderRect(uint16_t r, uint16_t c)
{
	ConsoleRect borderRect;
	borderRect.left = (c + 0) * CELL_SIZE;
	borderRect.top = (r + 0) * CELL_SIZE + INFO_AREA_SIZE;
	borderRect.right = (c + 1) * CELL_SIZE;
	borderRect.bottom = (r + 1) * CELL_SIZE + INFO_AREA_SIZE;
	return borderRect;
}

static ConsoleRect sGetMarkerRect(uint16_t r, uint16_t c)
{
	ConsoleRect markerRect = sGetBorderRect(r, c);
	markerRect.left += BORDER_SIZE + PAD_SIZE;
	markerRect.top += BORDER_SIZE + PAD_SIZE;
	markerRect.right -= BORDER_SIZE + PAD_SIZE;
	markerRect.bottom -= BORDER_SIZE + PAD_SIZE;
	return markerRect;
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
	static_assert(GameSimulation::kNumPlayers == 2, "sGetPlayerName() needs updating.");
}

static char sGetPlayerChar(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return 'X';
		case 1:		return 'O';
		default:	return ' ';
	}
	static_assert(GameSimulation::kNumPlayers == 2, "sGetPlayerChar() needs updating.");
}

static ConsoleColor sGetPlayerColor(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return ConsoleColor::LightRed;
		case 1:		return ConsoleColor::LightBlue;
		default:	return ConsoleColor::Black;
	}
	static_assert(GameSimulation::kNumPlayers == 2, "sGetPlayerColor() needs updating.");
}
