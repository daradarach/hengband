#include "cmd-action/cmd-tunnel.h"
#include "action/tunnel-execution.h"
#include "cmd-action/cmd-attack.h"
#include "core/disturbance.h"
#include "floor/geometry.h"
#include "grid/grid.h"
#include "io/input-key-requester.h"
#include "player-base/player-class.h"
#include "player-info/samurai-data-type.h"
#include "player-status/player-energy.h"
#include "player/attack-defense-types.h"
#include "player/special-defense-types.h"
#include "status/action-setter.h"
#include "system/floor/floor-info.h"
#include "system/grid-type-definition.h"
#include "system/player-type-definition.h"
#include "system/redrawing-flags-updater.h"
#include "system/terrain/terrain-definition.h"
#include "target/target-getter.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

/*!
 * @brief 「掘る」動作コマンドのメインルーチン /
 * Tunnels through "walls" (including rubble and closed doors)
 * @details
 * <pre>
 * Note that you must tunnel in order to hit invisible monsters
 * in walls, though moving into walls still takes a turn anyway.
 *
 * Digging is very difficult without a "digger" weapon, but can be
 * accomplished by strong players using heavy weapons.
 * </pre>
 */
void do_cmd_tunnel(PlayerType *player_ptr)
{
    PlayerClass(player_ptr).break_samurai_stance({ SamuraiStanceType::MUSOU });

    if (command_arg) {
        command_rep = command_arg - 1;
        RedrawingFlagsUpdater::get_instance().set_flag(MainWindowRedrawingFlag::ACTION);
        command_arg = 0;
    }

    int dir;
    if (!get_rep_dir(player_ptr, &dir)) {
        disturb(player_ptr, false, false);
        return;
    }

    auto more = false;
    const auto pos = player_ptr->get_neighbor(dir);
    const auto &grid = player_ptr->current_floor_ptr->get_grid(pos);
    const auto &terrain_mimic = grid.get_terrain(TerrainKind::MIMIC);
    if (terrain_mimic.flags.has(TerrainCharacteristics::DOOR)) {
        msg_print(_("ドアは掘れない。", "You cannot tunnel through doors."));
    } else if (terrain_mimic.flags.has_not(TerrainCharacteristics::TUNNEL)) {
        msg_print(_("そこは掘れない。", "You can't tunnel through that."));
    } else if (grid.has_monster()) {
        PlayerEnergy(player_ptr).set_player_turn_energy(100);
        msg_print(_("モンスターが立ちふさがっている！", "There is a monster in the way!"));
        do_cmd_attack(player_ptr, pos.y, pos.x, HISSATSU_NONE);
    } else {
        more = exe_tunnel(player_ptr, pos.y, pos.x);
    }

    if (!more) {
        disturb(player_ptr, false, false);
    }
}
