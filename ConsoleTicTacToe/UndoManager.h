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

		void Initialize(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc);
		void Terminate();

		void Add(const T& move);
		void Undo();
		void Redo();

		uint32_t GetAvailableUndosCount() const;
		uint32_t GetAvailableRedosCount() const;

	private:
		bool _isInitialized;
		ApplyFunc _applyUndoFunc;
		ApplyFunc _applyRedoFunc;

		std::list<T> _moveList;
		typename std::list<T>::const_iterator _undoPosition;
	};

	#pragma region UndoManager<T> Implementation

	template <typename T>
	UndoManager<T>::UndoManager() :
		_isInitialized(false),
		_applyUndoFunc(nullptr),
		_applyRedoFunc(nullptr),
		_moveList(),
		_undoPosition()
	{
	}

	template <typename T>
	void UndoManager<T>::Initialize(ApplyFunc applyUndoFunc, ApplyFunc applyRedoFunc)
	{
		if (!_isInitialized)
		{
			_applyUndoFunc = std::move(applyUndoFunc);
			_applyRedoFunc = std::move(applyRedoFunc);
			_undoPosition = _moveList.end();
			_isInitialized = true;
		}
	}

	template <typename T>
	void UndoManager<T>::Terminate()
	{
		if (_isInitialized)
		{
			_applyUndoFunc = NULL;
			_applyRedoFunc = NULL;
			_moveList.clear();
			_undoPosition = _moveList.end();
			_isInitialized = false;
		}
	}

	template <typename T>
	void UndoManager<T>::Add(const T& move)
	{
		assert(_isInitialized);
		_undoPosition = _moveList.erase(_undoPosition, _moveList.end());
		_moveList.push_back(move);
	}

	template <typename T>
	void UndoManager<T>::Undo()
	{
		assert(_isInitialized);
		if (_undoPosition != _moveList.begin())
		{
			_undoPosition--;
			_applyUndoFunc(*_undoPosition);
		}
	}

	template <typename T>
	void UndoManager<T>::Redo()
	{
		assert(_isInitialized);
		if (_undoPosition != _moveList.end())
		{
			_applyRedoFunc(*_undoPosition);
			_undoPosition++;
		}
	}

	template <typename T>
	uint32_t UndoManager<T>::GetAvailableUndosCount() const
	{
		assert(_isInitialized);
		return static_cast<uint32_t>(std::distance(_moveList.cbegin(), _undoPosition));
	}

	template <typename T>
	uint32_t UndoManager<T>::GetAvailableRedosCount() const
	{
		assert(_isInitialized);
		return static_cast<uint32_t>(std::distance(_undoPosition, _moveList.cend()));
	}

	#pragma endregion
}
