#include "GameSimulation.h"

using namespace tictactoe;

static PlayerID sGetNextPlayerID(PlayerID id);
static PlayerID sGetPrevPlayerID(PlayerID id);

GameSimulation::GameSimulation(uint32_t m, uint32_t n, uint32_t k) :
	_gameBoard(m, n, k),
	_moveHistory(),
	_activePlayer(0),
	_gameStatus(GameStatus::Active)
{
	_moveHistory.SetCallbacks(
		[=](const PlayerMove& move) { this->OnUndo(move); },
		[=](const PlayerMove& move) { this->OnRedo(move); });
}

GameSimulation::~GameSimulation()
{
	_moveHistory.SetCallbacks(nullptr, nullptr);
}

void GameSimulation::Reset()
{
	_gameBoard.Clear();
	_moveHistory.Clear();
	_activePlayer = 0;
	UpdateGameStatus();
}

MarkResult GameSimulation::Mark(const BoardPosition& position)
{
	auto result = _gameBoard.Mark(_activePlayer, position);
	if (result == MarkResult::Success)
	{
		_moveHistory.Add({ _activePlayer, position });
		_activePlayer = sGetNextPlayerID(_activePlayer);
		UpdateGameStatus();
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

void GameSimulation::UpdateGameStatus()
{
	if (_gameBoard.GetWinningPlayerID() != kInvalidPlayerID)
	{
		_gameStatus = GameStatus::Won;
	}
	else if (_gameBoard.IsFilled())
	{
		_gameStatus = GameStatus::Draw;
	}
	else
	{
		_gameStatus = GameStatus::Active;
	}
}

void GameSimulation::OnUndo(const PlayerMove& move)
{
	auto result = _gameBoard.Unmark(move.playerID, move.position);
	assert(result == UnmarkResult::Success);
	_activePlayer = sGetPrevPlayerID(_activePlayer);
	UpdateGameStatus();
}


void GameSimulation::OnRedo(const PlayerMove& move)
{
	auto result = _gameBoard.Mark(move.playerID, move.position);
	assert(result == MarkResult::Success);
	_activePlayer = sGetNextPlayerID(_activePlayer);
	UpdateGameStatus();
}

static PlayerID sGetNextPlayerID(PlayerID id)
{
	return (id + 1) % GameSimulation::kNumPlayers;
}

static PlayerID sGetPrevPlayerID(PlayerID id)
{
	return (id + GameSimulation::kNumPlayers - 1) % GameSimulation::kNumPlayers;
}
