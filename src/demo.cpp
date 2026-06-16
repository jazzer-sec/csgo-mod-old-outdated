// Feature self-test: drives each section with mocked state and checks behavior.
//   g++ -std=c++17 -O2 src/demo.cpp -o /tmp/modtest && /tmp/modtest
#include "Features/FeatureManager.h"
#include <cstdio>
#include <cmath>

using namespace sdk;
using namespace feat;

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { std::printf("  FAIL  %s\n", msg); ++fails; } \
                           else std::printf("  ok    %s\n", msg); } while (0)

int main() {
    g_globals.curtime = 10.f;

    Weapon ak; ak.def_index = WEAP_AK; ak.base_damage = 36; ak.next_primary_attack = 0.f;
    Player local; local.index = 0; local.enemy = false; local.team = 2;
    local.eye = {0, 0, 64}; local.origin = {0, 0, 0}; local.weapon = &ak; local.name = "me";

    Player e1; e1.index = 1; e1.name = "trash_kid"; e1.eye = {300, 0, 64};  e1.origin = {300, 0, 0};
    e1.health = 100; e1.armor = 100;
    Player e2; e2.index = 2; e2.name = "noscoper";  e2.eye = {1200, 200, 64}; e2.origin = {1200, 200, 0};
    e2.health = 40; e2.armor = 0; e2.scoped = true;
    std::vector<Player> players = {e1, e2};

    std::printf("[rage]\n");
    cfg::g_cfg.rage.target = 0;
    AimResult aim = ragebot_run(local, players);
    CHECK(aim.has_target && aim.target == 1, "picks nearest enemy");
    cfg::g_cfg.rage.target = 1;
    aim = ragebot_run(local, players);
    CHECK(aim.target == 2, "picks lowest-hp enemy");
    cfg::g_cfg.rage.target = 0; cfg::g_cfg.rage.min_damage = 200;
    aim = ragebot_run(local, players);
    CHECK(!aim.fire && aim.reason == "min damage", "respects minimum damage");
    cfg::g_cfg.rage.min_damage = 1;
    aim = ragebot_run(local, players);
    CHECK(aim.fire, "fires when damage/hitchance/ready");

    std::printf("[anti-aim]\n");
    cfg::g_cfg.aa.enabled = true; cfg::g_cfg.aa.yaw_base = 1; cfg::g_cfg.aa.yaw_jitter = 0;
    cfg::g_cfg.aa.manual_left = cfg::g_cfg.aa.manual_right = cfg::g_cfg.aa.manual_back = false;
    UserCmd cmd;
    AAResult aa = anti_aim_apply(&cmd, 0.f, 10);
    CHECK(std::fabs(normalize_yaw(aa.yaw - 180.f)) < 1.f, "yaw points backward");
    CHECK(aa.pitch > 80.f, "pitch aims down");
    CHECK((cmd.buttons & IN_DUCK) != 0, "fake-duck sets duck bit");
    cfg::g_cfg.aa.manual_left = true;
    UserCmd cmd2;
    aa = anti_aim_apply(&cmd2, 0.f, 11);
    CHECK(std::fabs(normalize_yaw(aa.yaw - 90.f)) < 1.f, "manual-left overrides to left");
    cfg::g_cfg.aa.manual_left = false;

    std::printf("[movement]\n");
    cfg::g_cfg.misc.bhop = true;
    Player air = local; air.on_ground = false;
    UserCmd jc; jc.buttons = IN_JUMP;
    movement_run(&jc, air, 0.f);
    CHECK((jc.buttons & IN_JUMP) == 0, "bhop releases jump while airborne");
    UserCmd gc;
    movement_run(&gc, local, 0.f);
    CHECK((gc.buttons & IN_JUMP) != 0, "bhop jumps on the ground");

    std::printf("[visuals]\n");
    cfg::g_cfg.visuals.players = true;
    auto esp = esp_build(local, players);
    CHECK(esp.size() == 2, "builds one entry per enemy");
    CHECK(!esp[1].flags.empty(), "flags low/scoped detected");

    std::printf("[pipeline]\n");
    cfg::g_cfg.rage.min_damage = 1;
    TickInput in; in.local = local; in.players = players; in.view_yaw = 0.f; in.tick = 12;
    UserCmd pc;
    TickOutput o = create_move(&pc, in);
    CHECK((pc.buttons & IN_ATTACK) != 0 && o.aim.fire, "create_move fires through the pipeline");

    std::printf(fails ? "\n%d check(s) FAILED\n" : "\nall checks passed\n", fails);
    return fails ? 1 : 0;
}
