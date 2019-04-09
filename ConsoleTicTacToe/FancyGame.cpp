#include "FancyGame.h"

using namespace tictactoe;

#define MARK_SIZE	5
#define PAD_SIZE	1
#define BORDER_SIZE	1
#define CELL_SIZE	(MARK_SIZE + (2 * PAD_SIZE) + BORDER_SIZE)

#define INFO_AREA_SIZE	2

#define VK_Y	0x59
#define VK_Z	0x5A

static const BoardPosition kInvalidBoardPosition = { UINT16_MAX, UINT16_MAX };

static BoardPosition sGetBoardPosition(uint16_t x, uint16_t y);
static ConsoleRect sGetBorderRect(uint16_t r, uint16_t c);
static ConsoleRect sGetMarkerRect(uint16_t r, uint16_t c);
static bool sConsoleRectIntersect(const ConsoleRect& a, const ConsoleRect& b);

ConsoleColor FancyGame::GetPlayerColor(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return ConsoleColor::LightRed;
		case 1:		return ConsoleColor::LightBlue;
		default:	return ConsoleColor::Black;
	}
	static_assert(GameSimulation::kNumPlayers == 2, "GetPlayerColor() needs updating.");
}

FancyGame::FancyGame(uint16_t m, uint16_t n, uint16_t k) :
	GameSimulation(m, n, k),
	_consoleInterface(),
	_isGameAreaDirty(true),
	_isInfoPanelDirty(true),
	_isMouseCellMarkerDirty(true),
	_isQuitRequested(false),
	_currentMouseCell(kInvalidBoardPosition),
	_prevMouseCell(kInvalidBoardPosition),
	_prevViewportRect()
{
	_consoleInterface.SetCallbacks(
		[=](const KEY_EVENT_RECORD& event) { this->OnKeyEvent(event); },
		[=](const MOUSE_EVENT_RECORD& event) { this->OnMouseEvent(event); },
		[=](const ConsoleSize& newSize) { this->OnResizeEvent(newSize); });

	ConsoleSize minBufferSize;
	minBufferSize.width = (CELL_SIZE * m);
	minBufferSize.height = (CELL_SIZE * n) + INFO_AREA_SIZE;
	_consoleInterface.SetMinBufferSize(minBufferSize);

	// Attempt to fit the console window to the game area.
	ConsoleSize windowSize;
	windowSize.width = min(minBufferSize.width, _consoleInterface.GetMaximumBufferViewportSize().width - 1);
	windowSize.height = min(minBufferSize.height, _consoleInterface.GetMaximumBufferViewportSize().height - 1);

	ConsoleSize bufferSize;
	bufferSize.width = max(windowSize.width + 1, minBufferSize.width);
	bufferSize.height = max(windowSize.height + 1, minBufferSize.height);
	_consoleInterface.SetSizes(bufferSize, windowSize);
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

	const GameBoard& gameBoard = GetGameBoard();
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
		_isMouseCellMarkerDirty = true;
	}

	// If the mouse has changed the cell it's over the info panel and cell marker will need to be redrawn.
	if (_prevMouseCell.x != _currentMouseCell.x ||
		_prevMouseCell.y != _currentMouseCell.y)
	{
		_isInfoPanelDirty = true;
		_isMouseCellMarkerDirty = true;
	}

	// Draw the game area.
	if (_isGameAreaDirty)
	{
		_consoleInterface.Clear();
		_isInfoPanelDirty = true;

		// If a player has won, highlight the backgrounds of the winning cells.
		if (GetGameStatus() == GameStatus::Won)
		{
			const auto& positionList = gameBoard.GetWinPositionList();
			for (auto iter = positionList.begin(); iter != positionList.end(); iter++)
			{
				ConsoleRect markerRect = sGetMarkerRect(iter->y, iter->x);
				DrawPlayerMarkerWinBackground(markerRect);
			}
		}

		// Draw the current state of every cell in the game board and its borders.
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
					DrawPlayerMarker(markerRect, gameBoard.GetMarker({ c, r }));
				}
			}
		}

		_isGameAreaDirty = false;
	}

	// Draw the info panel.
	if (_isInfoPanelDirty)
	{
		static_assert(INFO_AREA_SIZE == 2, "Info panel size is assumed to be 2");

		const ConsoleSize viewportSize = viewportRect.GetSize();
		uint16_t minWidth = min(minBufferSize.width, viewportSize.width);
		uint16_t currentY;

		char buffer[32];
		int16_t bufferCharCount;

		// Print the top line.
		currentY = viewportRect.top + 0;
		{
			_consoleInterface.DrawLine(
				viewportRect.left, currentY,
				viewportRect.left + minWidth, currentY,
				ConsoleColor::White);

			// Print the general game information.
			{
				bufferCharCount = sprintf_s(
					buffer,
					"%u-in-a-row",
					gameBoard.GetWinCondition());
				_consoleInterface.DrawString(
					buffer,
					viewportRect.left,
					currentY,
					ConsoleColor::Black,
					ConsoleColor::White);
			}

			// Print the current mouse cell information.
			{
				if (_currentMouseCell.x < numColumns &&
					_currentMouseCell.y < numRows)
				{
					bufferCharCount = sprintf_s(
						buffer,
						"(%u, %u)",
						_currentMouseCell.x,
						_currentMouseCell.y);
				}
				else
				{
					bufferCharCount = sprintf_s(buffer, "(-, -)");
				}

				_consoleInterface.DrawString(
					buffer,
					viewportRect.left + minWidth - bufferCharCount + 1,
					currentY,
					ConsoleColor::Black,
					ConsoleColor::White);
			}
		}

		// Print the current turn information (second line).
		currentY = viewportRect.top + 1;
		{
			ConsoleColor foreground;
			ConsoleColor background;
			if (GetGameStatus() == GameStatus::Won)
			{
				auto winningPlayerID = gameBoard.GetWinningPlayer();
				bufferCharCount = sprintf_s(
					buffer,
					"-- %s (%c) wins! --",
					GetPlayerName(winningPlayerID),
					GetPlayerChar(winningPlayerID));
				foreground = GetPlayerColor(winningPlayerID);
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
					GetPlayerName(GetActivePlayer()),
					GetPlayerChar(GetActivePlayer()));
				foreground = ConsoleColor::Black;
				background = ConsoleColor::White;
			}

			_consoleInterface.DrawLine(
				viewportRect.left, currentY,
				viewportRect.left + minWidth, currentY,
				background);

			_consoleInterface.DrawString(
				buffer,
				viewportRect.left + (minWidth / 2) - (bufferCharCount / 2),
				currentY,
				foreground,
				background);
		}

		_isInfoPanelDirty = false;
	}

	// Draw the mouse cell marker.
	if (_isMouseCellMarkerDirty)
	{
		if (GetGameStatus() == GameStatus::Active)
		{
			// Cleanup any temporary marker in the previous mouse cell.
			if (gameBoard.IsValidPosition(_prevMouseCell) &&
				gameBoard.GetMarker(_prevMouseCell) == kInvalidPlayerID)
			{
				DrawPlayerMarker(sGetMarkerRect(_prevMouseCell.y, _prevMouseCell.x), GetActivePlayer(), ConsoleColor::Black);
			}

			// Draw a temporary marker in the current mouse cell.
			if (gameBoard.IsValidPosition(_currentMouseCell) &&
				gameBoard.GetMarker(_currentMouseCell) == kInvalidPlayerID)
			{
				DrawPlayerMarker(sGetMarkerRect(_currentMouseCell.y, _currentMouseCell.x), GetActivePlayer(), ConsoleColor::DarkGray);
			}
		}

		_isMouseCellMarkerDirty = false;
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
	_isMouseCellMarkerDirty = true;
	_isQuitRequested = false;
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
					if (Redo())
					{
						_isMouseCellMarkerDirty = true;
						_isGameAreaDirty = true;
					}
				}
				break;

			case VK_Z:
				if (isCtrlPressed)
				{
					if (Undo())
					{
						_isMouseCellMarkerDirty = true;
						_isGameAreaDirty = true;
					}
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
		}
	}
}

void FancyGame::OnResizeEvent(const ConsoleSize& newSize)
{
	_isGameAreaDirty = true;
	_isInfoPanelDirty = true;
	_isMouseCellMarkerDirty = true;
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
	DrawPlayerMarker(markerRect, playerID, GetPlayerColor(playerID));
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
		static_cast<uint16_t>(x / CELL_SIZE),
		static_cast<uint16_t>((y - INFO_AREA_SIZE) / CELL_SIZE)
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
