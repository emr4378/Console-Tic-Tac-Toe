#include "BasicGame.h"
#include "FancyGame.h"

#include <iomanip>
#include <iostream>
#include <string>

static tictactoe::GameSimulation* sgGame = nullptr;
static void sCreateGameSimulation(uint16_t m, uint16_t n, uint16_t k, bool isFancy);
static void sDestroyGameSimulation();

static BOOL WINAPI sConsoleCtrlHandler(DWORD dwCtrlType);

static bool sTryParseUInt(const std::string& str, uint16_t minValue, uint16_t* outValue);
static void sPrintUsage();

int main(int argc, char** argv)
{
	if (argc < 4 || argc > 5)
	{
		sPrintUsage();
		return EXIT_FAILURE;
	}

	// Parse the first 3 parameters (m, n, k).
	uint16_t m;
	uint16_t n;
	uint16_t k;
	if (!sTryParseUInt(argv[1], 3, &m) ||
		!sTryParseUInt(argv[2], 3, &n) ||
		!sTryParseUInt(argv[3], 3, &k))
	{
		sPrintUsage();
		return EXIT_FAILURE;
	}

	// Parse the -fancy parameter, if given.
	bool isFancy = false;
	if (argc == 5)
	{
		if (strcmp(argv[4], "-fancy") == 0)
		{
			isFancy = true;
		}
		else
		{
			sPrintUsage();
			return EXIT_FAILURE;
		}
	}

	// Create and run the game simulation.
	sCreateGameSimulation(m, n, k, isFancy);
	while (sgGame != nullptr)
	{
		if (!sgGame->Update())
		{
			break;
		}
	}
	sDestroyGameSimulation();

	return EXIT_SUCCESS;
}

static void sCreateGameSimulation(uint16_t m, uint16_t n, uint16_t k, bool isFancy)
{
	if (sgGame == nullptr)
	{
		SetConsoleCtrlHandler(sConsoleCtrlHandler, TRUE);
		sgGame = isFancy ?
			static_cast<tictactoe::GameSimulation*>(new tictactoe::FancyGame(m, n, k)) :
			static_cast<tictactoe::GameSimulation*>(new tictactoe::BasicGame(m, n, k));
	}
}

static void sDestroyGameSimulation()
{
	if (sgGame != nullptr)
	{
		SetConsoleCtrlHandler(sConsoleCtrlHandler, FALSE);
		delete sgGame;
		sgGame = nullptr;
	}
}

static BOOL WINAPI sConsoleCtrlHandler(DWORD dwCtrlType)
{
	// This ensures everything is cleaned up regardless of how the game is closed.
	// Primarily needed to ensure ConsoleInterface restores the initial console state.
	sDestroyGameSimulation();
	return false;
}

static bool sTryParseUInt(const std::string& str, uint16_t minValue, uint16_t* outValue)
{
	bool result = false;
	*outValue = 0;

	try
	{
		size_t pos;
		auto value = std::stoul(str, &pos);
		if (value >= minValue && value <= UINT16_MAX && pos == str.length())
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

static void sPrintUsage()
{
	std::cout << "ConsoleTicTacToe - Created by Eduardo Rodrigues (edrodrigues.com)" << std::endl;
	std::cout << std::endl;
	std::cout << "A simple 2-player tic-tac-toe game for the Windows console." << std::endl;
	std::cout << std::endl;
	std::cout << "usage: ConsoleTicTacToe m n k [-fancy]" << std::endl;

	auto printSubItem = [](const char* itemName, const char* itemDesc)
	{
		std::cout << "  " << std::left << std::setw(16) << itemName << itemDesc << std::endl;
	};

	std::cout << "Input Arguments:" << std::endl;
	{
		printSubItem("m", "(m >= 3) The number of columns in the game board.");
		printSubItem("n", "(n >= 3) The number of rows in the game board.");
		printSubItem("k", "(k >= 3) The number of marks a player must get in a row to win.");
		printSubItem("[-fancy]", "(Optional) Indicates the fancier 'graphical' UI should be used.");
	}
	std::cout << std::endl;

	std::cout << "Fancy-mode Controls:" << std::endl;
	{
		printSubItem("Mouse Move", "Change the currently selected cell.");
		printSubItem("Mouse Click", "Places a marker at the selected cell and ends the current turn.");
		printSubItem("Ctrl+Z", "Moves back a turn, reverting a marker placement.");
		printSubItem("Ctrl+Y", "Moves forward a turn, re-placing a reverted marker placement.");
		printSubItem("Space", "Clears the current game board and restarts the game.");
		printSubItem("ESC", "Ends the game and exits this console application.");
	}
	std::cout << std::endl;
}
