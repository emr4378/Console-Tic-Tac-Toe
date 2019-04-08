#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <list>

namespace tictactoe
{
	// Maintains a history of generic objects, allowing them to be un- & re- applied with the given ApplyFuncs.
	template <typename T>
	class UndoManager
	{
	public:
		typedef std::function<void(const T&)> ApplyFunc;

		UndoManager();

		void SetCallbacks(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc);

		void Add(const T& move);
		void Undo();
		void Redo();

		uint32_t GetAvailableUndosCount() const;
		uint32_t GetAvailableRedosCount() const;

	private:
		ApplyFunc _applyUndoFunc;
		ApplyFunc _applyRedoFunc;

		std::list<T> _moveList;
		typename std::list<T>::const_iterator _undoPosition;
	};

	#pragma region UndoManager<T> Implementation

	template <typename T>
	UndoManager<T>::UndoManager() :
		_applyUndoFunc(nullptr),
		_applyRedoFunc(nullptr),
		_moveList(),
		_undoPosition(_moveList.end())
	{
	}

	template <typename T>
	void UndoManager<T>::SetCallbacks(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc)
	{
		_applyUndoFunc = std::move(applyUndoFunc);
		_applyRedoFunc = std::move(applyRedoFunc);
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

	template <typename T>
	uint32_t UndoManager<T>::GetAvailableUndosCount() const
	{
		return static_cast<uint32_t>(std::distance(_moveList.cbegin(), _undoPosition));
	}

	template <typename T>
	uint32_t UndoManager<T>::GetAvailableRedosCount() const
	{
		return static_cast<uint32_t>(std::distance(_undoPosition, _moveList.cend()));
	}

	#pragma endregion
}
