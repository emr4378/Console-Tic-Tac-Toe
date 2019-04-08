#pragma once

#include "ConsoleInterface.h"
#include "GameSimulation.h"

namespace tictactoe
{
	class FancyGame : public GameSimulation
	{
	public:
		FancyGame(uint32_t m, uint32_t n, uint32_t k);
		virtual ~FancyGame();

		virtual void Update() override;

	private:
		void OnKeyEvent(const KEY_EVENT_RECORD& event);
		void OnMouseEvent(const MOUSE_EVENT_RECORD& event);
		void OnResizeEvent(const ConsoleSize& newSize);

	private:
		ConsoleInterface _consoleInterface;

		bool _isGameAreaDirty;
		bool _isInfoPanelDirty;

		BoardPosition _currentMouseCell;
		BoardPosition _prevMouseCell;
		ConsoleRect _prevViewportRect;
	};
}
