#pragma once

#include "Common.h"

#include <functional>
#include <list>

namespace tictactoe
{
	struct PlayerMove
	{
		PlayerID playerID;
		BoardPosition position;
	};

	// Maintains a history of PlayerMoves to allow them to be un-/re-applied.
	class MoveHistory
	{
	public:
		typedef std::function<void(const PlayerMove&)> ApplyFunc;

		MoveHistory(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc);

		void Add(const PlayerMove& move);
		void Undo();
		void Redo();

	private:
		ApplyFunc _applyUndoFunc;
		ApplyFunc _applyRedoFunc;

		std::list<PlayerMove> _moveList;
		std::list<PlayerMove>::iterator _undoPosition;
	};
}
