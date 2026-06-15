#pragma once
#include "Render2D.h"

// Theme — legacy HvH palette: near-black warm-tinted panels, sharp 1px frames,
// a single punchy GOLD accent (gamesense/aimware lineage, distinct from the old
// cold "modern" violet). One accent drives the whole UI; exposed for retuning.
struct Theme {
    Color accent     {245, 208, 54};    // gamesense yellow
    Color accent2    {255, 226, 120};   // light yellow (highlight)
    Color bg         {11,  11,  13};
    Color panel      {22,  22,  25, 250};
    Color panel2     {32,  32,  36};
    Color rail       {17,  17,  19};
    Color section    {26,  26,  29};
    Color text       {214, 216, 220};
    Color textDim    {122, 124, 132};
    Color textFaint  {84,  86,  94};
    Color outline    {40,  40,  46};
    Color outlineLt  {56,  56,  64};
    Color good       {126, 204, 116};
    Color bad        {221, 84,  80};
    Color warn       {245, 208, 54};
    Color shadow     {0,   0,   0, 175};
};

inline const Theme& theme() {
    static Theme t;
    return t;
}
