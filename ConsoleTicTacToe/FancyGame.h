#pragma once

#include "ConsoleInterface.h"
#include "GameSimulation.h"

namespace tictactoe
{
	class FancyGame : public GameSimulation
	{
	public:
		FancyGame(uint16_t m, uint16_t n, uint16_t k);
		virtual ~FancyGame();

		virtual bool Update() override;
		virtual void Reset() override;

	private:
		void OnKeyEvent(const KEY_EVENT_RECORD& event);
		void OnMouseEvent(const MOUSE_EVENT_RECORD& event);
		void OnResizeEvent(const ConsoleSize& newSize);

		void DrawCellBorderRightSide(const ConsoleRect& borderRect);
		void DrawCellBorderBottomSide(const ConsoleRect& borderRect);
		void DrawPlayerMarker(const ConsoleRect& markerRect, PlayerID playerID);
		void DrawPlayerMarker(const ConsoleRect& markerRect, PlayerID playerID, ConsoleColor color);
		void DrawPlayerMarkerWinBackground(const ConsoleRect& markerRect);

	private:
		ConsoleInterface _consoleInterface;

		bool _isGameAreaDirty;
		bool _isInfoPanelDirty;
		bool _isQuitRequested;

		BoardPosition _currentMouseCell;
		BoardPosition _prevMouseCell;
		ConsoleRect _prevViewportRect;
	};
}
