RollBall for OS/2 - Version 1.3
================================


OVERVIEW
--------
RollBall is a 32-bit multithreaded Presentation Manager game for OS/2
and ArcaOS. Roll a ball to collect points while avoiding holes.

The red ball rolls around the playing field, bouncing off borders and
deflectors. Use the mouse to place deflectors to guide the ball toward
point symbols and away from holes.


HOW TO PLAY
-----------
- The ball (red circle) rolls continuously.
- Left mouse button  : place a \ deflector on the clicked cell.
- Right mouse button : place a / deflector on the clicked cell.
- Deflectors change the ball's direction when it passes over them.
- Collect colored dots to score points; avoid black holes.
- Start a game from Game > New Game (Ctrl+N).
- Pause or stop the game from the Game menu.


SCORING
-------
  Blue dot    1 point
  Green dot   5 points
  Magenta dot 10 points
  Violet dot  20 points

Black holes end the game immediately.


CONTROLS
--------
  Left mouse button   Place \ deflector
  Right mouse button  Place / deflector
  Ctrl+N              Start a new game
  Ctrl+P              Pause / Resume game
  Ctrl+Q              Quit current game (shows score)
  Ctrl+X              Exit application
  Ctrl+B              Toggle Background Run (keep running when unfocused)
  Ctrl+F              Toggle Frame Controls (borderless mode)


MENUS
-----
  Game > New Game          Start a new game (Ctrl+N)
  Game > Pause Game        Pause or resume
  Game > Quit Game         End current game (shows score) (Ctrl+Q)
  Game > Exit              Close the application

  Options > Score Info     Explain the scoring system
  Options > Language       Switch display language
  Options > Save settings on exit
  Options > Background Run Toggle auto-pause on focus loss
  Options > Frame Controls Toggle title bar / menu bar

  Help > About RollBall


FILE LIST
---------
  bin\rollball.exe    The game executable
  doc\Readme.txt      This file
  doc\Changelog.txt   Change history
  doc\LICENSE.txt     GNU General Public License v3
  src\                Source code


REQUIREMENTS
------------
  ArcaOS 5.x or OS/2 Warp 4


DISCLAIMER
----------
This software is provided "as is" without warranty of any kind.
See doc\LICENSE.txt for the full license terms.


AUTHORS
-------
  Stangl Roman     (original author, 1992-2017)
  Martin Iturbide  (ArcaOS port, 2026)
