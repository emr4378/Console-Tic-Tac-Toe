#pragma once

#include <functional>
#include <list>

namespace tictactoe
{
	// Maintains a history of generic objects, allowing them to be un- & re- applied with the given ApplyFuncs.
	template <typename T>
	class UndoManager
	{
	public:
		typedef std::function<void(const T&)> ApplyFunc;

		UndoManager(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc);

		void Add(const T& move);
		void Undo();
		void Redo();

	private:
		ApplyFunc _applyUndoFunc;
		ApplyFunc _applyRedoFunc;

		std::list<T> _moveList;
		typename std::list<T>::iterator _undoPosition;
	};

	#pragma region UndoManager<T> Implementation

	template <typename T>
	UndoManager<T>::UndoManager(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc) :
		_applyUndoFunc(std::move(applyUndoFunc)),
		_applyRedoFunc(std::move(applyRedoFunc)),
		_moveList(),
		_undoPosition(_moveList.end())
	{
	}

	template <typename T>
	void UndoManager<T>::Add(const T& move)
	{
		_undoPosition = _moveList.erase(_undoPosition, _moveList.end());
		_moveList.push_back(move);
	}

	template <typename T>
	void UndoManager<T>::Undo()
	{
		if (_undoPosition != _moveList.begin())
		{
			_undoPosition--;
			_applyUndoFunc(*_undoPosition);
		}
	}

	template <typename T>
	void UndoManager<T>::Redo()
	{
		if (_undoPosition != _moveList.end())
		{
			_applyRedoFunc(*_undoPosition);
			_undoPosition++;
		}
	}

	#pragma endregion
}
