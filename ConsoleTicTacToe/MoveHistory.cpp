#include "MoveHistory.h"

using namespace tictactoe;

MoveHistory::MoveHistory(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc) :
	_applyUndoFunc(std::move(applyUndoFunc)),
	_applyRedoFunc(std::move(applyRedoFunc)),
	_moveList(),
	_undoPosition(_moveList.end())
{
}

void MoveHistory::Add(const PlayerMove& move)
{
	_undoPosition = _moveList.erase(_undoPosition, _moveList.end());
	_moveList.push_back(move);
}

void MoveHistory::Undo()
{
	if (_undoPosition != _moveList.begin())
	{
		_undoPosition--;
		_applyUndoFunc(*_undoPosition);
	}
}

void MoveHistory::Redo()
{
	if (_undoPosition != _moveList.end())
	{
		_applyRedoFunc(*_undoPosition);
		_undoPosition++;
	}
}
