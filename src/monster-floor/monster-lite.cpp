#include "monster-floor/monster-lite.h"
#include "dungeon/dungeon-flag-types.h"
#include "floor/cave.h"
#include "grid/grid.h"
#include "monster-floor/monster-lite-util.h"
#include "monster-race/race-brightness-flags.h"
#include "monster/monster-status.h"
#include "player-base/player-class.h"
#include "player-info/ninja-data-type.h"
#include "player/special-defense-types.h"
#include "system/angband-system.h"
#include "system/dungeon/dungeon-definition.h"
#include "system/enums/terrain/terrain-characteristics.h"
#include "system/floor/floor-info.h"
#include "system/grid-type-definition.h"
#include "system/monrace/monrace-definition.h"
#include "system/monster-entity.h"
#include "system/player-type-definition.h"
#include "system/redrawing-flags-updater.h"
#include "util/point-2d.h"
#include "view/display-messages.h"
#include "world/world.h"
#include <vector>

/*!
 * @brief モンスターによる光量状態更新 / Add a square to the changes array
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param points 座標たちを記録する配列
 * @param y Y座標
 * @param x X座標
 */
static void update_monster_lite(
    PlayerType *const player_ptr, std::vector<Pos2D> &points, const POSITION y, const POSITION x, const monster_lite_type *const ml_ptr)
{
    Grid *g_ptr;
    int dpf, d;
    POSITION midpoint;
    g_ptr = &player_ptr->current_floor_ptr->grid_array[y][x];
    if ((g_ptr->info & (CAVE_MNLT | CAVE_VIEW)) != CAVE_VIEW) {
        return;
    }

    if (!g_ptr->has_los_terrain()) {
        if (((y < player_ptr->y) && (y > ml_ptr->mon_fy)) || ((y > player_ptr->y) && (y < ml_ptr->mon_fy))) {
            dpf = player_ptr->y - ml_ptr->mon_fy;
            d = y - ml_ptr->mon_fy;
            midpoint = ml_ptr->mon_fx + ((player_ptr->x - ml_ptr->mon_fx) * std::abs(d)) / std::abs(dpf);
            if (x < midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y, x + 1)) {
                    return;
                }
            } else if (x > midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y, x - 1)) {
                    return;
                }
            } else if (ml_ptr->mon_invis) {
                return;
            }
        }

        if (((x < player_ptr->x) && (x > ml_ptr->mon_fx)) || ((x > player_ptr->x) && (x < ml_ptr->mon_fx))) {
            dpf = player_ptr->x - ml_ptr->mon_fx;
            d = x - ml_ptr->mon_fx;
            midpoint = ml_ptr->mon_fy + ((player_ptr->y - ml_ptr->mon_fy) * std::abs(d)) / std::abs(dpf);
            if (y < midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y + 1, x)) {
                    return;
                }
            } else if (y > midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y - 1, x)) {
                    return;
                }
            } else if (ml_ptr->mon_invis) {
                return;
            }
        }
    }

    if (!(g_ptr->info & CAVE_MNDK)) {
        points.emplace_back(y, x);
    } else {
        g_ptr->info &= ~(CAVE_MNDK);
    }

    g_ptr->info |= CAVE_MNLT;
}

/*
 * Add a square to the changes array
 */
static void update_monster_dark(
    PlayerType *const player_ptr, std::vector<Pos2D> &points, const POSITION y, const POSITION x, const monster_lite_type *const ml_ptr)
{
    Grid *g_ptr;
    int midpoint, dpf, d;
    g_ptr = &player_ptr->current_floor_ptr->grid_array[y][x];
    if ((g_ptr->info & (CAVE_LITE | CAVE_MNLT | CAVE_MNDK | CAVE_VIEW)) != CAVE_VIEW) {
        return;
    }

    if (!g_ptr->has_los_terrain() && !g_ptr->cave_has_flag(TerrainCharacteristics::PROJECT)) {
        const auto &floor = *player_ptr->current_floor_ptr;
        if (((y < player_ptr->y) && (y > ml_ptr->mon_fy)) || ((y > player_ptr->y) && (y < ml_ptr->mon_fy))) {
            dpf = player_ptr->y - ml_ptr->mon_fy;
            d = y - ml_ptr->mon_fy;
            midpoint = ml_ptr->mon_fx + ((player_ptr->x - ml_ptr->mon_fx) * std::abs(d)) / std::abs(dpf);
            if (x < midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y, x + 1) && !floor.has_terrain_characteristics({ y, x + 1 }, TerrainCharacteristics::PROJECT)) {
                    return;
                }
            } else if (x > midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y, x - 1) && !floor.has_terrain_characteristics({ y, x - 1 }, TerrainCharacteristics::PROJECT)) {
                    return;
                }
            } else if (ml_ptr->mon_invis) {
                return;
            }
        }

        if (((x < player_ptr->x) && (x > ml_ptr->mon_fx)) || ((x > player_ptr->x) && (x < ml_ptr->mon_fx))) {
            dpf = player_ptr->x - ml_ptr->mon_fx;
            d = x - ml_ptr->mon_fx;
            midpoint = ml_ptr->mon_fy + ((player_ptr->y - ml_ptr->mon_fy) * std::abs(d)) / std::abs(dpf);
            if (y < midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y + 1, x) && !floor.has_terrain_characteristics({ y + 1, x }, TerrainCharacteristics::PROJECT)) {
                    return;
                }
            } else if (y > midpoint) {
                if (!cave_los_bold(player_ptr->current_floor_ptr, y - 1, x) && !floor.has_terrain_characteristics({ y - 1, x }, TerrainCharacteristics::PROJECT)) {
                    return;
                }
            } else if (ml_ptr->mon_invis) {
                return;
            }
        }
    }

    points.emplace_back(y, x);
    g_ptr->info |= CAVE_MNDK;
}

/*!
 * @brief Update squares illuminated or darkened by monsters.
 * The CAVE_TEMP and CAVE_XTRA flag are used to store the state during the
 * updating.  Only squares in view of the player, whos state
 * changes are drawn via lite_spot().
 * @todo player-status からのみ呼ばれている。しかしあちらは行数が酷いので要調整
 */
void update_mon_lite(PlayerType *player_ptr)
{
    // 座標たちを記録する配列。
    std::vector<Pos2D> points;

    void (*add_mon_lite)(PlayerType *, std::vector<Pos2D> &, const POSITION, const POSITION, const monster_lite_type *);
    auto &floor = *player_ptr->current_floor_ptr;
    const auto &dungeon = floor.get_dungeon_definition();
    auto dis_lim = (dungeon.flags.has(DungeonFeatureType::DARKNESS) && !player_ptr->see_nocto) ? (MAX_PLAYER_SIGHT / 2 + 1) : (MAX_PLAYER_SIGHT + 3);
    for (int i = 0; i < floor.mon_lite_n; i++) {
        auto &grid = floor.get_grid({ floor.mon_lite_y[i], floor.mon_lite_x[i] });
        grid.add_info((grid.info & CAVE_MNLT) ? CAVE_TEMP : CAVE_XTRA);
        grid.info &= ~(CAVE_MNLT | CAVE_MNDK);
    }

    const auto &world = AngbandWorld::get_instance();
    if (!world.timewalk_m_idx) {
        MonsterEntity *m_ptr;
        MonraceDefinition *r_ptr;
        for (int i = 1; i < floor.m_max; i++) {
            m_ptr = &floor.m_list[i];
            r_ptr = &m_ptr->get_monrace();
            if (!m_ptr->is_valid() || (m_ptr->cdis > dis_lim)) {
                continue;
            }

            int rad = 0;
            if (r_ptr->brightness_flags.has_any_of({ MonsterBrightnessType::HAS_LITE_1, MonsterBrightnessType::SELF_LITE_1 })) {
                rad++;
            }

            if (r_ptr->brightness_flags.has_any_of({ MonsterBrightnessType::HAS_LITE_2, MonsterBrightnessType::SELF_LITE_2 })) {
                rad += 2;
            }

            if (r_ptr->brightness_flags.has_any_of({ MonsterBrightnessType::HAS_DARK_1, MonsterBrightnessType::SELF_DARK_1 })) {
                rad--;
            }

            if (r_ptr->brightness_flags.has_any_of({ MonsterBrightnessType::HAS_DARK_2, MonsterBrightnessType::SELF_DARK_2 })) {
                rad -= 2;
            }

            if (!rad) {
                continue;
            }

            TerrainCharacteristics f_flag;
            if (rad > 0) {
                auto should_lite = r_ptr->brightness_flags.has_none_of({ MonsterBrightnessType::SELF_LITE_1, MonsterBrightnessType::SELF_LITE_2 });
                should_lite &= (m_ptr->is_asleep() || (!floor.is_underground() && world.is_daytime()) || AngbandSystem::get_instance().is_phase_out());
                if (should_lite) {
                    continue;
                }

                if (dungeon.flags.has(DungeonFeatureType::DARKNESS)) {
                    rad = 1;
                }

                add_mon_lite = update_monster_lite;
                f_flag = TerrainCharacteristics::LOS;
            } else {
                if (r_ptr->brightness_flags.has_none_of({ MonsterBrightnessType::SELF_DARK_1, MonsterBrightnessType::SELF_DARK_2 }) && (m_ptr->is_asleep() || (!floor.is_underground() && !world.is_daytime()))) {
                    continue;
                }

                add_mon_lite = update_monster_dark;
                f_flag = TerrainCharacteristics::PROJECT;
                rad = -rad;
            }

            monster_lite_type tmp_ml;
            monster_lite_type *ml_ptr = initialize_monster_lite_type(floor.grid_array[m_ptr->fy][m_ptr->fx].info, &tmp_ml, m_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx + 1, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx - 1, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx + 1, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx - 1, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx + 1, ml_ptr);
            add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx - 1, ml_ptr);
            if (rad < 2) {
                continue;
            }

            Grid *g_ptr;
            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy + 1, ml_ptr->mon_fx }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 2, ml_ptr->mon_fx + 1, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 2, ml_ptr->mon_fx, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 2, ml_ptr->mon_fx - 1, ml_ptr);
                g_ptr = &floor.grid_array[ml_ptr->mon_fy + 2][ml_ptr->mon_fx];
                if ((rad == 3) && g_ptr->cave_has_flag(f_flag)) {
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 3, ml_ptr->mon_fx + 1, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 3, ml_ptr->mon_fx, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 3, ml_ptr->mon_fx - 1, ml_ptr);
                }
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy - 1, ml_ptr->mon_fx }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 2, ml_ptr->mon_fx + 1, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 2, ml_ptr->mon_fx, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 2, ml_ptr->mon_fx - 1, ml_ptr);
                g_ptr = &floor.grid_array[ml_ptr->mon_fy - 2][ml_ptr->mon_fx];
                if ((rad == 3) && g_ptr->cave_has_flag(f_flag)) {
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 3, ml_ptr->mon_fx + 1, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 3, ml_ptr->mon_fx, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 3, ml_ptr->mon_fx - 1, ml_ptr);
                }
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy, ml_ptr->mon_fx + 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx + 2, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx + 2, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx + 2, ml_ptr);
                g_ptr = &floor.grid_array[ml_ptr->mon_fy][ml_ptr->mon_fx + 2];
                if ((rad == 3) && g_ptr->cave_has_flag(f_flag)) {
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx + 3, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx + 3, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx + 3, ml_ptr);
                }
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy, ml_ptr->mon_fx - 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx - 2, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx - 2, ml_ptr);
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx - 2, ml_ptr);
                g_ptr = &floor.grid_array[ml_ptr->mon_fy][ml_ptr->mon_fx - 2];
                if ((rad == 3) && g_ptr->cave_has_flag(f_flag)) {
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 1, ml_ptr->mon_fx - 3, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy, ml_ptr->mon_fx - 3, ml_ptr);
                    add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 1, ml_ptr->mon_fx - 3, ml_ptr);
                }
            }

            if (rad != 3) {
                continue;
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy + 1, ml_ptr->mon_fx + 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 2, ml_ptr->mon_fx + 2, ml_ptr);
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy + 1, ml_ptr->mon_fx - 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy + 2, ml_ptr->mon_fx - 2, ml_ptr);
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy - 1, ml_ptr->mon_fx + 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 2, ml_ptr->mon_fx + 2, ml_ptr);
            }

            if (floor.has_terrain_characteristics({ ml_ptr->mon_fy - 1, ml_ptr->mon_fx - 1 }, f_flag)) {
                add_mon_lite(player_ptr, points, ml_ptr->mon_fy - 2, ml_ptr->mon_fx - 2, ml_ptr);
            }
        }
    }

    const auto end_temp = size(points);
    for (int i = 0; i < floor.mon_lite_n; i++) {
        POSITION fx = floor.mon_lite_x[i];
        POSITION fy = floor.mon_lite_y[i];
        const auto &grid = floor.get_grid({ fy, fx });
        if (grid.info & CAVE_TEMP) {
            if ((grid.info & (CAVE_VIEW | CAVE_MNLT)) == CAVE_VIEW) {
                cave_note_and_redraw_later(&floor, fy, fx);
            }
        } else if ((grid.info & (CAVE_VIEW | CAVE_MNDK)) == CAVE_VIEW) {
            cave_note_and_redraw_later(&floor, fy, fx);
        }

        points.emplace_back(fy, fx);
    }

    floor.mon_lite_n = 0;
    for (size_t i = 0; i < end_temp; i++) {
        const auto &[fy, fx] = points[i];

        const auto &grid = floor.get_grid({ fy, fx });
        if (grid.info & CAVE_MNLT) {
            if ((grid.info & (CAVE_VIEW | CAVE_TEMP)) == CAVE_VIEW) {
                cave_note_and_redraw_later(&floor, fy, fx);
            }
        } else if ((grid.info & (CAVE_VIEW | CAVE_XTRA)) == CAVE_VIEW) {
            cave_note_and_redraw_later(&floor, fy, fx);
        }

        floor.mon_lite_x[floor.mon_lite_n] = fx;
        floor.mon_lite_y[floor.mon_lite_n] = fy;
        floor.mon_lite_n++;
    }

    for (size_t i = end_temp; i < size(points); i++) {
        const auto &[y, x] = points[i];
        floor.get_grid({ y, x }).info &= ~(CAVE_TEMP | CAVE_XTRA);
    }

    RedrawingFlagsUpdater::get_instance().set_flag(StatusRecalculatingFlag::DELAY_VISIBILITY);
    player_ptr->monlite = (floor.grid_array[player_ptr->y][player_ptr->x].info & CAVE_MNLT) != 0;
    auto ninja_data = PlayerClass(player_ptr).get_specific_data<ninja_data_type>();
    if (!ninja_data || !ninja_data->s_stealth) {
        player_ptr->old_monlite = player_ptr->monlite;
        return;
    }

    if (player_ptr->old_monlite == player_ptr->monlite) {
        player_ptr->old_monlite = player_ptr->monlite;
        return;
    }

    if (player_ptr->monlite) {
        msg_print(_("影の覆いが薄れた気がする。", "Your mantle of shadow becomes thin."));
    } else {
        msg_print(_("影の覆いが濃くなった！", "Your mantle of shadow is restored to its original darkness."));
    }

    player_ptr->old_monlite = player_ptr->monlite;
}

/*!
 * @brief 画面切り替え等でモンスターの灯りを消去する
 * @param floor_ptr 現在フロアへの参照ポインタ
 */
void clear_mon_lite(FloorType *floor_ptr)
{
    for (int i = 0; i < floor_ptr->mon_lite_n; i++) {
        Grid *g_ptr;
        g_ptr = &floor_ptr->grid_array[floor_ptr->mon_lite_y[i]][floor_ptr->mon_lite_x[i]];
        g_ptr->info &= ~(CAVE_MNLT | CAVE_MNDK);
    }

    floor_ptr->mon_lite_n = 0;
}
