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

GameBoard::GameBoard(uint32_t columns, uint32_t rows, uint32_t winCondition) :
	_columns(columns),
	_rows(rows),
	_winCondition(winCondition),
	_grid(nullptr)
{
	_grid = new PlayerID*[_rows];
	for (uint32_t row = 0; row < _rows; row++)
	{
		_grid[row] = new PlayerID[_columns];
		for (uint32_t column = 0; column < _columns; column++)
		{
			_grid[row][column] = kInvalidPlayerID;
		}
	}
}

GameBoard::~GameBoard()
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
}

MarkResult GameBoard::Mark(PlayerID playerID, const BoardPosition& position)
{
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
	assert(sInRangeGrid(position, _columns, _rows));
	return _grid[position.y][position.x];
}
