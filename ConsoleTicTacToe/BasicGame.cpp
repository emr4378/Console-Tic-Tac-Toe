#include "BasicGame.h"

#include <iostream>
#include <string>

using namespace tictactoe;

std::ostream& operator<<(std::ostream& os, const BoardPosition& position);
std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard);

BasicGame::BasicGame() :
	GameSimulation()
{
}

BasicGame::~BasicGame()
{
}

void BasicGame::Initialize(uint32_t m, uint32_t n, uint32_t k)
{
	GameSimulation::Initialize(m, n, k);
}

void BasicGame::Terminate()
{
	GameSimulation::Terminate();
}

void BasicGame::Update()
{
	assert(!"Not Implemented");

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
