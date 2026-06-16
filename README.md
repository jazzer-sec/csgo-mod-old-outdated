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

- `src/SDK/` — math + engine stand-ins (player/weapon/usercmd/globals).
- `src/Config.h` — per-section config structs + combo label tables.
- `src/Features/Ragebot.h` — target selection + damage/hitchance gating.
- `src/Features/AntiAim.h` — fake/real angles, manual, jitter, desync, fake-duck.
- `src/Features/Movement.h` — bunny hop, auto-strafe, fast-stop.
- `src/Features/Esp.h` — per-enemy ESP entries (data for the HUD).
- `src/Features/FeatureManager.h` — the create-move pipeline tying it together.
- `src/demo.cpp` — self-test exercising every section.

```sh
g++ -std=c++17 src/demo.cpp -o /tmp/modtest && /tmp/modtest
```

### UI/HUD preview (`Harness/`)
Software-rendered preview of the menu + HUD (see `Harness/README.md`). It binds
the menu rows and HUD windows directly to `src/Config.h`, so the preview shows
the same state the features read.
