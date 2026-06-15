# UI/HUD preview harness

A standalone, dependency-free C++17 program that renders the custom **jazzhook**
interface to PNG images, so the look/feel can be reviewed without injecting into
the game (handy when there's no DX9/Win32 around).

The 2D draw API in `Render2D` deliberately mirrors ArcticTech's `CRender`
(`BoxFilled` / `GradientBox` / `Line` / `Circle` / `Text` / …). The menu and HUD
code is written against that API, so porting into the real mod means swapping the
backend (software framebuffer → DirectX 9), not rewriting the UI.

## Build & run
```sh
g++ -std=c++17 -O2 Harness/*.cpp -o /tmp/uiharness
./uiharness            # writes Harness/out/*.png
```

## Files
- `PngWriter.h`  — minimal PNG encoder (stored-deflate, CRC32/Adler32). No deps.
- `Render2D.*`   — software RGBA framebuffer + AA primitives + bitmap font.
- `Anim.h`       — eased, framerate-independent interpolation (the animation core).
- `Theme.h`      — accent-driven palette (default: violet).
- `Menu.*`       — animated menu: header/logo, tab bar w/ gliding underline,
                   animated widgets (toggle, slider+tooltip, combo, keybind, color).
- `Hud.*`        — watermark, keybind list, spectator list, hit log, bottom
                   indicators, off-screen + manual-AA arrows, crosshair, ESP.
- `main.cpp`     — renders 4 mocked frames into `out/`.

## Frames
1. `01_scene.png` — full HvH overlay scene, menu closed.
2. `02_menu.png` — menu open, settled.
3. `03_menu_interact.png` — combo dropdown expanded + slider drag tooltip.
4. `04_menu_opening.png` — mid-open keyframe (fade + slide, underline gliding).

All content is mocked. In the mod these bind to the entity list / game events /
`m_KeyBinds` / `config_t` as described in the project plan.
