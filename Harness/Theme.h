#pragma once
#include "Render2D.h"

// Theme — premium legacy-HvH palette. Warm charcoal, cream text, a single
// restrained TEAL accent (not the bright gamesense yellow, not modern violet).
// Quiet, expensive-feeling; accent used sparingly.
struct Theme {
    Color accent     {86,  190, 170};   // muted teal
    Color accent2    {150, 220, 205};   // light teal (highlight)
    Color accentDim  {60,  120, 110};   // teal, recessed

    Color bg         {11,  11,  13};
    Color panel      {24,  25,  29, 252};
    Color sideTop    {26,  27,  31};
    Color sideBot    {18,  19,  22};
    Color content    {28,  29,  33};
    Color container  {33,  34,  39};
    Color container2 {30,  31,  36};     // gradient partner (lower)
    Color field      {20,  21,  24};

    Color text       {223, 221, 213};    // warm cream
    Color textDim    {132, 132, 136};
    Color textFaint  {84,  85,  92};

    Color line       {40,  41,  47};
    Color lineSoft   {34,  35,  40};
    Color good       {130, 200, 130};
    Color bad        {214, 110, 96};     // muted terracotta (miss)
    Color shadow     {0,   0,   0, 180};
};

inline const Theme& theme() {
    static Theme t;
    return t;
}
