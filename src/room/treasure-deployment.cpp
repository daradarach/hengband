/*!
 * @brief 部屋にアイテム・モンスター・罠を配置する処理
 * @date 2020/07/24
 * @author Hourier
 */

#include "room/treasure-deployment.h"
#include "floor/cave.h"
#include "floor/geometry.h"
#include "grid/object-placer.h"
#include "grid/trap.h"
#include "monster-floor/monster-generator.h"
#include "monster-floor/place-monster-types.h"
#include "object-enchant/item-apply-magic.h"
#include "system/enums/terrain/terrain-characteristics.h"
#include "system/floor/floor-info.h"
#include "system/grid-type-definition.h"
#include "system/player-type-definition.h"

/*
 * Routine that fills the empty areas of a room with treasure and monsters.
 */
void fill_treasure(PlayerType *player_ptr, POSITION x1, POSITION x2, POSITION y1, POSITION y2, int difficulty)
{
    POSITION cx = (x1 + x2) / 2;
    POSITION cy = (y1 + y2) / 2;
    POSITION size = abs(x2 - x1) + abs(y2 - y1);
    auto *floor_ptr = player_ptr->current_floor_ptr;
    for (POSITION x = x1; x <= x2; x++) {
        for (POSITION y = y1; y <= y2; y++) {
            int32_t value = ((((int32_t)(distance(cx, cy, x, y))) * 100) / size) + randint1(10) - difficulty;
            if ((randint1(100) - difficulty * 3) > 50) {
                value = 20;
            }

            const Pos2D pos(y, x);
            if (!floor_ptr->get_grid(pos).is_floor() && (!floor_ptr->has_terrain_characteristics(pos, TerrainCharacteristics::PLACE) || !floor_ptr->has_terrain_characteristics(pos, TerrainCharacteristics::DROP))) {
                continue;
            }

            if (value < 0) {
                floor_ptr->monster_level = floor_ptr->base_level + 40;
                place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                floor_ptr->monster_level = floor_ptr->base_level;
                floor_ptr->object_level = floor_ptr->base_level + 20;
                place_object(player_ptr, y, x, AM_GOOD);
                floor_ptr->object_level = floor_ptr->base_level;
            } else if (value < 5) {
                floor_ptr->monster_level = floor_ptr->base_level + 20;
                place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                floor_ptr->monster_level = floor_ptr->base_level;
                floor_ptr->object_level = floor_ptr->base_level + 10;
                place_object(player_ptr, y, x, AM_GOOD);
                floor_ptr->object_level = floor_ptr->base_level;
            } else if (value < 10) {
                floor_ptr->monster_level = floor_ptr->base_level + 9;
                place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                floor_ptr->monster_level = floor_ptr->base_level;
            } else if (value < 17) {
            } else if (value < 23) {
                if (one_in_(4)) {
                    place_object(player_ptr, y, x, 0L);
                } else {
                    place_trap(floor_ptr, y, x);
                }
            } else if (value < 30) {
                floor_ptr->monster_level = floor_ptr->base_level + 5;
                place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                floor_ptr->monster_level = floor_ptr->base_level;
                place_trap(floor_ptr, y, x);
            } else if (value < 40) {
                if (one_in_(2)) {
                    floor_ptr->monster_level = floor_ptr->base_level + 3;
                    place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                    floor_ptr->monster_level = floor_ptr->base_level;
                }

                if (one_in_(2)) {
                    floor_ptr->object_level = floor_ptr->base_level + 7;
                    place_object(player_ptr, y, x, 0L);
                    floor_ptr->object_level = floor_ptr->base_level;
                }
            } else if (value < 50) {
                place_trap(floor_ptr, y, x);
            } else {
                if (one_in_(5)) {
                    place_random_monster(player_ptr, y, x, PM_ALLOW_SLEEP | PM_ALLOW_GROUP);
                } else if (one_in_(2)) {
                    place_trap(floor_ptr, y, x);
                } else if (one_in_(2)) {
                    place_object(player_ptr, y, x, 0L);
                }
            }
        }
    }
}
