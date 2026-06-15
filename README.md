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
