#include "BasicGame.h"
#include "FancyGame.h"

#include <cassert>
#include <iostream>
#include <string>
#include <Windows.h>

#define NUM_PLAYERS 2

using namespace tictactoe;

static GameSimulation* sgGame = nullptr;

static BOOL sConsoleCtrlHandler(DWORD dwCtrlType)
{
	sgGame->Terminate();
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

	bool addCtrlHandlerResult = SetConsoleCtrlHandler(sConsoleCtrlHandler, true);
	assert(addCtrlHandlerResult);

	sgGame = isFancy ?
		static_cast<GameSimulation*>(new FancyGame()) :
		static_cast<GameSimulation*>(new BasicGame());

	sgGame->Initialize(m, n, k);
	while (true)
	{
		sgGame->Update();
	}
	sgGame->Terminate();

	bool removeCtrlHandlerResult = SetConsoleCtrlHandler(sConsoleCtrlHandler, false);
	assert(removeCtrlHandlerResult);

	return EXIT_SUCCESS;
}
