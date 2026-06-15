#pragma once
#include "Render2D.h"

// Theme — one accent color drives the palette. Default is my call (the user
// granted creative freedom): a violet accent on a near-black panel stack, which
// reads well over the bright in-game backdrop. All of this would be exposed in
// the Visuals/Theme tab in the real build.
struct Theme {
    Color accent     {139, 124, 255};   // violet
    Color accent2    {92,  225, 230};   // cyan (gradient partner)
    Color bg         {13,  13,  18};
    Color panel      {21,  21,  28, 245};
    Color panel2     {28,  28,  38};
    Color groupbox   {18,  18,  25};
    Color text       {228, 230, 238};
    Color textDim    {120, 124, 142};
    Color outline    {44,  44,  58};
    Color good       {86,  214, 122};
    Color bad        {233, 84,  92};
    Color warn       {240, 191, 78};
    Color shadow     {0,   0,   0, 160};
};

inline const Theme& theme() {
    static Theme t;
    return t;
}
