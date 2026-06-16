# Dear ImGui build (`ImguiApp/`)

The same standalone app as `App/`, but rendered through **Dear ImGui** on a
**Win32 + DirectX 9** backend (GPU-accelerated). It is a 1:1 reproduction of the
software UI: the identical `Hud.cpp` / `Menu.cpp` / `Scene.h` code runs unchanged,
because `Render2D`'s backend is swapped at compile time (`-DR2D_IMGUI`) to emit
into an ImGui `ImDrawList` instead of the CPU framebuffer.

No game, no injection — the menu, HUD, and `create_move` pipeline run against a
simulated scene.

- `main_dx9.cpp` — Win32 window + D3D9 device + ImGui frame loop; loads system
  Verdana at 11/16/22 px (Cyrillic + arrow glyphs) and drives the shared scene.
- `Harness/Render2DImgui.{h,cpp}` — the ImGui backend of the `Render2D` API.
- `third_party/imgui/` — vendored Dear ImGui (core + win32/dx9 backends).

## Controls
`INSERT` toggle menu · left-click drive it · `ESC` quit. The menu is centered
on screen.

## Build (MinGW — single self-contained .exe)

```sh
x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows -DR2D_IMGUI \
  -Ithird_party/imgui -Ithird_party/imgui/backends -Isrc \
  ImguiApp/main_dx9.cpp Harness/Render2DImgui.cpp Harness/Hud.cpp Harness/Menu.cpp \
  third_party/imgui/imgui.cpp third_party/imgui/imgui_draw.cpp \
  third_party/imgui/imgui_tables.cpp third_party/imgui/imgui_widgets.cpp \
  third_party/imgui/backends/imgui_impl_win32.cpp \
  third_party/imgui/backends/imgui_impl_dx9.cpp \
  -ld3d9 -lgdi32 -ldwmapi -limm32 -static -static-libgcc -static-libstdc++ \
  -o jazzhook_imgui.exe
```

(On a native MSYS2 MINGW64 shell drop the `x86_64-w64-mingw32-` prefix.)
The result depends only on standard Windows DLLs (d3d9, dwmapi, user32, gdi32, …).
