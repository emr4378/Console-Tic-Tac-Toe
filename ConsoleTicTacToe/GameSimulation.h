#pragma once

#include "Common.h"
#include "GameBoard.h"
#include "UndoManager.h"

namespace tictactoe
{
	class GameSimulation
	{
	public:
		GameSimulation(uint32_t m, uint32_t n, uint32_t k);
		virtual ~GameSimulation();

		virtual void Update() = 0;

		MarkResult Mark(const BoardPosition& position);
		void Undo();
		void Redo();

	protected:
		void OnUndo(const PlayerMove& move);
		void OnRedo(const PlayerMove& move);

		// TODO: private:
		GameBoard _gameBoard;
		UndoManager<PlayerMove> _moveHistory;

		PlayerID _activePlayer;
	};
}
