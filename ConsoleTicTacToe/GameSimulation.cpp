#include "GameSimulation.h"

using namespace tictactoe;

static PlayerID sGetNextPlayerID(PlayerID id);
static PlayerID sGetPrevPlayerID(PlayerID id);

const char* GameSimulation::GetPlayerName(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return "Player 1";
		case 1:		return "Player 2";
		default:	return nullptr;
	}
	static_assert(GameSimulation::kNumPlayers == 2, "GetPlayerName() needs updating.");
}

char GameSimulation::GetPlayerChar(PlayerID playerID)
{
	switch (playerID)
	{
		case 0:		return 'X';
		case 1:		return 'O';
		default:	return ' ';
	}
	static_assert(GameSimulation::kNumPlayers == 2, "GetPlayerChar() needs updating.");
}

GameSimulation::GameSimulation(uint16_t m, uint16_t n, uint16_t k) :
	_gameBoard(m, n, k),
	_moveHistory(),
	_activePlayer(0),
	_gameStatus(GameStatus::Active)
{
	_moveHistory.SetCallbacks(
		[=](const PlayerMove& move) { this->ApplyUndo(move); },
		[=](const PlayerMove& move) { this->ApplyRedo(move); });
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

bool GameSimulation::Undo()
{
	return _moveHistory.Undo();
	
}

bool GameSimulation::Redo()
{
	return _moveHistory.Redo();
}

void GameSimulation::UpdateGameStatus()
{
	if (_gameBoard.GetWinningPlayer() != kInvalidPlayerID)
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

void GameSimulation::ApplyUndo(const PlayerMove& move)
{
	auto result = _gameBoard.Unmark(move.playerID, move.position);
	assert(result == UnmarkResult::Success);
	_activePlayer = sGetPrevPlayerID(_activePlayer);
	UpdateGameStatus();
}


void GameSimulation::ApplyRedo(const PlayerMove& move)
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
