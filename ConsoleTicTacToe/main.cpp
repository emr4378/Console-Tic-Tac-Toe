#include "BasicGame.h"
#include "FancyGame.h"

#include <iostream>
#include <string>

static tictactoe::GameSimulation* sgGame = nullptr;
static void sCreateGameSimulation(uint32_t m, uint32_t n, uint32_t k, bool isFancy);
static void sDestroyGameSimulation();

static BOOL sConsoleCtrlHandler(DWORD dwCtrlType);

static bool sTryParseUInt(const std::string& str, uint32_t* outValue);

int main(int argc, char** argv)
{
	if (argc < 4 || argc > 5)
	{
		// TODO: printUsage()
		return EXIT_FAILURE;
	}

	// Parse the first 3 parameters (m, n, k).
	uint32_t m;
	uint32_t n;
	uint32_t k;
	if (!sTryParseUInt(argv[1], &m) ||
		!sTryParseUInt(argv[2], &n) ||
		!sTryParseUInt(argv[3], &k))
	{
		// TODO: printUsage()
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
			// TODO: printUsage()
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

static void sCreateGameSimulation(uint32_t m, uint32_t n, uint32_t k, bool isFancy)
{
	if (sgGame == nullptr)
	{
		SetConsoleCtrlHandler(sConsoleCtrlHandler, true);
		sgGame = isFancy ?
			static_cast<tictactoe::GameSimulation*>(new tictactoe::FancyGame(m, n, k)) :
			static_cast<tictactoe::GameSimulation*>(new tictactoe::BasicGame(m, n, k));
	}
}

static void sDestroyGameSimulation()
{
	if (sgGame != nullptr)
	{
		SetConsoleCtrlHandler(sConsoleCtrlHandler, false);
		delete sgGame;
		sgGame = nullptr;
	}
}

static BOOL sConsoleCtrlHandler(DWORD dwCtrlType)
{
	// This ensures everything is cleaned up regardless of how the game is closed.
	// Primarily needed to ensure ConsoleInterface restores the initial console state.
	sDestroyGameSimulation();
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
