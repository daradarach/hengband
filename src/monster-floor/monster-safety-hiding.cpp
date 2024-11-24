/*!
 * @brief モンスターの逃走・隠匿に関する処理
 * @date 2020/03/08
 * @author Hourier
 */

#include "monster-floor/monster-safety-hiding.h"
#include "floor/cave.h"
#include "floor/geometry.h"
#include "grid/grid.h"
#include "monster-floor/monster-dist-offsets.h"
#include "monster/monster-flag-types.h"
#include "monster/monster-info.h"
#include "monster/monster-processor-util.h"
#include "mspell/mspell-checker.h"
#include "system/floor-type-definition.h"
#include "system/grid-type-definition.h"
#include "system/monster-entity.h"
#include "system/monster-race-info.h"
#include "system/player-type-definition.h"
#include "target/projection-path-calculator.h"

/*!
 * @brief モンスターが逃げ込める地点を走査する
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param m_idx モンスターID
 * @param y_offsets
 * @param x_offsets
 * @param d モンスターがいる地点からの距離
 * @return 逃げ込める地点の候補地
 */
static coordinate_candidate sweep_safe_coordinate(PlayerType *player_ptr, MONSTER_IDX m_idx, const POSITION *y_offsets, const POSITION *x_offsets, int d)
{
    coordinate_candidate candidate;
    const auto &floor = *player_ptr->current_floor_ptr;
    const auto &monster = player_ptr->current_floor_ptr->m_list[m_idx];
    const auto p_pos = player_ptr->get_position();
    const auto m_pos = monster.get_position();
    for (auto i = 0, dx = x_offsets[0], dy = y_offsets[0]; dx != 0 || dy != 0; i++, dx = x_offsets[i], dy = y_offsets[i]) {
        const Pos2DVec vec(dy, dx);
        const auto pos = m_pos + vec;
        if (!in_bounds(&floor, pos.y, pos.x)) {
            continue;
        }

        const auto &monrace = monster.get_monrace();
        const auto &grid = floor.get_grid(pos);
        BIT_FLAGS16 riding_mode = monster.is_riding() ? CEM_RIDING : 0;
        if (!monster_can_cross_terrain(player_ptr, grid.feat, &monrace, riding_mode)) {
            continue;
        }

        if (monster.mflag2.has_not(MonsterConstantFlagType::NOFLOW)) {
            const auto dist = grid.get_distance(monrace.get_grid_flow_type());
            if (dist == 0) {
                continue;
            }
            if (dist > floor.get_grid(m_pos).get_distance(monrace.get_grid_flow_type()) + 2 * d) {
                continue;
            }
        }

        if (projectable(player_ptr, p_pos, pos)) {
            continue;
        }

        POSITION dis = distance(pos.y, pos.x, p_pos.y, p_pos.x);
        if (dis <= candidate.gdis) {
            continue;
        }

        candidate.gy = pos.y;
        candidate.gx = pos.x;
        candidate.gdis = dis;
    }

    return candidate;
}

/*!
 * @brief モンスターが逃げ込める安全な地点を返す /
 * Choose a "safe" location near a monster for it to run toward.
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * A location is "safe" if it can be reached quickly and the player\n
 * is not able to fire into it (it isn't a "clean shot").  So, this will\n
 * cause monsters to "duck" behind walls.  Hopefully, monsters will also\n
 * try to run towards corridor openings if they are in a room.\n
 *\n
 * This function may take lots of CPU time if lots of monsters are\n
 * fleeing.\n
 *\n
 * Return TRUE if a safe location is available.\n
 */
bool find_safety(PlayerType *player_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
    auto *m_ptr = &player_ptr->current_floor_ptr->m_list[m_idx];
    for (POSITION d = 1; d < 10; d++) {
        const POSITION *y_offsets;
        y_offsets = dist_offsets_y[d];

        const POSITION *x_offsets;
        x_offsets = dist_offsets_x[d];

        coordinate_candidate candidate = sweep_safe_coordinate(player_ptr, m_idx, y_offsets, x_offsets, d);

        if (candidate.gdis <= 0) {
            continue;
        }

        *yp = m_ptr->fy - candidate.gy;
        *xp = m_ptr->fx - candidate.gx;

        return true;
    }

    return false;
}

/*!
 * @brief モンスターが隠れられる地点を走査する
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param m_idx モンスターID
 * @param y_offsets
 * @param x_offsets
 * @param candidate 隠れられる地点の候補地
 */
static void sweep_hiding_candidate(
    PlayerType *player_ptr, MonsterEntity *m_ptr, const POSITION *y_offsets, const POSITION *x_offsets, coordinate_candidate *candidate)
{
    const auto &monrace = m_ptr->get_monrace();
    const auto p_pos = player_ptr->get_position();
    const auto m_pos = m_ptr->get_position();
    for (POSITION i = 0, dx = x_offsets[0], dy = y_offsets[0]; dx != 0 || dy != 0; i++, dx = x_offsets[i], dy = y_offsets[i]) {
        const Pos2DVec vec(dy, dx);
        const auto pos = m_pos + vec;
        if (!in_bounds(player_ptr->current_floor_ptr, pos.y, pos.x)) {
            continue;
        }
        if (!monster_can_enter(player_ptr, pos.y, pos.x, &monrace, 0)) {
            continue;
        }
        if (projectable(player_ptr, p_pos, pos) || !clean_shot(player_ptr, m_ptr->fy, m_ptr->fx, pos.y, pos.x, false)) {
            continue;
        }

        POSITION dis = distance(pos.y, pos.x, player_ptr->y, player_ptr->x);
        if (dis < candidate->gdis && dis >= 2) {
            candidate->gy = pos.y;
            candidate->gx = pos.x;
            candidate->gdis = dis;
        }
    }
}

/*!
 * @brief モンスターが隠れ潜める地点を返す /
 * Choose a good hiding place near a monster for it to run toward.
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param m_idx モンスターの参照ID
 * @param yp 移動先のマスのY座標を返す参照ポインタ
 * @param xp 移動先のマスのX座標を返す参照ポインタ
 * @return 有効なマスがあった場合TRUEを返す
 * @details
 * Pack monsters will use this to "ambush" the player and lure him out\n
 * of corridors into open space so they can swarm him.\n
 *\n
 * Return TRUE if a good location is available.\n
 */
bool find_hiding(PlayerType *player_ptr, MONSTER_IDX m_idx, POSITION *yp, POSITION *xp)
{
    auto *m_ptr = &player_ptr->current_floor_ptr->m_list[m_idx];
    coordinate_candidate candidate;
    candidate.gdis = 999;

    for (POSITION d = 1; d < 10; d++) {
        const POSITION *y_offsets;
        y_offsets = dist_offsets_y[d];

        const POSITION *x_offsets;
        x_offsets = dist_offsets_x[d];

        sweep_hiding_candidate(player_ptr, m_ptr, y_offsets, x_offsets, &candidate);
        if (candidate.gdis >= 999) {
            continue;
        }

        *yp = m_ptr->fy - candidate.gy;
        *xp = m_ptr->fx - candidate.gx;
        return true;
    }

    return false;
}
