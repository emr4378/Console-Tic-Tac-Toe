#include "BasicGame.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace tictactoe;

static bool sTryParseUInt(const std::string& str, uint16_t* outValue);
std::ostream& operator<<(std::ostream& os, const BoardPosition& position);
std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard);

BasicGame::BasicGame(uint16_t m, uint16_t n, uint16_t k) :
	GameSimulation(m, n, k),
	_isQuitRequested(false)
{
}

BasicGame::~BasicGame()
{
}

void BasicGame::Reset()
{
	GameSimulation::Reset();
	_isQuitRequested = false;
}

bool BasicGame::Update()
{
	ExecuteStatusCommand();

	bool isTurnOver = false;
	while (!isTurnOver)
	{
		if (GetGameStatus() == GameStatus::Active)
		{
			std::cout
				<< "[" << GetPlayerName(GetActivePlayer())
				<< " (" << GetPlayerChar(GetActivePlayer()) << ")"
				<< "] ";
		}
		std::cout << "Enter a command: ";

		std::string inputCommand;
		std::getline(std::cin, inputCommand);

		auto commandNameEnd = inputCommand.find(' ');
		if (inputCommand.compare(0, commandNameEnd, "mark") == 0)
		{
			isTurnOver = ExecuteMarkCommand(inputCommand.substr(commandNameEnd + 1));
		}
		else if (inputCommand.compare(0, commandNameEnd, "undo") == 0)
		{
			isTurnOver = ExecuteUndoCommand();
		}
		else if (inputCommand.compare(0, commandNameEnd, "redo") == 0)
		{
			isTurnOver = ExecuteRedoCommand();
		}
		else if (inputCommand.compare(0, commandNameEnd, "help") == 0)
		{
			ExecuteHelpCommand();
			isTurnOver = false;
		}
		else if (inputCommand.compare(0, commandNameEnd, "status") == 0)
		{
			ExecuteStatusCommand();
			isTurnOver = false;
		}
		else if (inputCommand.compare(0, commandNameEnd, "reset") == 0)
		{
			isTurnOver = ExecuteResetCommand();
		}
		else if (inputCommand.compare(0, commandNameEnd, "quit") == 0 ||
				inputCommand.compare(0, commandNameEnd, "exit") == 0)
		{
			isTurnOver = ExecuteQuitCommand();
		}
		else
		{
			std::cerr << "Error: Unrecognized command. Type 'help' for a list of available commands." << std::endl;
		}
	}

	return (_isQuitRequested == false);
}

void BasicGame::ApplyUndo(const PlayerMove& move)
{
	GameSimulation::ApplyUndo(move);
	std::cout
		<< "Marker '"
		<< GetPlayerChar(move.playerID)
		<< "' has been removed from "
		<< move.position
		<< "."
		<< std::endl;
}

void BasicGame::ApplyRedo(const PlayerMove& move)
{
	GameSimulation::ApplyRedo(move);
	std::cout
		<< "Marker '"
		<< GetPlayerChar(move.playerID)
		<< "' has been re-placed at "
		<< move.position
		<< "."
		<< std::endl;
}

bool BasicGame::ExecuteMarkCommand(std::string params)
{
	bool result = false;

	std::string xParam;
	std::string yParam;
	{
		size_t paramStart = 0;
		size_t paramEnd = params.find(' ', paramStart);
		xParam = params.substr(paramStart, paramEnd - paramStart);

		paramStart = paramEnd + 1;
		paramEnd = std::string::npos;
		yParam = params.substr(paramStart, paramEnd);
	}

	BoardPosition position;
	if (sTryParseUInt(xParam, &position.x) &&
		sTryParseUInt(yParam, &position.y))
	{
		switch (Mark(position))
		{
			case MarkResult::Success:
				std::cout << "Marker placed at " << position << std::endl;
				result = true;
				break;

			case MarkResult::PositionOutOfBounds:
				std::cerr << position << " is out of bounds." << std::endl;
				break;

			case MarkResult::PositionAlreadyMarked:
				std::cerr << position << " is already marked." << std::endl;
				break;

			case MarkResult::GameAlreadyOver:
				std::cerr << "Cannot mark position; game has ended." << std::endl;
				break;
		}
		static_assert(static_cast<int16_t>(MarkResult::Count) == 4, "BasicGame::ExecuteMarkCommand() needs updating.");
	}
	else
	{
		std::cerr << "Invalid input for 'mark <x> <y>' command." << std::endl;
	}

	return result;
}

bool BasicGame::ExecuteUndoCommand()
{
	bool result = Undo();
	if (!result)
	{
		std::cerr << "Error: Unable to perform undo." << std::endl;
	}
	return result;
}

bool BasicGame::ExecuteRedoCommand()
{
	bool result = Redo();
	if (!result)
	{
		std::cerr << "Error: Unable to perform redo." << std::endl;
	}
	return result;
}

bool BasicGame::ExecuteHelpCommand()
{
	auto printCmd = [](const char* cmd, const char* desc)
	{
		std::cout << "  " << std::left << std::setw(16) << cmd << desc << std::endl;
	};

	std::cout << "Available commands:" << std::endl;
	if (GetGameStatus() == GameStatus::Active)
	{
		printCmd("mark <x> <y>", "Places a marker at the given coordinates and ends the current turn.");
	}
	printCmd("undo",			"Moves back a turn, reverting a marker placement.");
	printCmd("redo",			"Moves forward a turn, re-placing a reverted marker placement.");
	printCmd("help",			"Prints this help message.");
	printCmd("status",			"Prints the current state of the game.");
	printCmd("reset",			"Clears the current game board and restarts the game.");
	printCmd("exit",			"Ends the game and exits this console application.");
	printCmd("quit",			"Ends the game and exits this console application.");

	return true;
}

bool BasicGame::ExecuteStatusCommand()
{
	std::cout << std::endl;
	{
		std::cout << GetGameBoard();

		switch (GetGameStatus())
		{
			case GameStatus::Active:
				std::cout
					<< GetPlayerName(GetActivePlayer())
					<< " (" << GetPlayerChar(GetActivePlayer()) << ")"
					<< "'s turn."
					<< std::endl;
				break;

			case GameStatus::Won:
				std::cout
					<< GetPlayerName(GetWinningPlayer())
					<< " (" << GetPlayerChar(GetWinningPlayer()) << ")"
					<< " wins!"
					<< std::endl;
				break;

			case GameStatus::Draw:
				std::cout << "Draw - no player wins." << std::endl;
				break;
		}
		static_assert(static_cast<int>(GameStatus::Count) == 3, "BasicGame::ExecuteStatusCommand() needs updating.");
	}
	std::cout << std::endl;

	return true;
}

bool BasicGame::ExecuteResetCommand()
{
	std::cout << "Resetting the game to it's initial state." << std::endl;
	Reset();
	return true;
}

bool BasicGame::ExecuteQuitCommand()
{
	_isQuitRequested = true;
	return true;
}

static bool sTryParseUInt(const std::string& str, uint16_t* outValue)
{
	bool result = false;
	*outValue = 0;

	try
	{
		size_t pos;
		auto value = std::stoul(str, &pos);
		if (value < UINT16_MAX && pos == str.length())
		{
			*outValue = static_cast<uint16_t>(value);
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
	os << "(" << position.x << ", " << position.y << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const GameBoard& gameBoard)
{
	os << gameBoard.GetColumns() << "x" << gameBoard.GetRows();
	os << ", ";
	os << gameBoard.GetWinCondition() << "-in-a-row";
	os << std::endl;

	for (uint16_t row = 0; row < gameBoard.GetRows(); row++)
	{
		for (uint16_t column = 0; column < gameBoard.GetColumns(); column++)
		{
			os << BasicGame::GetPlayerChar(gameBoard.GetMarker({ column, row }));

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

	return os;
}
