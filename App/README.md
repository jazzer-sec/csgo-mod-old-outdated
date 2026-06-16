# Standalone interactive app (`App/`)

A self-contained windowed build: the menu, HUD, and the real feature pipeline
(`create_move`) all compiled directly into one executable, running against a
**simulated** HvH scene. There is no game and no injection — the cheat logic
drives scripted bots inside this process so you can interact with the menu and
watch the decisions react live.

- `Scene.h` — backend-agnostic frame: scene + overlays + sim + interactive menu.
- `main_sdl.cpp` — thin SDL2 window/input loop calling `Scene.h`.
- `headless_test.cpp` — drives the frame with synthetic input (no window) and
  asserts the menu mutates `cfg::g_cfg`; this is what's verified in CI/locally.

## Controls
- `INSERT` — toggle the menu
- left-click — drive the menu (tabs, checkboxes, sliders, combos cycle on click)
- `ESC` — quit

## Build

### Windows — single self-contained .exe (recommended, no extra libs)

The Win32/GDI build (`main_win32.cpp`) uses only Windows system DLLs, so the
result is one double-clickable file with no SDL/FreeType to ship. With MinGW
(MSYS2 MINGW64 shell, or a cross toolchain):

```sh
x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows App/main_win32.cpp \
  Harness/Render2D.cpp Harness/TextGdi.cpp Harness/Hud.cpp Harness/Menu.cpp \
  -lgdi32 -luser32 -static -static-libgcc -static-libstdc++ -o jazzhook.exe
```

(On a native MSYS2 shell drop the `x86_64-w64-mingw32-` prefix.)

### Cross-platform — SDL2 build

The SDL2 build (`main_sdl.cpp`) also runs on Linux/macOS. Needs **SDL2** +
**FreeType**. On Windows use MSYS2 (MINGW64 shell):

```sh
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-freetype \
          mingw-w64-x86_64-pkg-config

g++ -std=c++17 -O2 App/main_sdl.cpp Harness/Render2D.cpp Harness/Hud.cpp \
    Harness/Menu.cpp $(pkg-config --cflags --libs sdl2 freetype2) -o jazzhook_app.exe
./jazzhook_app.exe
```

Linux (`apt install libsdl2-dev libfreetype6-dev`) and macOS (`brew install sdl2
freetype`) use the same `g++` command (drop the `.exe`).

## Verify without a window

```sh
g++ -std=c++17 -O2 App/headless_test.cpp Harness/Render2D.cpp Harness/Hud.cpp \
    Harness/Menu.cpp $(pkg-config --cflags --libs freetype2) -o /tmp/apptest && /tmp/apptest
```

Runs the interaction checks and writes `App/out/standalone_frame.png`.
