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
		typedef UndoManager<PlayerMove> MoveHistory;

		static const uint16_t kNumPlayers = 2;

	public:
		GameSimulation(uint32_t m, uint32_t n, uint32_t k);
		virtual ~GameSimulation();

		virtual bool Update() = 0;
		virtual void Reset();

		const GameBoard& GetGameBoard() const { return _gameBoard; }
		const MoveHistory& GetMoveHistory() const { return _moveHistory; }

		GameStatus GetGameStatus() const { return _gameStatus; }
		PlayerID GetActivePlayer() const { return _gameStatus == GameStatus::Active ? _activePlayer : kInvalidPlayerID; }
		PlayerID GetWinningPlayer() const { return _gameStatus == GameStatus::Won ? _gameBoard.GetWinningPlayer() : kInvalidPlayerID; }

		MarkResult Mark(const BoardPosition& position);
		bool Undo();
		bool Redo();

	protected:
		void UpdateGameStatus();
		virtual void ApplyUndo(const PlayerMove& move);
		virtual void ApplyRedo(const PlayerMove& move);

	private:
		GameBoard _gameBoard;
		MoveHistory _moveHistory;

		PlayerID _activePlayer;
		GameStatus _gameStatus;
	};
}
