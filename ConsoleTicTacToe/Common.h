#pragma once

#include <cstdint>

namespace tictactoe
{
	typedef uint16_t PlayerID;
	static const PlayerID kInvalidPlayerID = -1;

	struct BoardPosition
	{
		uint32_t x;
		uint32_t y;
	};
}
