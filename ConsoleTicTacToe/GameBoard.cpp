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
	_grid(nullptr),
	_markerCount(0),
	_winningPlayerID(kInvalidPlayerID),
	_winningPositions()
{
	assert(_winCondition > 1);

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
	if (_winningPlayerID != kInvalidPlayerID)
	{
		result = MarkResult::GameAlreadyOver;
	}
	else if (!IsValidPosition(position))
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
		_markerCount++;
		CheckForWin(playerID, position);
		result = MarkResult::Success;
	}
	return result;
}

UnmarkResult GameBoard::Unmark(PlayerID playerID, const BoardPosition& position)
{
	assert(playerID != kInvalidPlayerID);

	UnmarkResult result;
	if (!IsValidPosition(position))
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
		_markerCount--;
		ClearWin();
		result = UnmarkResult::Success;
	}
	return result;
}

void GameBoard::Clear()
{
	for (uint32_t row = 0; row < _rows; row++)
	{
		for (uint32_t column = 0; column < _columns; column++)
		{
			_grid[row][column] = kInvalidPlayerID;
		}
	}
	_markerCount = 0;

	ClearWin();
}

bool GameBoard::IsValidPosition(const BoardPosition& position) const
{
	return sInRangeGrid(position, _columns, _rows);
}

PlayerID GameBoard::GetMarker(const BoardPosition& position) const
{
	assert(IsValidPosition(position));
	return _grid[position.y][position.x];
}

void GameBoard::CheckForWin(PlayerID playerID, const BoardPosition& position)
{
	assert(_winningPlayerID == kInvalidPlayerID);
	assert(_winningPositions.empty());

	struct Offset {
		int16_t x;
		int16_t y;
	};

	const Offset offsets[4] =
	{
		{ -1,  1 },	// '/' - forward slash
		{ -1,  0 }, // '-' - horizontal
		{ -1, -1 },	// '\' - backslash
		{  0, -1 },	// '|' - vertical
	};
	const uint16_t remainingSteps = (_winCondition - 1);

	for (int i = 0; i < 4; i++)
	{
		const Offset& offset = offsets[i];
		uint16_t a = CountConsecutive(playerID, position, offset.x, offset.y, remainingSteps);
		uint16_t b = CountConsecutive(playerID, position, -offset.x, -offset.y, remainingSteps);
		if (a + b >= remainingSteps)
		{
			for (int16_t i = -a; i <= b; i++)
			{
				BoardPosition temp = { position.x - (i * offset.x), position.y - (i * offset.y) };
				_winningPositions.push_back(temp);
			}
		}
	}

	if (!_winningPositions.empty())
	{
		_winningPlayerID = playerID;
	}
}

void GameBoard::ClearWin()
{
	_winningPositions.clear();
	_winningPlayerID = kInvalidPlayerID;
}

uint16_t GameBoard::CountConsecutive(
	PlayerID marker,
	BoardPosition position,
	int16_t xOffset, int16_t yOffset,
	uint16_t remainingSteps) const
{
	position.x += xOffset;
	position.y += yOffset;
	remainingSteps--;

	uint16_t result = 0;
	if (IsValidPosition(position) &&
		GetMarker(position) == marker)
	{
		result += 1;
		if (remainingSteps > 0)
		{
			result += CountConsecutive(marker, position, xOffset, yOffset, remainingSteps);
		}
	}
	return result;
}
