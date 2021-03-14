﻿#include "lore/lore-util.h"
#include "game-option/birth-options.h"
#include "monster-race/monster-race.h"
#include "term/screen-processor.h"
#include "term/term-color-types.h"

const who_word_definition Who::words = {
    { WHO_WORD_TYPE::WHO,
        {
            { false, { { MSEX_NONE, _("それ", "it") }, { MSEX_MALE, _("彼", "he") }, { MSEX_FEMALE, _("彼女", "she") } } },
            { true, { { MSEX_NONE, _("それら", "they") }, { MSEX_MALE, _("彼ら", "they") }, { MSEX_FEMALE, _("彼女ら", "they") } } },
        } },
    { WHO_WORD_TYPE::WHOSE,
        {
            { false, { { MSEX_NONE, _("それの", "its") }, { MSEX_MALE, _("彼の", "his") }, { MSEX_FEMALE, _("彼女の", "her") } } },
            { true, { { MSEX_NONE, _("それらの", "their") }, { MSEX_MALE, _("彼らの", "their") }, { MSEX_FEMALE, _("彼女らの", "their") } } },
        } },
    { WHO_WORD_TYPE::WHOM,
        {
            { false, { { MSEX_NONE, _("それ", "it") }, { MSEX_MALE, _("彼", "him") }, { MSEX_FEMALE, _("彼女", "her") } } },
            { true, { { MSEX_NONE, _("それら", "them") }, { MSEX_MALE, _("彼ら", "them") }, { MSEX_FEMALE, _("彼女ら", "them") } } },
        } },
};

/*
 * Prepare hook for c_roff(). It will be changed for spoiler generation in wizard1.c.
 */
hook_c_roff_pf hook_c_roff = c_roff;

lore_type *initialize_lore_type(lore_type *lore_ptr, MONRACE_IDX r_idx, monster_lore_mode mode)
{
#ifdef JP
#else
    lore_ptr->sin = FALSE;
#endif
    lore_ptr->r_idx = r_idx;
    lore_ptr->nightmare = ironman_nightmare && (mode != MONSTER_LORE_DEBUG);
    lore_ptr->r_ptr = &r_info[r_idx];
    lore_ptr->speed = lore_ptr->nightmare ? lore_ptr->r_ptr->speed + 5 : lore_ptr->r_ptr->speed;
    lore_ptr->drop_gold = lore_ptr->r_ptr->r_drop_gold;
    lore_ptr->drop_item = lore_ptr->r_ptr->r_drop_item;
    lore_ptr->flags1 = (lore_ptr->r_ptr->flags1 & lore_ptr->r_ptr->r_flags1);
    lore_ptr->flags2 = (lore_ptr->r_ptr->flags2 & lore_ptr->r_ptr->r_flags2);
    lore_ptr->flags3 = (lore_ptr->r_ptr->flags3 & lore_ptr->r_ptr->r_flags3);
    lore_ptr->flags4 = (lore_ptr->r_ptr->flags4 & lore_ptr->r_ptr->r_flags4);
    lore_ptr->a_ability_flags1 = (lore_ptr->r_ptr->a_ability_flags1 & lore_ptr->r_ptr->r_flags5);
    lore_ptr->a_ability_flags2 = (lore_ptr->r_ptr->a_ability_flags2 & lore_ptr->r_ptr->r_flags6);
    lore_ptr->flags7 = (lore_ptr->r_ptr->flags7 & lore_ptr->r_ptr->flags7);
    lore_ptr->flagsr = (lore_ptr->r_ptr->flagsr & lore_ptr->r_ptr->r_flagsr);
    lore_ptr->reinforce = FALSE;
    lore_ptr->know_everything = FALSE;
    lore_ptr->mode = mode;
    lore_ptr->old = FALSE;
    lore_ptr->count = 0;
    return lore_ptr;
}

/*!
 * @brief モンスターの思い出メッセージをあらかじめ指定された関数ポインタに基づき出力する
 * @param str 出力文字列
 * @return なし
 */
void hooked_roff(concptr str) { hook_c_roff(TERM_WHITE, str); }
