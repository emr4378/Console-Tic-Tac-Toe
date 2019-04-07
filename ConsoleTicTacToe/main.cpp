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

static void sInitializeGameSystems(uint32_t m, uint32_t n, uint32_t k);
static void sTerminateGameSystems();
static BOOL sConsoleCtrlHandler(DWORD dwCtrlType);

static bool sTryParseUInt(const std::string& str, uint32_t* outValue);

std::ostream& operator<<(std::ostream& os, const BoardPosition& position);
std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard);

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

	//{
	//	// TODO: Print coordinates of currently mouse-hovered gridcell at bottom-right corner
	//	// https://docs.microsoft.com/en-us/windows/console/low-level-console-output-functions
	//	// https://docs.microsoft.com/en-us/windows/console/console-screen-buffers
	//	// https://stackoverflow.com/a/39241792

	//	HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	//	assert(stdOutHandle != INVALID_HANDLE_VALUE);

	//	//HWND x = GetConsoleWindow();
	//	//ShowScrollBar(x, SB_BOTH, false);

	//	CONSOLE_SCREEN_BUFFER_INFO currentConsoleInfo;
	//	if (GetConsoleScreenBufferInfo(stdOutHandle, &currentConsoleInfo))
	//	{
	//		DWORD dummy;
	//		COORD newSize = {
	//			(currentConsoleInfo.srWindow.Right - currentConsoleInfo.srWindow.Left) + 1,
	//			(currentConsoleInfo.srWindow.Bottom - currentConsoleInfo.srWindow.Top) + 1
	//		};

	//		bool resizeSuccess = SetConsoleScreenBufferSize(stdOutHandle, newSize);
	//		assert(resizeSuccess);

	//		COORD edtestPosition;
	//		edtestPosition.X = newSize.X - 1;
	//		edtestPosition.Y = newSize.Y - 1;
	//		bool result = WriteConsoleOutputCharacterA(
	//			stdOutHandle,
	//			"WAFFLES",
	//			7,
	//			edtestPosition,
	//			&dummy);
	//		assert(result);

	//		edtestPosition.X = 0;
	//		edtestPosition.Y = newSize.Y - 1;
	//		result = SetConsoleCursorPosition(stdOutHandle, edtestPosition);
	//		assert(result);

	//		//
	//		//bool fillSuccess = FillConsoleOutputCharacterA(stdOutHandle, 'A', newSize.X * newSize.Y, { 0, 0 }, &dummy);
	//		//assert(fillSuccess);
	//	}
	//}

	while (true)
	{
		sgConsoleInterface.Update();

		sgConsoleInterface.Clear();
		sgConsoleInterface.DrawLine(0, 0, sgConsoleInterface.GetWidth(), sgConsoleInterface.GetHeight(), ConsoleColor::LightMagenta);

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

static void sInitializeGameSystems(uint32_t m, uint32_t n, uint32_t k)
{
	sgGameBoard.Initialize(m, n, k);
	sgMoveHistory.Initialize(
		[](const PlayerMove& move) { sgGameBoard.Unmark(move.playerID, move.position); },
		[](const PlayerMove& move) { sgGameBoard.Mark(move.playerID, move.position); });
	sgConsoleInterface.Initialize();

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
