#pragma once

#include "GameBoard.h"
#include "UndoManager.h"

namespace tictactoe
{
	enum class GameStatus
	{
		Active,
		Won,
		Draw,

		Count
	};

	class GameSimulation
	{
	public:
		static const uint16_t kNumPlayers = 2;

	public:
		GameSimulation(uint32_t m, uint32_t n, uint32_t k);
		virtual ~GameSimulation();

		virtual bool Update() = 0;
		virtual void Reset();

		PlayerID GetActivePlayer() const { return _gameStatus == GameStatus::Active ? _activePlayer : kInvalidPlayerID; }
		GameStatus GetGameStatus() const { return _gameStatus; }

		MarkResult Mark(const BoardPosition& position);
		void Undo();
		void Redo();

	protected:
		void UpdateGameStatus();
		void OnUndo(const PlayerMove& move);
		void OnRedo(const PlayerMove& move);

	protected:
		GameBoard _gameBoard;

	private:
		UndoManager<PlayerMove> _moveHistory;

		PlayerID _activePlayer;
		GameStatus _gameStatus;
	};
}
