#pragma once

// Shared config — the single source of truth. Both the feature modules and the
// menu read these structs, so a menu row and the code it drives can never drift.
// In the real mod this is what gets serialized to/from the .cfg files.
//
// Option set + grouping mirror the ArcticTech fork's UI (Features/RageBot,
// AntiAim, Visuals, Misc) 1:1 for the core HvH features. Purely cosmetic /
// game-only options (skins, agent/knife/mask models, per-chams colors, grenade
// ESP, fog/skybox) are intentionally omitted — they need the engine and have no
// standalone behavior here.
namespace cfg {

// combo-box option labels (index = stored enum value)
inline const char* kTarget[]     = {"nearest", "lowest hp", "crosshair", "high damage"};
inline const char* kPitch[]      = {"down", "up", "zero", "jitter"};
inline const char* kYawBase[]    = {"forward", "backward", "at target", "spin"};
inline const char* kYawJitter[]  = {"off", "center", "offset", "random"};
inline const char* kInverter[]   = {"static", "switch"};
inline const char* kHideShots[]  = {"off", "on key", "always"};
inline const char* kBoxes[]      = {"cornered", "full", "filled"};
inline const char* kChams[]      = {"flat", "textured", "glow", "off"};
inline const char* kFps[]        = {"off", "low", "med", "high"};
inline const char* kRemoveScope[]= {"off", "line", "disabled"};
inline const char* kLegMovement[]= {"off", "slide", "static"};

struct Ragebot {
    bool enabled = true;
    // aimbot
    bool auto_fire = true;
    bool auto_stop = true;
    bool auto_scope = true;
    bool dormant_aim = false;
    bool extended_backtrack = true;
    // weapon (universal)
    int  min_damage = 35;      // 0..100
    int  hitchance = 72;       // 0..100
    int  target = 0;           // kTarget
    bool aim_head_safe = true; // prefer head only when it's a safe point
    bool force_baim = false;   // body aim only
    // exploits
    bool double_tap = true;
    bool hide_shots = true;
    int  shift_amount = 14;    // ticks, 0..16
    // overrides
    bool delay_shot = false;
    bool accuracy_boost = true;
    bool peek_assist = false;
    bool show_points = false;
};

struct AntiAim {
    bool enabled = true;
    // angles
    int  pitch = 0;            // kPitch
    int  yaw_base = 1;         // kYawBase (backward)
    int  yaw_jitter = 2;       // kYawJitter (offset)
    // desync
    int  desync = 62;          // 0..100
    int  inverter = 1;         // kInverter (switch)
    int  body_lean = 40;       // 0..100 (body yaw limit)
    bool legacy_desync = false;
    // fake lag
    bool fake_lag = true;
    int  fl_limit = 8;         // ticks 0..16
    int  fl_variance = 30;     // %
    // other
    bool fake_duck = true;
    bool defensive = false;
    int  hide_shots = 1;       // kHideShots
    bool slow_walk = false;
    bool jitter_move = false;
    int  leg_movement = 1;     // kLegMovement
    // manual
    bool manual_left = false;
    bool manual_right = false;
    bool manual_back = false;
    bool freestand = false;
};

struct Visuals {
    // players
    bool players = true;
    int  boxes = 0;            // kBoxes
    bool skeleton = true;
    bool health = true;
    bool name = true;
    bool weapon = true;
    bool flags = true;
    // chams
    int  chams = 0;            // kChams
    bool chams_behind = true;
    // world
    int  remove_scope = 1;     // kRemoveScope
    int  fov = 90;             // 70..120
    bool force_thirdperson = false;
    bool night_mode = false;
    // interface
    bool watermark = true;
    bool keybinds = true;
    bool spectators = true;
    bool hit_log = true;
    bool hit_marker = true;
    bool damage_marker = true;
};

struct Misc {
    // movement
    bool bhop = true;          // auto jump
    bool auto_strafe = true;
    int  auto_strafe_smooth = 35; // %
    bool fast_stop = true;     // quick stop
    bool edge_jump = false;
    bool infinity_duck = false;
    // automation
    bool auto_accept = true;
    bool auto_pistol = true;
    bool anti_aimblock = true;
    // other
    bool anti_untrusted = true;
    bool clantag = false;
    bool ad_block = true;
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
