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

	// An abstract base class for a 2-player m,n,k-game simulation.
	// Maintains the game board, active player, and undo history state,
	// and provides an interface for manipulating that state (see Mark(), Undo(), and Redo()).
	class GameSimulation
	{
	public:
		typedef UndoManager<PlayerMove> MoveHistory;

		static const uint16_t kNumPlayers = 2;

		static const char* GetPlayerName(PlayerID playerID);
		static char GetPlayerChar(PlayerID playerID);

	public:
		GameSimulation(uint16_t m, uint16_t n, uint16_t k);
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
