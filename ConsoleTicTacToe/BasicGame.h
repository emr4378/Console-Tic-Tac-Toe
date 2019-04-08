#pragma once

#include "GameSimulation.h"

namespace tictactoe
{
	//class Game
	//{
	//public:
	//	Game(uint32_t m, uint32_t n, uint32_t k);
	//
	//	void Update();
	//
	//private:
	//	void ExecuteHelp() const;
	//	void ExecuteStatus() const;
	//	void ExecuteMark(const std::string& params);
	//	void ExecuteUndo();
	//	void ExecuteRedo();
	//	void ExecuteQuit();
	//
	//	GameBoard _gameBoard;
	//	UndoManager<PlayerMove> _moveHistory;
	//};

	class BasicGame : public GameSimulation
	{
	public:
		BasicGame();
		virtual ~BasicGame();

		virtual void Initialize(uint32_t m, uint32_t n, uint32_t k) override;
		virtual void Terminate() override;
		virtual void Update() override;
	};
}