// Standalone interactive build: a window with the simulated HvH scene + the
// real feature pipeline + the interactive menu compiled straight in. No game and
// no injection — everything runs inside this executable.
//
// Controls:  INSERT toggles the menu,  left-click drives it,  ESC quits.
//
// Build (MSYS2/MinGW on Windows, or Linux/macOS) — SDL2 + FreeType:
//   g++ -std=c++17 -O2 App/main_sdl.cpp Harness/Render2D.cpp Harness/Hud.cpp \
//       Harness/Menu.cpp $(pkg-config --cflags --libs sdl2 freetype2) -o jazzhook_app
//
// The windowing layer is deliberately thin; all UI/sim lives in App/Scene.h and
// is exercised headless by App/headless_test.cpp.

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "Scene.h"

int main() {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    const int W = 1280, H = 720;
    SDL_Window* win = SDL_CreateWindow("jazzhook — standalone (no game)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING, W, H);
    if (!win || !ren || !tex) {
        SDL_Log("SDL setup failed: %s", SDL_GetError());
        return 1;
    }

    Render2D r(W, H);
    AppState st;
    app_init(st);
    st.splash.start();   // play the on-inject splash on launch

    bool running = true;
    bool prevDown = false;
    Uint64 prev = SDL_GetPerformanceCounter();

    while (running) {
        bool toggleMenu = false;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) running = false;
                else if (e.key.keysym.sym == SDLK_INSERT) toggleMenu = true;
            }
        }

        int mx, my;
        Uint32 buttons = SDL_GetMouseState(&mx, &my);
        bool down = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

        Menu::Input in;
        in.mx = (float)mx; in.my = (float)my;
        in.down = down;
        in.clicked = down && !prevDown; // left-button press edge
        prevDown = down;

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - prev) / (float)SDL_GetPerformanceFrequency();
        prev = now;
        if (dt > 0.1f) dt = 0.1f; // clamp huge stalls

        app_frame(r, st, in, toggleMenu, dt);

        SDL_UpdateTexture(tex, nullptr, r.data(), W * 4);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
