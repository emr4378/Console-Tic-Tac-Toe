#include "ConsoleInterface.h"
#include "GameBoard.h"
#include "UndoManager.h"

#include <cassert>
#include <iostream>
#include <string>
#include <Windows.h>

#define NUM_PLAYERS 2

using namespace tictactoe;

//class Game
//{
//public:
//	Game(uint32_t m, uint32_t n, uint32_t k);
//
//	void Update();
//
//private:
//	void ExecuteHelp() const;
//	void ExecuteStatus() const;
//	void ExecuteMark(const std::string& params);
//	void ExecuteUndo();
//	void ExecuteRedo();
//	void ExecuteQuit();
//
//	GameBoard _gameBoard;
//	UndoManager<PlayerMove> _moveHistory;
//};

static GameBoard sgGameBoard;
static UndoManager<PlayerMove> sgMoveHistory;
static ConsoleInterface sgConsoleInterface;
static COORD sgMousePosition;
static bool sgIsDirty = true;
static bool sgIsShutdownRequested = false;

static void sInitializeGameSystems(uint32_t m, uint32_t n, uint32_t k);
static void sTerminateGameSystems();
static BOOL sConsoleCtrlHandler(DWORD dwCtrlType);

static bool sTryParseUInt(const std::string& str, uint32_t* outValue);

std::ostream& operator<<(std::ostream& os, const BoardPosition& position);
std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard);

static bool sConsoleRectIntersect(const ConsoleRect& a, const ConsoleRect& b)
{
	return !(b.left > a.right ||
		b.right < a.left ||
		b.top > a.bottom ||
		b.bottom < a.top);
}



#define MARK_SIZE	5
#define PAD_SIZE	1
#define BORDER_SIZE	1
#define CELL_SIZE	(MARK_SIZE + (2 * PAD_SIZE) + BORDER_SIZE)

#define INFO_AREA_SIZE	1

static ConsoleRect sgLastViewportRect;
int main(int argc, char** argv)
{
	uint32_t m;
	uint32_t n;
	uint32_t k;
	if (argc != 4 ||
		!sTryParseUInt(argv[1], &m) ||
		!sTryParseUInt(argv[2], &n) ||
		!sTryParseUInt(argv[3], &k))
	{
		// TODO: printUsage()
		return EXIT_FAILURE;
	}

	sInitializeGameSystems(m, n, k);

	uint16_t minBufferWidth = (CELL_SIZE * m);
	uint16_t minBufferHeight = (CELL_SIZE * n) + INFO_AREA_SIZE;
	sgConsoleInterface.SetMinBufferSize({ minBufferWidth, minBufferHeight });

	uint16_t maxColumns = minBufferWidth / CELL_SIZE;
	uint16_t maxRows = minBufferHeight / CELL_SIZE;

	while (!sgIsShutdownRequested)
	{
		sgConsoleInterface.Update();

		const ConsoleRect& viewportRect = sgConsoleInterface.GetCurrentBufferViewportRect();
		const ConsoleSize viewportSize = viewportRect.GetSize();

		if (viewportRect.left != sgLastViewportRect.left ||
			viewportRect.right != sgLastViewportRect.right ||
			viewportRect.top != sgLastViewportRect.top ||
			viewportRect.bottom != sgLastViewportRect.bottom)
		{
			sgIsDirty = true;
		}
		sgLastViewportRect = viewportRect;

		if (sgIsDirty)
		{
			sgConsoleInterface.Clear();

			// Draw the game board
			for (auto r = 0; r < maxRows; r++)
			{
				for (auto c = 0; c < maxColumns; c++)
				{
					ConsoleRect borderRect;
					borderRect.left =	(c + 0) * CELL_SIZE;
					borderRect.top =	(r + 0) * CELL_SIZE + INFO_AREA_SIZE;
					borderRect.right =	(c + 1) * CELL_SIZE;
					borderRect.bottom =	(r + 1) * CELL_SIZE + INFO_AREA_SIZE;

					if (sConsoleRectIntersect(borderRect, viewportRect))
					{
						ConsoleRect markRect = borderRect;
						markRect.left += BORDER_SIZE + PAD_SIZE;
						markRect.top += BORDER_SIZE + PAD_SIZE;
						markRect.right -= BORDER_SIZE + PAD_SIZE;
						markRect.bottom -= BORDER_SIZE + PAD_SIZE;

						// Draw player marker
						//if (playerID == 0)
						{
							// Draw '\'
							sgConsoleInterface.DrawLine(
								markRect.left, markRect.top,
								markRect.right, markRect.bottom,
								ConsoleColor::LightRed);

							// Draw '/'
							sgConsoleInterface.DrawLine(
								markRect.right, markRect.top,
								markRect.left, markRect.bottom,
								ConsoleColor::LightRed);
						}

						//if (playerID == 1)
						{
							// Draw 'O'
							sgConsoleInterface.DrawCircle(
								(markRect.right + markRect.left) / 2,
								(markRect.top + markRect.bottom) / 2,
								MARK_SIZE / 2,
								ConsoleColor::LightBlue);
						}

						// Draw right-side border
						if (c < maxColumns - 1)
						{
							sgConsoleInterface.DrawLine(
								borderRect.right,
								borderRect.top + BORDER_SIZE,
								borderRect.right,
								borderRect.bottom - BORDER_SIZE,
								ConsoleColor::LightGray);
							static_assert(BORDER_SIZE == 1, "Border size assumed to be 1 here.");
						}

						// Draw bottom-side border
						if (r < maxRows - 1)
						{
							sgConsoleInterface.DrawLine(
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

			sgIsDirty = false;
		}

		// Draw the info area
		{
			sgConsoleInterface.DrawRectangle(
				viewportRect.left, viewportRect.top,
				viewportRect.right + 1, viewportRect.top + INFO_AREA_SIZE,
				ConsoleColor::Black,
				ConsoleColor::Black);

			auto mouseColumn = static_cast<uint16_t>(sgMousePosition.X / CELL_SIZE);
			auto mouseRow = static_cast<uint16_t>(sgMousePosition.Y / CELL_SIZE);

			char buffer[32];
			auto strLength = sprintf_s(buffer, "Player %u's turn (%c)", 1, 'X');
			sgConsoleInterface.DrawString(
				buffer,
				viewportRect.left + (viewportSize.width / 2) - (strLength / 2),
				viewportRect.top,
				ConsoleColor::Black,
				ConsoleColor::White);

			strLength = sprintf_s(buffer, "X: %u, Y: %u\0", mouseColumn, mouseRow);
			sgConsoleInterface.DrawString(
				buffer,
				viewportRect.right - strLength,
				viewportRect.top,
				ConsoleColor::Black,
				ConsoleColor::White);
		}

		Sleep(10);
	}

	std::cout << "Exiting" << std::endl;
	sTerminateGameSystems();



	//std::cout << "M: " << m << " N: " << n << " K: " << k << std::endl;

	//GameBoard gameBoard(m, n, k);
	//UndoManager<PlayerMove> moveHistory(
	//	[&gameBoard](const PlayerMove& move) { gameBoard.Unmark(move.playerID, move.position); },
	//	[&gameBoard](const PlayerMove& move) { gameBoard.Mark(move.playerID, move.position); });

	//PlayerID playerID = 0;
	//while (true)
	//{
	//	std::string userInput;
	//	bool advanceTurn = false;

	//	//std::cout << gameBoard << std::endl;
	//	std::cout << "[Player " << playerID << "] Enter a command: ";
	//	getline(std::cin, userInput);

	//	auto commandNameEnd = userInput.find(' ');
	//	if (userInput.compare(0, commandNameEnd, "Place") == 0)
	//	{
	//		std::string xParam;
	//		std::string yParam;
	//		{
	//			auto paramStart = commandNameEnd + 1;
	//			auto paramEnd = userInput.find(',', paramStart);
	//			xParam = userInput.substr(paramStart, paramEnd - paramStart);

	//			paramStart = paramEnd + 1;
	//			paramEnd = std::string::npos;
	//			yParam = userInput.substr(paramStart, paramEnd);
	//		}

	//		BoardPosition position;
	//		if (sTryParseUInt(xParam, &position.x) &&
	//			sTryParseUInt(yParam, &position.y))
	//		{
	//			MarkResult result = gameBoard.Mark(playerID, position);
	//			switch (result)
	//			{
	//			case MarkResult::Success:
	//				moveHistory.Add({ playerID, position });
	//				advanceTurn = true;
	//				std::cout << "Marker placed at " << position << std::endl;
	//				break;

	//			case MarkResult::PositionOutOfBounds:
	//				std::cerr << position << " is out of bounds." << std::endl;
	//				break;

	//			case MarkResult::PositionAlreadyMarked:
	//				std::cerr << position << " is already owned by " << playerID << std::endl;
	//				break;
	//			}
	//			static_assert(static_cast<int32_t>(MarkResult::Count) == 3, "MarkResult changed without updating switch!");
	//		}
	//		else
	//		{
	//			// TODO: Invalid input
	//			std::cerr << "Oh no, invalid input for place." << std::endl;
	//		}
	//	}
	//	else if (userInput.compare(0, commandNameEnd, "Undo") == 0)
	//	{
	//		if (moveHistory.GetAvailableUndosCount() >= NUM_PLAYERS)
	//		{
	//			for (uint16_t _ = 0; _ < NUM_PLAYERS; _++)
	//			{
	//				moveHistory.Undo();
	//			}
	//		}
	//		else
	//		{
	//			std::cerr << "Unable to undo any further." << std::endl;
	//		}
	//	}
	//	else if (userInput.compare(0, commandNameEnd, "Redo") == 0)
	//	{
	//		if (moveHistory.GetAvailableRedosCount() >= NUM_PLAYERS)
	//		{
	//			for (uint16_t _ = 0; _ < NUM_PLAYERS; _++)
	//			{
	//				moveHistory.Redo();
	//			}
	//		}
	//		else
	//		{
	//			std::cerr << "Unable to redo any further." << std::endl;
	//		}
	//	}

	// -- Command Ideas --
	// Help
	// Status
	// Mark <x> <y>
	// Undo
	// Redo
	// Quit

	//	if (advanceTurn)
	//	{
	//		playerID = (playerID + 1) % NUM_PLAYERS;
	//	}
	//}

	return EXIT_SUCCESS;
}

static void sEdtestKeyEventCallback(const KEY_EVENT_RECORD& event)
{
	if (event.bKeyDown)
	{
		if (event.wVirtualKeyCode == VK_SPACE)
		{
			sgIsDirty = true;
		}
		else if (event.wVirtualKeyCode == VK_ESCAPE)
		{
			sgIsShutdownRequested = true;
		}
	}
}

static void sEdtestMouseEventCallback(const MOUSE_EVENT_RECORD& event)
{
	//sgConsoleInterface.DrawLine(0, 0, event.dwMousePosition.X, event.dwMousePosition.Y, ConsoleColor::LightCyan);
	
	if (event.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		//sgConsoleInterface.DrawPixel(event.dwMousePosition.X, event.dwMousePosition.Y, ConsoleColor::LightCyan);
		sgConsoleInterface.DrawLine(sgMousePosition.X, sgMousePosition.Y, event.dwMousePosition.X, event.dwMousePosition.Y, ConsoleColor::LightCyan);
	}
	else if (event.dwButtonState & RIGHTMOST_BUTTON_PRESSED)
	{
		sgConsoleInterface.DrawLine(sgMousePosition.X, sgMousePosition.Y, event.dwMousePosition.X, event.dwMousePosition.Y, ConsoleColor::Black);
	}
	sgMousePosition = event.dwMousePosition;
}

static void sEdtestResizeEventCallback(const ConsoleSize& size)
{
	sgIsDirty = true;
}

static void sInitializeGameSystems(uint32_t m, uint32_t n, uint32_t k)
{
	sgGameBoard.Initialize(m, n, k);
	sgMoveHistory.Initialize(
		[](const PlayerMove& move) { sgGameBoard.Unmark(move.playerID, move.position); },
		[](const PlayerMove& move) { sgGameBoard.Mark(move.playerID, move.position); });
	sgConsoleInterface.Initialize(
		sEdtestKeyEventCallback,
		sEdtestMouseEventCallback,
		sEdtestResizeEventCallback);

	bool result = SetConsoleCtrlHandler(sConsoleCtrlHandler, true);
	assert(result);
}

static void sTerminateGameSystems()
{
	bool result = SetConsoleCtrlHandler(sConsoleCtrlHandler, false);
	assert(result);

	sgConsoleInterface.Terminate();
	sgMoveHistory.Terminate();
	sgGameBoard.Terminate();
}

static BOOL sConsoleCtrlHandler(DWORD dwCtrlType)
{
	sTerminateGameSystems();
	return false;
}

static bool sTryParseUInt(const std::string& str, uint32_t* outValue)
{
	bool result = false;
	*outValue = 0;

	try
	{
		size_t pos;
		auto value = std::stoul(str, &pos);
		if (value < LONG_MAX && pos == str.length())
		{
			*outValue = value;
			result = true;
		}
	}
	catch (...)
	{
		// Do nothing.
	}

	return result;
}

std::ostream& operator<<(std::ostream& os, const BoardPosition& position)
{
	os << "Position {" << position.x << ", " << position.y << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard)
{
	os << "GameBoard (" << gameBoard.GetColumns() << "x" << gameBoard.GetRows() << ", " << gameBoard.GetWinCondition() << "-in-a-row)" << std::endl;

	for (uint32_t row = 0; row < gameBoard.GetRows(); row++)
	{
		for (uint32_t column = 0; column < gameBoard.GetColumns(); column++)
		{
			switch (gameBoard.GetMarker({ column, row }))
			{
			case 0:
				os << 'X';
				break;

			case 1:
				os << 'O';
				break;

			default:
				os << ' ';
				break;
			}

			if (column < gameBoard.GetColumns() - 1)
			{
				os << '|';
			}
		}
		os << std::endl;

		if (row < gameBoard.GetRows() - 1)
		{
			os << std::string(gameBoard.GetColumns() * 2 - 1, '-') << std::endl;
		}
	}
	os << std::endl;

	return os;
}
