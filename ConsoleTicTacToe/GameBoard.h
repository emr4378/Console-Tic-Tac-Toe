#pragma once

#include "Common.h"

#include <vector>

namespace tictactoe
{
	enum class MarkResult
	{
		Success = 0,
		PositionOutOfBounds,
		PositionAlreadyMarked,
		GameAlreadyOver,

		Count
	};

	enum class UnmarkResult
	{
		Success = 0,
		PositionOutOfBounds,
		PositionNotMarkedByPlayer,

		Count
	};

	// Represents the 2-dimensional game board that a m,n,k-game is played on, where:
	// - m is the number of columns of the game board
	// - n is the number of rows of the game board
	// - k is the win condition, the number of sequential marks in any direction that a player must obtain to win
	// See https://en.wikipedia.org/wiki/M,n,k-game
	class GameBoard
	{
	public:
		typedef std::vector<BoardPosition> WinPositionList;

		GameBoard(uint32_t columns, uint32_t rows, uint32_t winCondition);
		virtual ~GameBoard();

		MarkResult Mark(PlayerID playerID, const BoardPosition& position);
		UnmarkResult Unmark(PlayerID playerID, const BoardPosition& position);
		void Clear();

		bool IsValidPosition(const BoardPosition& position) const;
		PlayerID GetMarker(const BoardPosition& position) const;

		uint32_t GetRows() const { return _rows; }
		uint32_t GetColumns() const { return _columns; }
		uint32_t GetWinCondition() const { return _winCondition; }

		PlayerID GetWinningPlayerID() const { return _winningPlayerID; }
		const WinPositionList& GetWinPositionList() const { return _winningPositions; }

	private:
		void CheckForWin(PlayerID playerID, const BoardPosition& position);
		void ClearWin();
		uint16_t CountConsecutive(
			PlayerID marker,
			BoardPosition position,
			int16_t xOffset, int16_t yOffset,
			uint16_t remainingSteps) const;

		uint32_t _columns;		// m
		uint32_t _rows;			// n
		uint32_t _winCondition;	// k

		PlayerID** _grid;

		PlayerID _winningPlayerID;
		WinPositionList _winningPositions;
	};
}
