#pragma once

// Shared config — the single source of truth. Both the feature modules and the
// menu read these structs, so a menu row and the code it drives can never drift.
// In the real mod this is what gets serialized to/from the .cfg files.
namespace cfg {

// combo-box option labels (index = stored enum value)
inline const char* kTarget[]    = {"nearest", "lowest hp", "crosshair", "high damage"};
inline const char* kPitch[]     = {"down", "up", "zero"};
inline const char* kYawBase[]   = {"forward", "backward", "left", "right"};
inline const char* kYawJitter[] = {"off", "center", "offset", "random"};
inline const char* kInverter[]  = {"static", "switch"};
inline const char* kHideShots[] = {"off", "on key", "always"};
inline const char* kBoxes[]     = {"cornered", "full", "filled"};
inline const char* kChams[]     = {"flat", "textured", "glow", "off"};
inline const char* kFps[]       = {"off", "low", "med", "high"};

struct Ragebot {
    bool enabled = true;
    int  min_damage = 35;      // 0..100
    int  target = 0;           // kTarget
    bool auto_fire = true;
    bool auto_stop = true;
    bool auto_scope = true;
    int  hitchance = 72;       // 0..100
    bool double_tap = true;
    bool hide_shots = true;
    int  shift_amount = 14;    // ticks, 0..16
};

struct AntiAim {
    bool enabled = true;
    int  pitch = 0;            // kPitch
    int  yaw_base = 1;         // kYawBase (backward)
    int  yaw_jitter = 2;       // kYawJitter (offset)
    int  desync = 62;          // 0..100
    int  inverter = 1;         // kInverter (switch)
    int  body_lean = 40;       // 0..100
    bool fake_duck = true;
    bool defensive = false;
    int  hide_shots = 1;       // kHideShots
    bool manual_left = false;
    bool manual_right = false;
    bool manual_back = false;
    bool freestand = false;
};

struct Visuals {
    bool players = true;
    int  boxes = 0;            // kBoxes
    bool skeleton = true;
    bool health = true;
    int  chams = 0;            // kChams
    bool hit_marker = true;
    bool remove_scope = true;
    bool watermark = true;
    bool keybinds = true;
    bool spectators = true;
    bool hit_log = true;
};

struct Misc {
    bool bhop = true;
    bool auto_strafe = true;
    bool fast_stop = true;
    bool auto_accept = true;
    bool auto_pistol = true;
    bool anti_aimblock = true;
    bool fake_lag = true;
    int  fps_boost = 2;        // kFps
    bool third_person = false;
    bool free_cam = false;
};

struct Config {
    Ragebot rage;
    AntiAim aa;
    Visuals visuals;
    Misc    misc;
};

inline Config g_cfg;

} // namespace cfg
