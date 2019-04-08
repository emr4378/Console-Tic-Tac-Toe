#include "GameBoard.h"

#include <cassert>

using namespace tictactoe;

static constexpr bool sInRangeArray(uint32_t val, uint32_t min, uint32_t max)
{
	return (min <= val && val < max);
}

static constexpr bool sInRangeGrid(const BoardPosition& position, uint32_t columns, uint32_t rows)
{
	return (sInRangeArray(position.x, 0, columns) && sInRangeArray(position.y, 0, rows));
}

GameBoard::GameBoard() :
	_isInitialized(false),
	_columns(0),
	_rows(0),
	_winCondition(0),
	_grid(nullptr)
{
}

void GameBoard::Initialize(uint32_t columns, uint32_t rows, uint32_t winCondition)
{
	if (!_isInitialized)
	{
		_columns = columns;
		_rows = rows;
		_winCondition = winCondition;

		_grid = new PlayerID*[_rows];
		for (uint32_t row = 0; row < _rows; row++)
		{
			_grid[row] = new PlayerID[_columns];
			for (uint32_t column = 0; column < _columns; column++)
			{
				_grid[row][column] = kInvalidPlayerID;
			}
		}

		_isInitialized = true;
	}
}

void GameBoard::Terminate()
{
	if (_isInitialized)
	{
		if (_grid != nullptr)
		{
			for (uint32_t row = 0; row < _rows; row++)
			{
				delete[] _grid[row];
			}
			delete[] _grid;
			_grid = nullptr;
		}

		_columns = 0;
		_rows = 0;
		_winCondition = 0;
		_isInitialized = false;
	}
}

MarkResult GameBoard::Mark(PlayerID playerID, const BoardPosition& position)
{
	assert(_isInitialized);
	assert(playerID != kInvalidPlayerID);

	MarkResult result;
	if (!sInRangeGrid(position, _columns, _rows))
	{
		result = MarkResult::PositionOutOfBounds;
	}
	else if (GetMarker(position) != kInvalidPlayerID)
	{
		result = MarkResult::PositionAlreadyMarked;
	}
	else
	{
		_grid[position.y][position.x] = playerID;
		result = MarkResult::Success;
	}
	return result;
}

UnmarkResult GameBoard::Unmark(PlayerID playerID, const BoardPosition& position)
{
	assert(_isInitialized);
	assert(playerID != kInvalidPlayerID);

	UnmarkResult result;
	if (!sInRangeGrid(position, _columns, _rows))
	{
		result = UnmarkResult::PositionOutOfBounds;
	}
	else if (GetMarker(position) != playerID)
	{
		result = UnmarkResult::PositionNotMarkedByPlayer;
	}
	else
	{
		_grid[position.y][position.x] = kInvalidPlayerID;
		result = UnmarkResult::Success;
	}
	return result;
}

PlayerID GameBoard::GetMarker(const BoardPosition& position) const
{
	assert(_isInitialized);
	assert(sInRangeGrid(position, _columns, _rows));
	return _grid[position.y][position.x];
}
