# csgo-mod-old-outdated
a mod for outdated csgo with no multiplayer, simply for skill development in c++

## Modules

### JazzHook (`JazzHook/`)
A custom prediction module modeled on ArcticTech's `CPrediction`
(`Features/Misc/Prediction.cpp` ~line 338): the R8 Revolver's delayed-fire
timing. Holding the R8 trigger arms a `m_flPostponeFireReadyTime` and the round
only leaves on a later tick. JazzHook predicts that ready tick from a circular
buffer of recent commands so the rest of the mod knows exactly when the shot
lands.

- `JazzHook/JazzHook.h` / `JazzHook.cpp` — the module.
- `JazzHook/SDK/Stubs.h` — minimal self-contained SDK stand-ins (swap for real
  engine interfaces when wiring into a client).
- `JazzHook/demo.cpp` — standalone driver, no game required.

Build the demo:

```sh
g++ -std=c++17 JazzHook/JazzHook.cpp JazzHook/demo.cpp -o jazzhook_demo
./jazzhook_demo
```

### Feature layer (`src/`)
The actual cheat sections, organized by menu area, over self-contained SDK
stubs. `src/Config.h` (`cfg::g_cfg`) is the single source of truth shared by the
features and the menu, so a menu row and the code it drives never drift.

Each section is ported toward how the ArcticTech fork structures it (multipoint +
autowall ragebot, animation-bounded desync, counter-strafe movement), but kept
standalone and testable: no game, no DX9, just the SDK stubs.

- `src/SDK/` — math + engine stand-ins (player/weapon/usercmd/globals) and
  `Hitbox.h` (hitbox skeleton, hit-group damage model, spread-sampled hitchance).
- `src/Config.h` — per-section config structs + combo label tables.
- `src/Features/Ragebot.h` — multipoint scan over hitboxes, `get_damage`/hitchance
  per point, safe-point preference, then target priority + fire gating.
- `src/Features/AntiAim.h` — real/fake angle split: pitch, base direction,
  freestanding (face away from the threat), jitter, movement-bounded desync delta.
- `src/Features/Movement.h` — bunny hop, air-strafe optimizer, counter-strafe fast-stop.
- `src/Features/Esp.h` — per-enemy ESP entries (data for the HUD).
- `src/Features/FeatureManager.h` — the create-move pipeline tying it together
  (ragebot snaps the command on fire; anti-aim runs otherwise).
- `src/demo.cpp` — self-test exercising every section (18 checks).

```sh
g++ -std=c++17 src/demo.cpp -o /tmp/modtest && /tmp/modtest
```

### UI/HUD preview (`Harness/`)
Software-rendered preview of the menu + HUD (see `Harness/README.md`). It binds
the menu rows and HUD windows directly to `src/Config.h`, so the preview shows
the same state the features read.

### Standalone interactive app (`App/`, `ImguiApp/`)
Windowed builds with the menu, HUD, and the real `create_move` pipeline compiled
in, running against a **simulated** HvH scene — no game, no injection. `INSERT`
toggles the (centered) menu, left-click drives it, and the feature logic reacts
to what you change.

- `App/` — software-rendered, via SDL2 (cross-platform) or a dependency-free
  Win32/GDI single-file `.exe`. See `App/README.md`.
- `ImguiApp/` — the same UI rendered 1:1 through **Dear ImGui** on Win32+DX9
  (GPU). Same `Hud`/`Menu`/`Scene` code; `Render2D`'s backend is swapped with
  `-DR2D_IMGUI`. See `ImguiApp/README.md`.

The interaction logic is verified headless:

```sh
g++ -std=c++17 -O2 App/headless_test.cpp Harness/Render2D.cpp Harness/Hud.cpp \
    Harness/Menu.cpp $(pkg-config --cflags --libs freetype2) -o /tmp/apptest && /tmp/apptest
```
