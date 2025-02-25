/*!
 * @brief 地形特性の集合論的処理定義
 * @author Hourier
 * @date 2024/12/08
 */

#pragma once

#include "util/abstract-vector-wrapper.h"
#include <map>
#include <optional>
#include <string_view>
#include <vector>

enum class TerrainTag;
class TerrainType;
class TerrainList : public util::AbstractVectorWrapper<TerrainType> {
public:
    TerrainList(const TerrainList &) = delete;
    TerrainList(TerrainList &&) = delete;
    TerrainList operator=(const TerrainList &) = delete;
    TerrainList operator=(TerrainList &&) = delete;

    static TerrainList &get_instance();
    TerrainType &get_terrain(short terrain_id);
    const TerrainType &get_terrain(short terrain_id) const;
    TerrainType &get_terrain(TerrainTag tag);
    const TerrainType &get_terrain(TerrainTag tag) const;
    short get_terrain_id(TerrainTag tag) const;
    short get_terrain_id_by_tag(std::string_view tag) const;

    void retouch();
    void emplace_tag(std::string_view tag);

private:
    TerrainList() = default;

    static TerrainList instance;
    std::vector<TerrainType> terrains;
    std::map<TerrainTag, short> tags; //!< @details 全てのTerrainTag を繰り込んだら、terrains からlookupが可能になる. そうなったら削除する.

    std::vector<TerrainType> &get_inner_container() override
    {
        return this->terrains;
    }

    std::optional<short> search_real_terrain(std::string_view tag) const;
};
