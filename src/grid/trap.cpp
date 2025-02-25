#include "grid/trap.h"
#include "cmd-io/cmd-dump.h"
#include "cmd-io/cmd-save.h"
#include "core/disturbance.h"
#include "dungeon/quest.h"
#include "effect/effect-characteristics.h"
#include "effect/effect-processor.h"
#include "floor/cave.h"
#include "floor/floor-mode-changer.h"
#include "game-option/birth-options.h"
#include "game-option/special-options.h"
#include "grid/feature.h"
#include "grid/grid.h"
#include "io/files-util.h"
#include "io/write-diary.h"
#include "main/sound-definitions-table.h"
#include "main/sound-of-music.h"
#include "mind/mind-mirror-master.h"
#include "monster-floor/monster-summon.h"
#include "monster-floor/place-monster-types.h"
#include "monster/monster-util.h"
#include "player-info/class-info.h"
#include "player/eldritch-horror.h"
#include "player/player-damage.h"
#include "player/player-personality-types.h"
#include "player/player-status-flags.h"
#include "player/player-status.h"
#include "spell-kind/spells-launcher.h"
#include "spell-kind/spells-random.h"
#include "spell-kind/spells-sight.h"
#include "spell-kind/spells-teleport.h"
#include "spell/summon-types.h"
#include "status/bad-status-setter.h"
#include "status/base-status.h"
#include "status/element-resistance.h"
#include "system/dungeon/dungeon-definition.h"
#include "system/floor/floor-info.h"
#include "system/grid-type-definition.h"
#include "system/monster-entity.h"
#include "system/player-type-definition.h"
#include "system/terrain/terrain-definition.h"
#include "system/terrain/terrain-list.h"
#include "target/projection-path-calculator.h"
#include "timed-effect/timed-effects.h"
#include "util/enum-converter.h"
#include "view/display-messages.h"
#include "world/world.h"

static std::vector<int16_t> normal_traps;

/*!
 * @brief 箱のトラップテーブル
 * @details
 * <pre>
 * Each chest has a certain set of traps, determined by pval
 * Each chest has a "pval" from 1 to the chest level (max 55)
 * If the "pval" is negative then the trap has been disarmed
 * The "pval" of a chest determines the quality of its treasure
 * Note that disarming a trap on a chest also removes the lock.
 * </pre>
 */
const std::vector<EnumClassFlagGroup<ChestTrapType>> chest_traps = {
    {}, /* empty */
    { ChestTrapType::POISON },
    { ChestTrapType::LOSE_STR },
    { ChestTrapType::LOSE_CON },
    { ChestTrapType::LOSE_STR },
    { ChestTrapType::LOSE_CON }, /* 5 == best small wooden */
    {},
    { ChestTrapType::ALARM },
    { ChestTrapType::ALARM },
    { ChestTrapType::LOSE_STR },
    { ChestTrapType::LOSE_CON },
    { ChestTrapType::POISON },
    { ChestTrapType::SCATTER },
    { ChestTrapType::LOSE_STR, ChestTrapType::LOSE_CON },
    { ChestTrapType::LOSE_STR, ChestTrapType::LOSE_CON },
    { ChestTrapType::SUMMON }, /* 15 == best large wooden */
    {},
    { ChestTrapType::ALARM },
    { ChestTrapType::SCATTER },
    { ChestTrapType::PARALYZE },
    { ChestTrapType::LOSE_STR, ChestTrapType::LOSE_CON },
    { ChestTrapType::SUMMON },
    { ChestTrapType::PARALYZE },
    { ChestTrapType::LOSE_STR },
    { ChestTrapType::LOSE_CON },
    { ChestTrapType::EXPLODE }, /* 25 == best small iron */
    {},
    { ChestTrapType::E_SUMMON },
    { ChestTrapType::POISON, ChestTrapType::LOSE_CON },
    { ChestTrapType::LOSE_STR, ChestTrapType::LOSE_CON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::BIRD_STORM },
    { ChestTrapType::POISON, ChestTrapType::SUMMON },
    { ChestTrapType::E_SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::EXPLODE },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON }, /* 35 == best large iron */
    {},
    { ChestTrapType::SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::EXPLODE },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::POISON, ChestTrapType::PARALYZE },
    { ChestTrapType::EXPLODE },
    { ChestTrapType::BIRD_STORM },
    { ChestTrapType::EXPLODE, ChestTrapType::E_SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::H_SUMMON }, /* 45 == best small steel */
    {},
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::BIRD_STORM },
    { ChestTrapType::RUNES_OF_EVIL },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::BIRD_STORM, ChestTrapType::ALARM },
    { ChestTrapType::H_SUMMON, ChestTrapType::ALARM },
    { ChestTrapType::RUNES_OF_EVIL },
    { ChestTrapType::H_SUMMON, ChestTrapType::SCATTER, ChestTrapType::ALARM },
    { ChestTrapType::RUNES_OF_EVIL, ChestTrapType::EXPLODE }, /* 55 == best large steel */
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
    { ChestTrapType::EXPLODE, ChestTrapType::SUMMON },
};

/*!
 * @brief タグに従って、基本トラップテーブルを初期化する / Initialize arrays for normal traps
 */
void init_normal_traps(void)
{
    static constexpr auto normal_trap_tags = {
        "TRAP_TRAPDOOR",
        "TRAP_PIT",
        "TRAP_SPIKED_PIT",
        "TRAP_POISON_PIT",
        "TRAP_TY_CURSE",
        "TRAP_TELEPORT",
        "TRAP_FIRE",
        "TRAP_ACID",
        "TRAP_SLOW",
        "TRAP_LOSE_STR",
        "TRAP_LOSE_DEX",
        "TRAP_LOSE_CON",
        "TRAP_BLIND",
        "TRAP_CONFUSE",
        "TRAP_POISON",
        "TRAP_SLEEP",
        "TRAP_TRAPS",
        "TRAP_ALARM",
    };

    std::transform(normal_trap_tags.begin(), normal_trap_tags.end(), std::back_inserter(normal_traps), [](const auto &tag) {
        return TerrainList::get_instance().get_terrain_id_by_tag(tag);
    });
}

/*!
 * @brief 基本トラップをランダムに選択する
 * @param floor_ptr 現在フロアへの参照ポインタ
 * @return 選択したトラップのID
 * @details トラップドアでないならばそのID.
 * トラップドアは、アリーナ・クエスト・ダンジョンの最下層には設置しない.
 */
short choose_random_trap(FloorType *floor_ptr)
{
    const auto &terrains = TerrainList::get_instance();
    while (true) {
        const auto terrain_id = rand_choice(normal_traps);
        if (terrains.get_terrain(terrain_id).flags.has_not(TerrainCharacteristics::MORE)) {
            return terrain_id;
        }

        if (floor_ptr->inside_arena || inside_quest(floor_ptr->get_quest_id())) {
            continue;
        }

        if (floor_ptr->dun_level >= floor_ptr->get_dungeon_definition().maxdepth) {
            continue;
        }

        return terrain_id;
    }
}

/*!
 * @brief マスに存在する隠しトラップを公開する /
 * Disclose an invisible trap
 * @param player
 * @param y 秘匿したいマスのY座標
 * @param x 秘匿したいマスのX座標
 */
void disclose_grid(PlayerType *player_ptr, POSITION y, POSITION x)
{
    auto *g_ptr = &player_ptr->current_floor_ptr->grid_array[y][x];

    if (g_ptr->cave_has_flag(TerrainCharacteristics::SECRET)) {
        /* No longer hidden */
        cave_alter_feat(player_ptr, y, x, TerrainCharacteristics::SECRET);
    } else if (g_ptr->mimic) {
        /* No longer hidden */
        g_ptr->mimic = 0;

        note_spot(player_ptr, y, x);
        lite_spot(player_ptr, y, x);
    }
}

/*!
 * @brief マスをトラップを配置する /
 * The location must be a legal, naked, floor grid.
 * @param y 配置したいマスのY座標
 * @param x 配置したいマスのX座標
 * @return
 * Note that all traps start out as "invisible" and "untyped", and then\n
 * when they are "discovered" (by detecting them or setting them off),\n
 * the trap is "instantiated" as a visible, "typed", trap.\n
 */
void place_trap(FloorType *floor_ptr, POSITION y, POSITION x)
{
    auto *g_ptr = &floor_ptr->grid_array[y][x];

    /* Paranoia -- verify location */
    if (!in_bounds(floor_ptr, y, x)) {
        return;
    }

    /* Require empty, clean, floor grid */
    if (!cave_clean_bold(floor_ptr, y, x)) {
        return;
    }

    /* Place an invisible trap */
    g_ptr->mimic = g_ptr->feat;
    g_ptr->feat = choose_random_trap(floor_ptr);
}

/*!
 * @brief プレイヤーへのトラップ命中判定 /
 * Determine if a trap affects the player.
 * @param power 基本回避難度
 * @return トラップが命中した場合TRUEを返す。
 * @details
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static int check_hit_from_monster_to_player(PlayerType *player_ptr, int power)
{
    int k;
    ARMOUR_CLASS ac;

    /* Percentile dice */
    k = randint0(100);

    /* Hack -- 5% hit, 5% miss */
    if (k < 10) {
        return k < 5;
    }

    if (player_ptr->ppersonality == PERSONALITY_LAZY) {
        if (one_in_(20)) {
            return true;
        }
    }

    /* Paranoia -- No power */
    if (power <= 0) {
        return false;
    }

    /* Total armor */
    ac = player_ptr->ac + player_ptr->to_a;

    /* Power competes against Armor */
    if (randint1(power) > ((ac * 3) / 4)) {
        return true;
    }

    /* Assume miss */
    return false;
}

/*!
 * @brief 落とし穴系トラップの判定とプレイヤーの被害処理
 * @param trap_feat_type トラップの種別ID
 */
static void hit_trap_pit(PlayerType *player_ptr, TrapType trap_feat_type)
{
    int dam;
    concptr trap_name = "";
    concptr spike_name = "";

    switch (trap_feat_type) {
    case TrapType::PIT:
        trap_name = _("落とし穴", "a pit trap");
        break;
    case TrapType::SPIKED_PIT:
        trap_name = _("スパイクが敷かれた落とし穴", "a spiked pit");
        spike_name = _("スパイク", "spikes");
        break;
    case TrapType::POISON_PIT:
        trap_name = _("スパイクが敷かれた落とし穴", "a spiked pit");
        spike_name = _("毒を塗られたスパイク", "poisonous spikes");
        break;
    default:
        return;
    }

    if (player_ptr->levitation) {
        msg_format(_("%sを飛び越えた。", "You fly over %s."), trap_name);
        return;
    }

    msg_format(_("%sに落ちてしまった！", "You have fallen into %s!"), trap_name);
    dam = Dice::roll(2, 6);
    if (((trap_feat_type != TrapType::SPIKED_PIT) && (trap_feat_type != TrapType::POISON_PIT)) || one_in_(2)) {
        take_hit(player_ptr, DAMAGE_NOESCAPE, dam, trap_name);
        return;
    }

    msg_format(_("%sが刺さった！", "You are impaled on %s!"), spike_name);
    dam = dam * 2;
    BadStatusSetter bss(player_ptr);
    (void)bss.mod_cut(randnum1<short>(dam));
    if (trap_feat_type != TrapType::POISON_PIT) {
        take_hit(player_ptr, DAMAGE_NOESCAPE, dam, trap_name);
        return;
    }

    if (has_resist_pois(player_ptr) || is_oppose_pois(player_ptr)) {
        msg_print(_("しかし毒の影響はなかった！", "The poison does not affect you!"));
        take_hit(player_ptr, DAMAGE_NOESCAPE, dam, trap_name);
        return;
    }

    dam = dam * 2;
    (void)bss.mod_poison(randnum1<short>(dam));
    take_hit(player_ptr, DAMAGE_NOESCAPE, dam, trap_name);
}

/*!
 * @brief ダーツ系トラップ（通常ダメージ）の判定とプレイヤーの被害処理
 * @return ダーツが命中した場合TRUEを返す
 */
static bool hit_trap_dart(PlayerType *player_ptr)
{
    bool hit = false;

    if (check_hit_from_monster_to_player(player_ptr, 125)) {
        msg_print(_("小さなダーツが飛んできて刺さった！", "A small dart hits you!"));
        take_hit(player_ptr, DAMAGE_ATTACK, Dice::roll(1, 4), _("ダーツの罠", "a dart trap"));
        if (!check_multishadow(player_ptr)) {
            hit = true;
        }
    } else {
        msg_print(_("小さなダーツが飛んできた！が、運良く当たらなかった。", "A small dart barely misses you."));
    }

    return hit;
}

/*!
 * @brief ダーツ系トラップ（通常ダメージ＋能力値減少）の判定とプレイヤーの被害処理
 * @param stat 低下する能力値ID
 */
static void hit_trap_lose_stat(PlayerType *player_ptr, int stat)
{
    if (hit_trap_dart(player_ptr)) {
        do_dec_stat(player_ptr, stat);
    }
}

/*!
 * @brief ダーツ系トラップ（通常ダメージ＋減速）の判定とプレイヤーの被害処理
 */
static void hit_trap_slow(PlayerType *player_ptr)
{
    if (hit_trap_dart(player_ptr)) {
        (void)BadStatusSetter(player_ptr).mod_deceleration(randint0(20) + 20, false);
    }
}

/*!
 * @brief プレイヤーへのトラップ作動処理メインルーチン /
 * Handle player hitting a real trap
 * @param break_trap 作動後のトラップ破壊が確定しているならばTRUE
 * @todo cmd-save.h への依存あり。コールバックで何とかしたい
 */
void hit_trap(PlayerType *player_ptr, bool break_trap)
{
    const Pos2D p_pos(player_ptr->y, player_ptr->x);
    const auto &floor = *player_ptr->current_floor_ptr;
    const auto &grid = floor.get_grid(p_pos);
    const auto &terrain = grid.get_terrain();
    TrapType trap_feat_type = terrain.flags.has(TerrainCharacteristics::TRAP) ? i2enum<TrapType>(terrain.subtype) : TrapType::NOT_TRAP;

    disturb(player_ptr, false, true);

    cave_alter_feat(player_ptr, p_pos.y, p_pos.x, TerrainCharacteristics::HIT_TRAP);

    /* Analyze */
    switch (trap_feat_type) {
    case TrapType::TRAPDOOR: {
        if (player_ptr->levitation) {
            msg_print(_("落とし戸を飛び越えた。", "You fly over a trap door."));
        } else {
            msg_print(_("落とし戸に落ちた！", "You have fallen through a trap door!"));
            if (is_echizen(player_ptr)) {
                msg_print(_("くっそ～！", ""));
            } else if (is_chargeman(player_ptr)) {
                msg_print(_("ジュラル星人の仕業に違いない！", ""));
            }

            sound(SOUND_FALL);
            const auto dam = Dice::roll(2, 8);
            constexpr auto name = _("落とし戸", "a trap door");

            take_hit(player_ptr, DAMAGE_NOESCAPE, dam, name);

            if (floor.is_in_quest()) {
                return;
            }

            /* Still alive and autosave enabled */
            if (autosave_l && (player_ptr->chp >= 0)) {
                do_cmd_save_game(player_ptr, true);
            }

            exe_write_diary(floor, DiaryKind::DESCRIPTION, 0, _("落とし戸に落ちた", "fell through a trap door!"));
            FloorChangeModesStore::get_instace()->set({ FloorChangeMode::SAVE_FLOORS, FloorChangeMode::DOWN, FloorChangeMode::RANDOM_PLACE, FloorChangeMode::RANDOM_CONNECT });
            player_ptr->leaving = true;
        }
        break;
    }

    case TrapType::PIT:
    case TrapType::SPIKED_PIT:
    case TrapType::POISON_PIT: {
        hit_trap_pit(player_ptr, trap_feat_type);
        break;
    }

    case TrapType::TY_CURSE: {
        msg_print(_("何かがピカッと光った！", "There is a flash of shimmering light!"));
        const auto num = 2 + randint1(3);
        for (auto i = 0; i < num; i++) {
            (void)summon_specific(player_ptr, p_pos.y, p_pos.x, player_ptr->current_floor_ptr->dun_level, SUMMON_NONE, (PM_ALLOW_GROUP | PM_ALLOW_UNIQUE | PM_NO_PET));
        }

        if (player_ptr->current_floor_ptr->dun_level > randint1(100)) /* No nasty effect for low levels */
        {
            bool stop_ty = false;
            int count = 0;

            do {
                stop_ty = activate_ty_curse(player_ptr, stop_ty, &count);
            } while (one_in_(6));
        }
        break;
    }

    case TrapType::TELEPORT: {
        msg_print(_("テレポート・トラップにひっかかった！", "You hit a teleport trap!"));
        teleport_player(player_ptr, 100, TELEPORT_PASSIVE);
        break;
    }

    case TrapType::FIRE: {
        msg_print(_("炎に包まれた！", "You are enveloped in flames!"));
        const auto dam = Dice::roll(4, 6);
        (void)fire_dam(player_ptr, dam, _("炎のトラップ", "a fire trap"), false);
        break;
    }

    case TrapType::ACID: {
        msg_print(_("酸が吹きかけられた！", "You are splashed with acid!"));
        const auto dam = Dice::roll(4, 6);
        (void)acid_dam(player_ptr, dam, _("酸のトラップ", "an acid trap"), false);
        break;
    }

    case TrapType::SLOW: {
        hit_trap_slow(player_ptr);
        break;
    }

    case TrapType::LOSE_STR: {
        hit_trap_lose_stat(player_ptr, A_STR);
        break;
    }

    case TrapType::LOSE_DEX: {
        hit_trap_lose_stat(player_ptr, A_DEX);
        break;
    }

    case TrapType::LOSE_CON: {
        hit_trap_lose_stat(player_ptr, A_CON);
        break;
    }

    case TrapType::BLIND:
        msg_print(_("黒いガスに包み込まれた！", "A black gas surrounds you!"));
        if (has_resist_blind(player_ptr) == 0) {
            (void)BadStatusSetter(player_ptr).mod_blindness(randint0(50) + 25);
        }

        break;
    case TrapType::CONFUSE: {
        msg_print(_("きらめくガスに包み込まれた！", "A gas of scintillating colors surrounds you!"));
        if (has_resist_conf(player_ptr) == 0) {
            (void)BadStatusSetter(player_ptr).mod_confusion(randint0(20) + 10);
        }

        break;
    }

    case TrapType::POISON: {
        msg_print(_("刺激的な緑色のガスに包み込まれた！", "A pungent green gas surrounds you!"));
        if (has_resist_pois(player_ptr) == 0) {
            (void)BadStatusSetter(player_ptr).mod_poison(randint0(20) + 10);
        }

        break;
    }

    case TrapType::SLEEP: {
        msg_print(_("奇妙な白い霧に包まれた！", "A strange white mist surrounds you!"));
        if (player_ptr->free_act) {
            break;
        }

        msg_print(_("あなたは眠りに就いた。", "You fall asleep."));
        if (ironman_nightmare) {
            msg_print(_("身の毛もよだつ光景が頭に浮かんだ。", "A horrible vision enters your mind."));
            sanity_blast(player_ptr, nullptr, false);
        }

        (void)BadStatusSetter(player_ptr).mod_paralysis(randint0(10) + 5);
        break;
    }

    case TrapType::TRAPS: {
        msg_print(_("まばゆい閃光が走った！", "There is a bright flash of light!"));
        /* Make some new traps */
        project(player_ptr, 0, 1, p_pos.y, p_pos.x, 0, AttributeType::MAKE_TRAP, PROJECT_HIDE | PROJECT_JUMP | PROJECT_GRID);

        break;
    }

    case TrapType::ALARM: {
        msg_print(_("けたたましい音が鳴り響いた！", "An alarm sounds!"));

        aggravate_monsters(player_ptr, 0);

        break;
    }

    case TrapType::OPEN: {
        msg_print(_("大音響と共にまわりの壁が崩れた！", "Suddenly, surrounding walls are opened!"));
        (void)project(player_ptr, 0, 3, p_pos.y, p_pos.x, 0, AttributeType::DISINTEGRATE, PROJECT_GRID | PROJECT_HIDE);
        (void)project(player_ptr, 0, 3, p_pos.y, p_pos.x - 4, 0, AttributeType::DISINTEGRATE, PROJECT_GRID | PROJECT_HIDE);
        (void)project(player_ptr, 0, 3, p_pos.y, p_pos.x + 4, 0, AttributeType::DISINTEGRATE, PROJECT_GRID | PROJECT_HIDE);
        aggravate_monsters(player_ptr, 0);

        break;
    }

    case TrapType::ARMAGEDDON: {
        static int levs[10] = { 0, 0, 20, 10, 5, 3, 2, 1, 1, 1 };
        int evil_idx = 0, good_idx = 0;

        DEPTH lev;
        msg_print(_("突然天界の戦争に巻き込まれた！", "Suddenly, you are surrounded by immotal beings!"));

        /* Summon Demons and Angels */
        for (lev = player_ptr->current_floor_ptr->dun_level; lev >= 20; lev -= 1 + lev / 16) {
            const auto num = levs[std::min(lev / 10, 9)];
            for (auto i = 0; i < num; i++) {
                const Pos2D pos(rand_spread(p_pos.y, 5), rand_spread(p_pos.x, 7));
                if (!in_bounds(player_ptr->current_floor_ptr, pos.y, pos.x)) {
                    continue;
                }

                /* Require line of projection */
                if (!projectable(player_ptr, p_pos, pos)) {
                    continue;
                }

                if (auto m_idx = summon_specific(player_ptr, pos.y, pos.x, lev, SUMMON_ARMAGE_EVIL, (PM_NO_PET))) {
                    evil_idx = *m_idx;
                }

                if (auto m_idx = summon_specific(player_ptr, pos.y, pos.x, lev, SUMMON_ARMAGE_GOOD, (PM_NO_PET))) {
                    good_idx = *m_idx;
                }

                /* Let them fight each other */
                if (evil_idx && good_idx) {
                    MonsterEntity *evil_ptr = &player_ptr->current_floor_ptr->m_list[evil_idx];
                    MonsterEntity *good_ptr = &player_ptr->current_floor_ptr->m_list[good_idx];
                    evil_ptr->target_y = good_ptr->fy;
                    evil_ptr->target_x = good_ptr->fx;
                    good_ptr->target_y = evil_ptr->fy;
                    good_ptr->target_x = evil_ptr->fx;
                }
            }
        }

        break;
    }

    case TrapType::PIRANHA: {
        msg_print(_("突然壁から水が溢れ出した！ピラニアがいる！", "Suddenly, the room is filled with water with piranhas!"));

        /* Water fills room */
        fire_ball_hide(player_ptr, AttributeType::WATER_FLOW, 0, 1, 10);

        /* Summon Piranhas */
        const auto num = 1 + player_ptr->current_floor_ptr->dun_level / 20;
        for (auto i = 0; i < num; i++) {
            (void)summon_specific(player_ptr, p_pos.y, p_pos.x, player_ptr->current_floor_ptr->dun_level, SUMMON_PIRANHAS, (PM_ALLOW_GROUP | PM_NO_PET));
        }
        break;
    }

    default:
        break;
    }

    if (break_trap && floor.is_trap(p_pos)) {
        cave_alter_feat(player_ptr, p_pos.y, p_pos.x, TerrainCharacteristics::DISARM);
        msg_print(_("トラップを粉砕した。", "You destroyed the trap."));
    }
}
