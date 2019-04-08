#include "GameSimulation.h"

using namespace tictactoe;

#define NUM_PLAYERS 2

static PlayerID sGetNextPlayerID(PlayerID id);
static PlayerID sGetPrevPlayerID(PlayerID id);

GameSimulation::GameSimulation() :
	_gameBoard(),
	_moveHistory(),
	_activePlayer(0)
{
}

GameSimulation::~GameSimulation()
{
}

void GameSimulation::Initialize(uint32_t m, uint32_t n, uint32_t k)
{
	_gameBoard.Initialize(m, n, k);
	_moveHistory.Initialize(
		[=](const PlayerMove& move) { this->OnUndo(move); },
		[=](const PlayerMove& move) { this->OnRedo(move); });
}

void GameSimulation::Terminate()
{
	_moveHistory.Terminate();
	_gameBoard.Terminate();
}

MarkResult GameSimulation::Mark(const BoardPosition& position)
{
	auto result = _gameBoard.Mark(_activePlayer, position);
	if (result == MarkResult::Success)
	{
		_moveHistory.Add({ _activePlayer, position });
		_activePlayer = sGetNextPlayerID(_activePlayer);
	}
	return result;
}

void GameSimulation::Undo()
{
	_moveHistory.Undo();
	
}

void GameSimulation::Redo()
{
	_moveHistory.Redo();
}

void GameSimulation::OnUndo(const PlayerMove& move)
{
	auto result = _gameBoard.Unmark(move.playerID, move.position);
	assert(result == UnmarkResult::Success);
	_activePlayer = sGetPrevPlayerID(_activePlayer);
}


void GameSimulation::OnRedo(const PlayerMove& move)
{
	auto result = _gameBoard.Mark(move.playerID, move.position);
	assert(result == MarkResult::Success);
	_activePlayer = sGetNextPlayerID(_activePlayer);
}

static PlayerID sGetNextPlayerID(PlayerID id)
{
	return (id + 1) % NUM_PLAYERS;
}

static PlayerID sGetPrevPlayerID(PlayerID id)
{
	return (id + NUM_PLAYERS - 1) % NUM_PLAYERS;
}
