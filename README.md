# Console-Tic-Tac-Toe
ConsoleTicTacToe - Created by Eduardo Rodrigues (edrodrigues.com)

A simple 2-player tic-tac-toe game for the Windows console.

# Usage
usage: ConsoleTicTacToe m n k [-fancy]

## Input Arguments:
- m               (m >= 3) The number of columns in the game board.
- n               (n >= 3) The number of rows in the game board.
- k               (k >= 3) The number of marks a player must get in a row to win.
- [-fancy]        (Optional) Indicates the fancier 'graphical' UI should be used.

## Fancy-mode Controls:
- Mouse Move      Change the currently selected cell.
- Mouse Click     Places a marker at the selected cell and ends the current turn.
- Ctrl+Z          Moves back a turn, reverting a marker placement.
- Ctrl+Y          Moves forward a turn, re-placing a reverted marker placement.
- Space           Clears the current game board and restarts the game.
- ESC             Ends the game and exits this console application.

## Basic-mode Commands:
- mark <x> <y>    Places a marker at the given coordinates and ends the current turn.
- undo            Moves back a turn, reverting a marker placement.
- redo            Moves forward a turn, re-placing a reverted marker placement.
- help            Prints this help message.
- status          Prints the current state of the game.
- reset           Clears the current game board and restarts the game.
- exit            Ends the game and exits this console application.
- quit            Ends the game and exits this console application.

# Notes
- Built using Microsoft Visual Studio Community 2017 (15.9.11)
- Tested on Microsoft Windows 10 Pro (10.0.17134) using Command Line and Powershell

# Time Estimates
- 3 hours for basic m,n,k game state and undo history management.
- 2 hours for BasicGame, the simple text-command version of the game.
- 1 weekend worth of hours for FancyGame & ConsoleInterface, the advanced 'graphical' version of the game.