# GAME-ACTION-Rollball

A 32-bit multithreaded Presentation Manager ball-rolling game for OS/2 and ArcaOS.

![Rollball ScreenShot](/doc/RollBall.png)

## Overview

Roll the red ball to collect colored dots for points, while placing deflectors with
the mouse to steer it away from holes. The ball speeds up as your score increases.

## Controls

| Input | Action |
|-------|--------|
| Left mouse button | Place `\` deflector |
| Right mouse button | Place `/` deflector |
| Ctrl+N | New Game |
| Ctrl+P | Pause / Resume |
| Ctrl+Q | Quit current game (shows score) |
| Ctrl+X | Exit |
| Ctrl+B | Background Run toggle |
| Ctrl+F | Frame Controls (borderless) |

## Build Instructions

Requires Open Watcom C/C++ and the OS/2 Toolkit 4.5 headers.

```
compile-wat.cmd
```

Or directly:

```
wmake -f makefile.wat all
```

Output is placed in `bin\rollball.exe`.

## Project Layout

```
src/            Source files (.c, .h, .rc, .def, .ico, .ptr)
bin/            Build output (.exe, .obj, .res, .map)
doc/            Documentation (Readme.txt, Changelog.txt, LICENSE.txt)
img/            Bitmap resources
legacy/         Original unmodified source kept for reference
makefile.wat    Open Watcom build file
compile-wat.cmd OS/2 CMD build script
```

## Changelog Summary

**1.3 (2026-09-10)** — New Game (Ctrl+N) / Quit Game (Ctrl+Q) shortcuts;
1024×768 default window; 24×24 scaled graphics; game-over message localized.

**1.01 (2026-09-10)** — Open Watcom port; Game/Options/Help menus;
Ctrl+X/P/B/F shortcuts; 6-language support; settings persistence.

**1.00 (2017)** — 25th anniversary update, IBM VisualAge C++.

**0.99 (1992)** — Original release by Stangl Roman.

## License

GNU GPL v3 — see [doc/LICENSE.txt](doc/LICENSE.txt)

## Authors

- Stangl Roman (original author, 1992–2017)
- Martin Iturbide (ArcaOS port, 2026)

## Links

- https://www.os2world.com/games
