#pragma once

#include "dungeon/dungeon-flag-types.h"
#include "monster-race/race-ability-flags.h"
#include "monster-race/race-behavior-flags.h"
#include "monster-race/race-brightness-flags.h"
#include "monster-race/race-drop-flags.h"
#include "monster-race/race-feature-flags.h"
#include "monster-race/race-flags-resistance.h"
#include "monster-race/race-kind-flags.h"
#include "monster-race/race-misc-flags.h"
#include "monster-race/race-population-flags.h"
#include "monster-race/race-resistance-mask.h"
#include "monster-race/race-speak-flags.h"
#include "monster-race/race-special-flags.h"
#include "monster-race/race-visual-flags.h"
#include "monster-race/race-wilderness-flags.h"
#include "system/angband.h"
#include "util/flag-group.h"
#include <array>
#include <string>
#include <vector>

constexpr auto DUNGEON_FEAT_PROB_NUM = 3;

enum class FixedArtifactId : short;
enum class MonraceId : short;
enum class MonsterSex;

enum class DungeonMode {
    AND = 1,
    NAND = 2,
    OR = 3,
    NOR = 4,
};

struct feat_prob {
    FEAT_IDX feat{}; /* Feature tile */
    PERCENTAGE percent{}; /* Chance of type */
};

/* A structure for the != dungeon types */
enum class TerrainCharacteristics;
class MonraceDefinition;
class DungeonDefinition {
public:
    std::string name; /* Name */
    std::string text; /* Description */

    POSITION dy{};
    POSITION dx{};

    std::array<feat_prob, DUNGEON_FEAT_PROB_NUM> floor{}; /* Floor probability */
    std::array<feat_prob, DUNGEON_FEAT_PROB_NUM> fill{}; /* Cave wall probability */
    FEAT_IDX outer_wall{}; /* Outer wall tile */
    FEAT_IDX inner_wall{}; /* Inner wall tile */
    FEAT_IDX stream1{}; /* stream tile */
    FEAT_IDX stream2{}; /* stream tile */

    DEPTH mindepth{}; /* Minimal depth */
    DEPTH maxdepth{}; /* Maximal depth */
    PLAYER_LEVEL min_plev{}; /* Minimal plev needed to enter -- it's an anti-cheating mesure */
    BIT_FLAGS16 pit{};
    BIT_FLAGS16 nest{};
    DungeonMode mode{}; /* Mode of combinaison of the monster flags */

    int min_m_alloc_level{}; /* Minimal number of monsters per level */
    int max_m_alloc_chance{}; /* There is a 1/max_m_alloc_chance chance per round of creating a new monster */

    EnumClassFlagGroup<DungeonFeatureType> flags{}; /* Dungeon Flags */

    EnumClassFlagGroup<MonsterAbilityType> mon_ability_flags;
    EnumClassFlagGroup<MonsterBehaviorType> mon_behavior_flags;
    EnumClassFlagGroup<MonsterVisualType> mon_visual_flags;
    EnumClassFlagGroup<MonsterKindType> mon_kind_flags;
    EnumClassFlagGroup<MonsterResistanceType> mon_resistance_flags;
    EnumClassFlagGroup<MonsterDropType> mon_drop_flags;
    EnumClassFlagGroup<MonsterWildernessType> mon_wilderness_flags;
    EnumClassFlagGroup<MonsterFeatureType> mon_feature_flags;
    EnumClassFlagGroup<MonsterPopulationType> mon_population_flags;
    EnumClassFlagGroup<MonsterSpeakType> mon_speak_flags;
    EnumClassFlagGroup<MonsterBrightnessType> mon_brightness_flags;
    EnumClassFlagGroup<MonsterSpecialType> mon_special_flags;
    EnumClassFlagGroup<MonsterMiscType> mon_misc_flags;
    MonsterSex mon_sex{};

    std::vector<char> r_chars; /* Monster symbols allowed */
    short final_object{}; /* The object you'll find at the bottom */
    FixedArtifactId final_artifact{}; /* The artifact you'll find at the bottom */
    MonraceId final_guardian{}; /* The artifact's guardian. If an artifact is specified, then it's NEEDED */

    PROB special_div{}; /* % of monsters affected by the flags/races allowed, to add some variety */
    int tunnel_percent{};
    int obj_great{};
    int obj_good{};

    bool has_river_flag() const;
    bool has_guardian() const;
    MonraceDefinition &get_guardian();
    const MonraceDefinition &get_guardian() const;
    short convert_terrain_id(short terrain_id, TerrainCharacteristics action) const;
    short convert_terrain_id(short terrain_id) const;
    bool is_open(short terrain_id) const;
    bool is_conquered() const;
    std::string build_entrance_message() const;
    std::string describe_depth() const;

    void set_guardian_flag();
};
