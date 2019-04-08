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
		bool Undo();
		bool Redo();

	protected:
		void UpdateGameStatus();
		virtual void ApplyUndo(const PlayerMove& move);
		virtual void ApplyRedo(const PlayerMove& move);

	protected:
		GameBoard _gameBoard;

	private:
		UndoManager<PlayerMove> _moveHistory;

		PlayerID _activePlayer;
		GameStatus _gameStatus;
	};
}
