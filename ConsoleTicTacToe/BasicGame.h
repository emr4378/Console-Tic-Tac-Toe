#pragma once

#include "GameSimulation.h"

namespace tictactoe
{
	class BasicGame : public GameSimulation
	{
	public:
		BasicGame(uint32_t m, uint32_t n, uint32_t k);
		virtual ~BasicGame();

		virtual bool Update() override;
		virtual void Reset() override;

	protected:
		virtual void ApplyUndo(const PlayerMove& move) override;
		virtual void ApplyRedo(const PlayerMove& move) override;

	private:
		bool ExecuteMarkCommand(std::string params);
		bool ExecuteUndoCommand();
		bool ExecuteRedoCommand();
		bool ExecuteHelpCommand();
		bool ExecuteStatusCommand();
		bool ExecuteResetCommand();
		bool ExecuteQuitCommand();

	private:
		bool _isQuitRequested;
	};
}