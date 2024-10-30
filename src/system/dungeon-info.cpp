#include "system/dungeon-info.h"
#include "dungeon/dungeon-flag-mask.h"
#include "grid/feature-action-flags.h"
#include "grid/feature.h"
#include "system/enums/monrace/monrace-id.h"
#include "system/monster-race-info.h"
#include "system/terrain-type-definition.h"

enum conversion_type {
    CONVERT_TYPE_FLOOR = 0,
    CONVERT_TYPE_WALL = 1,
    CONVERT_TYPE_INNER = 2,
    CONVERT_TYPE_OUTER = 3,
    CONVERT_TYPE_SOLID = 4,
    CONVERT_TYPE_STREAM1 = 5,
    CONVERT_TYPE_STREAM2 = 6,
};

/*
 * The dungeon arrays
 */
std::vector<dungeon_type> dungeons_info;

/*
 * Maximum number of dungeon in DungeonDefinitions.txt
 */
std::vector<DEPTH> max_dlv;

bool dungeon_type::has_river_flag() const
{
    return this->flags.has_any_of(DF_RIVER_MASK);
}

/*!
 * @brief ダンジョンが地下ダンジョンかを判定する
 * @return 地下ダンジョンならtrue、地上 (荒野)ならfalse
 */
bool dungeon_type::is_dungeon() const
{
    return this->idx > 0;
}

bool dungeon_type::has_guardian() const
{
    return this->final_guardian != MonraceId::PLAYER;
}

MonsterRaceInfo &dungeon_type::get_guardian()
{
    return MonraceList::get_instance().get_monrace(this->final_guardian);
}

const MonsterRaceInfo &dungeon_type::get_guardian() const
{
    return MonraceList::get_instance().get_monrace(this->final_guardian);
}

/*
 * @brief 地形とそれへのアクションから新しい地形IDを返す
 * @param terrain_id 元の地形ID
 * @param action 地形特性
 * @return 新しい地形ID
 */
short dungeon_type::convert_terrain_id(short terrain_id, TerrainCharacteristics action) const
{
    const auto &terrain = TerrainList::get_instance().get_terrain(terrain_id);
    for (auto i = 0; i < MAX_FEAT_STATES; i++) {
        if (terrain.state[i].action == action) {
            return this->convert_terrain_id(terrain.state[i].result);
        }
    }

    if (terrain.flags.has(TerrainCharacteristics::PERMANENT)) {
        return terrain_id;
    }

    const auto has_action_flag = any_bits(terrain_action_flags[enum2i(action)], FAF_DESTROY);
    return has_action_flag ? this->convert_terrain_id(terrain.destroyed) : terrain_id;
}

short dungeon_type::convert_terrain_id(short terrain_id) const
{
    const auto &terrain = TerrainList::get_instance().get_terrain(terrain_id);
    if (terrain.flags.has_not(TerrainCharacteristics::CONVERT)) {
        return terrain_id;
    }

    switch (terrain.subtype) {
    case CONVERT_TYPE_FLOOR:
        return rand_choice(feat_ground_type);
    case CONVERT_TYPE_WALL:
        return rand_choice(feat_wall_type);
    case CONVERT_TYPE_INNER:
        return feat_wall_inner;
    case CONVERT_TYPE_OUTER:
        return feat_wall_outer;
    case CONVERT_TYPE_SOLID:
        return feat_wall_solid;
    case CONVERT_TYPE_STREAM1:
        return this->stream1;
    case CONVERT_TYPE_STREAM2:
        return this->stream2;
    default:
        return terrain_id;
    }
}
