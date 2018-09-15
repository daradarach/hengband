﻿/*!
    @file cmd-spell.c
    @brief 魔法のインターフェイスと発動 / Purpose: Do everything for each spell
    @date 2013/12/31
    @author
    2013 Deskull rearranged comment for Doxygen.
 */

#include "angband.h"
#include "selfinfo.h"
#include "cmd-quaff.h"
#include "spells-summon.h"
#include "realm-arcane.h"
#include "realm-chaos.h"
#include "realm-craft.h"
#include "realm-crusade.h"
#include "realm-daemon.h"
#include "realm-death.h"
#include "realm-life.h"
#include "realm-song.h"
#include "realm-nature.h"
#include "realm-sorcery.h"
#include "realm-trump.h"

/*!
 * @brief
 * 魔法の効果を「キャプション:ダイス＋定数値」のフォーマットで出力する / Generate dice info string such as "foo 2d10"
 * @param str キャプション
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_string_dice(cptr str, int dice, int sides, int base)
{
	/* Fix value */
	if (!dice)
		return format("%s%d", str, base);

	/* Dice only */
	else if (!base)
		return format("%s%dd%d", str, dice, sides);

	/* Dice plus base value */
	else
		return format("%s%dd%d%+d", str, dice, sides, base);
}


/*!
 * @brief 魔法によるダメージを出力する / Generate damage-dice info string such as "dam 2d10"
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_damage(int dice, int sides, int base)
{
	return info_string_dice(_("損傷:", "dam "), dice, sides, base);
}

/*!
 * @brief 魔法の効果時間を出力する / Generate duration info string such as "dur 20+1d20"
 * @param base 固定値
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_duration(int base, int sides)
{
	return format(_("期間:%d+1d%d", "dur %d+1d%d"), base, sides);
}

/*!
 * @brief 魔法の効果範囲を出力する / Generate range info string such as "range 5"
 * @param range 効果範囲
 * @return フォーマットに従い整形された文字列
 */
cptr info_range(POSITION range)
{
	return format(_("範囲:%d", "range %d"), range);
}

/*!
 * @brief 魔法による回復量を出力する / Generate heal info string such as "heal 2d8"
 * @param dice ダイス数
 * @param sides ダイス目
 * @param base 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_heal(int dice, int sides, int base)
{
	return info_string_dice(_("回復:", "heal "), dice, sides, base);
}

/*!
 * @brief 魔法効果発動までの遅延ターンを出力する / Generate delay info string such as "delay 15+1d15"
 * @param base 固定値
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_delay(int base, int sides)
{
	return format(_("遅延:%d+1d%d", "delay %d+1d%d"), base, sides);
}


/*!
 * @brief 魔法によるダメージを出力する(固定値＆複数回処理) / Generate multiple-damage info string such as "dam 25 each"
 * @param dam 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_multi_damage(HIT_POINT dam)
{
	return format(_("損傷:各%d", "dam %d each"), dam);
}


/*!
 * @brief 魔法によるダメージを出力する(ダイスのみ＆複数回処理) / Generate multiple-damage-dice info string such as "dam 5d2 each"
 * @param dice ダイス数
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
cptr info_multi_damage_dice(int dice, int sides)
{
	return format(_("損傷:各%dd%d", "dam %dd%d each"), dice, sides);
}

/*!
 * @brief 魔法による一般的な効力値を出力する（固定値） / Generate power info string such as "power 100"
 * @param power 固定値
 * @return フォーマットに従い整形された文字列
 */
cptr info_power(int power)
{
	return format(_("効力:%d", "power %d"), power);
}


/*!
 * @brief 魔法による一般的な効力値を出力する（ダイス値） / Generate power info string such as "power 100"
 * @param dice ダイス数
 * @param sides ダイス目
 * @return フォーマットに従い整形された文字列
 */
/*
 * Generate power info string such as "power 1d100"
 */
cptr info_power_dice(int dice, int sides)
{
	return format(_("効力:%dd%d", "power %dd%d"), dice, sides);
}


/*!
 * @brief 魔法の効果半径を出力する / Generate radius info string such as "rad 100"
 * @param rad 効果半径
 * @return フォーマットに従い整形された文字列
 */
cptr info_radius(int rad)
{
	return format(_("半径:%d", "rad %d"), rad);
}


/*!
 * @brief 魔法効果の限界重量を出力する / Generate weight info string such as "max wgt 15"
 * @param weight 最大重量
 * @return フォーマットに従い整形された文字列
 */
cptr info_weight(int weight)
{
#ifdef JP
	return format("最大重量:%d.%dkg", lbtokg1(weight), lbtokg2(weight));
#else
	return format("max wgt %d", weight/10);
#endif
}


/*!
 * @brief 剣術の各処理を行う
 * @param spell 剣術ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_CAST)
 * @return SPELL_NAME / SPELL_DESC 時には文字列ポインタを返す。SPELL_CAST時はNULL文字列を返す。
 */
static cptr do_hissatsu_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;

	int dir;
	int plev = p_ptr->lev;

	switch (spell)
	{
	case 0:
		if (name) return _("飛飯綱", "Tobi-Izuna");
		if (desc) return _("2マス離れたところにいるモンスターを攻撃する。", "Attacks a two squares distant monster.");
    
		if (cast)
		{
			project_length = 2;
			if (!get_aim_dir(&dir)) return NULL;

			project_hook(GF_ATTACK, dir, HISSATSU_2, PROJECT_STOP | PROJECT_KILL);
		}
		break;

	case 1:
		if (name) return _("五月雨斬り", "3-Way Attack");
		if (desc) return _("3方向に対して攻撃する。", "Attacks in 3 directions in one time.");
    
		if (cast)
		{
			int cdir;
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (cdir = 0;cdir < 8; cdir++)
			{
				if (cdd[cdir] == dir) break;
			}

			if (cdir == 8) return NULL;

			y = p_ptr->y + ddy_cdd[cdir];
			x = p_ptr->x + ddx_cdd[cdir];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
			
			y = p_ptr->y + ddy_cdd[(cdir + 7) % 8];
			x = p_ptr->x + ddx_cdd[(cdir + 7) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
			
			y = p_ptr->y + ddy_cdd[(cdir + 1) % 8];
			x = p_ptr->x + ddx_cdd[(cdir + 1) % 8];
			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
				msg_print(_("攻撃は空を切った。", "You attack the empty air."));
		}
		break;

	case 2:
		if (name) return _("ブーメラン", "Boomerang");
		if (desc) return _("武器を手元に戻ってくるように投げる。戻ってこないこともある。", 
			"Throws current weapon. And it'll return to your hand unless failed.");
    
		if (cast)
		{
			if (!do_cmd_throw(1, TRUE, -1)) return NULL;
		}
		break;

	case 3:
		if (name) return _("焔霊", "Burning Strike");
		if (desc) return _("火炎耐性のないモンスターに大ダメージを与える。", "Attacks a monster with more damage unless it has resistance to fire.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_FIRE);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 4:
		if (name) return _("殺気感知", "Detect Ferocity");
		if (desc) return _("近くの思考することができるモンスターを感知する。", "Detects all monsters except mindless in your vicinity.");
    
		if (cast)
		{
			detect_monsters_mind(DETECT_RAD_DEFAULT);
		}
		break;

	case 5:
		if (name) return _("みね打ち", "Strike to Stun");
		if (desc) return _("相手にダメージを与えないが、朦朧とさせる。", "Attempts to stun a monster in the adjacent.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MINEUCHI);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 6:
		if (name) return _("カウンター", "Counter");
		if (desc) return _("相手に攻撃されたときに反撃する。反撃するたびにMPを消費。", 
			"Prepares to counterattack. When attack by a monster, strikes back using SP each time.");
    
		if (cast)
		{
			if (p_ptr->riding)
			{
				msg_print(_("乗馬中には無理だ。", "You cannot do it when riding."));
				return NULL;
			}
			msg_print(_("相手の攻撃に対して身構えた。", "You prepare to counter blow."));
			p_ptr->counter = TRUE;
		}
		break;

	case 7:
		if (name) return _("払い抜け", "Harainuke");
		if (desc) return _("攻撃した後、反対側に抜ける。", 
			"Attacks monster with your weapons normally, then move through counter side of the monster.");
    
		if (cast)
		{
			POSITION y, x;

			if (p_ptr->riding)
			{
				msg_print(_("乗馬中には無理だ。", "You cannot do it when riding."));
				return NULL;
			}
	
			if (!get_rep_dir2(&dir)) return NULL;
	
			if (dir == 5) return NULL;
			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];
	
			if (!cave[y][x].m_idx)
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
	
			py_attack(y, x, 0);
	
			if (!player_can_enter(cave[y][x].feat, 0) || is_trap(cave[y][x].feat))
				break;
	
			y += ddy[dir];
			x += ddx[dir];
	
			if (player_can_enter(cave[y][x].feat, 0) && !is_trap(cave[y][x].feat) && !cave[y][x].m_idx)
			{
				msg_print(NULL);
	
				/* Move the player */
				(void)move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP);
			}
		}
		break;

	case 8:
		if (name) return _("サーペンツタン", "Serpent's Tongue");
		if (desc) return _("毒耐性のないモンスターに大ダメージを与える。", "Attacks a monster with more damage unless it has resistance to poison.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_POISON);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 9:
		if (name) return _("斬魔剣弐の太刀", "Zammaken");
		if (desc) return _("生命のない邪悪なモンスターに大ダメージを与えるが、他のモンスターには全く効果がない。", 
			"Attacks an evil unliving monster with great damage. No effect to other  monsters.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ZANMA);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 10:
		if (name) return _("裂風剣", "Wind Blast");
		if (desc) return _("攻撃した相手を後方へ吹き飛ばす。", "Attacks an adjacent monster, and blow it away.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, 0);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
				return "";
			}
			if (cave[y][x].m_idx)
			{
				int i;
				POSITION ty = y, tx = x;
				POSITION oy = y, ox = x;
				MONSTER_IDX m_idx = cave[y][x].m_idx;
				monster_type *m_ptr = &m_list[m_idx];
				char m_name[80];
	
				monster_desc(m_name, m_ptr, 0);
	
				for (i = 0; i < 5; i++)
				{
					y += ddy[dir];
					x += ddx[dir];
					if (cave_empty_bold(y, x))
					{
						ty = y;
						tx = x;
					}
					else break;
				}
				if ((ty != oy) || (tx != ox))
				{
					msg_format(_("%sを吹き飛ばした！", "You blow %s away!"), m_name);
					cave[oy][ox].m_idx = 0;
					cave[ty][tx].m_idx = m_idx;
					m_ptr->fy = ty;
					m_ptr->fx = tx;
	
					update_mon(m_idx, TRUE);
					lite_spot(oy, ox);
					lite_spot(ty, tx);
	
					if (r_info[m_ptr->r_idx].flags7 & (RF7_LITE_MASK | RF7_DARK_MASK))
						p_ptr->update |= (PU_MON_LITE);
				}
			}
		}
		break;

	case 11:
		if (name) return _("刀匠の目利き", "Judge");
		if (desc) return _("武器・防具を1つ識別する。レベル45以上で武器・防具の能力を完全に知ることができる。", 
			"Identifies a weapon or armor. Or *identifies* these at level 45.");
    
		if (cast)
		{
			if (plev > 44)
			{
				if (!identify_fully(TRUE)) return NULL;
			}
			else
			{
				if (!ident_spell(TRUE)) return NULL;
			}
		}
		break;

	case 12:
		if (name) return _("破岩斬", "Rock Smash");
		if (desc) return _("岩を壊し、岩石系のモンスターに大ダメージを与える。", "Breaks rock. Or greatly damage a monster made by rocks.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_HAGAN);
	
			if (!cave_have_flag_bold(y, x, FF_HURT_ROCK)) break;
	
			/* Destroy the feature */
			cave_alter_feat(y, x, FF_HURT_ROCK);
	
			/* Update some things */
			p_ptr->update |= (PU_FLOW);
		}
		break;

	case 13:
		if (name) return _("乱れ雪月花", "Midare-Setsugekka");
		if (desc) return _("攻撃回数が増え、冷気耐性のないモンスターに大ダメージを与える。", 
			"Attacks a monster with increased number of attacks and more damage unless it has resistance to cold.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_COLD);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 14:
		if (name) return _("急所突き", "Spot Aiming");
		if (desc) return _("モンスターを一撃で倒す攻撃を繰り出す。失敗すると1点しかダメージを与えられない。", 
			"Attempts to kill a monster instantly. If failed cause only 1HP of damage.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_KYUSHO);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 15:
		if (name) return _("魔神斬り", "Majingiri");
		if (desc) return _("会心の一撃で攻撃する。攻撃がかわされやすい。", 
			"Attempts to attack with critical hit. But this attack is easy to evade for a monster.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_MAJIN);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 16:
		if (name) return _("捨て身", "Desperate Attack");
		if (desc) return _("強力な攻撃を繰り出す。次のターンまでの間、食らうダメージが増える。", 
			"Attacks with all of your power. But all damages you take will be doubled for one turn.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_SUTEMI);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			p_ptr->sutemi = TRUE;
		}
		break;

	case 17:
		if (name) return _("雷撃鷲爪斬", "Lightning Eagle");
		if (desc) return _("電撃耐性のないモンスターに非常に大きいダメージを与える。", 
			"Attacks a monster with more damage unless it has resistance to electricity.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_ELEC);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 18:
		if (name) return _("入身", "Rush Attack");
		if (desc) return _("素早く相手に近寄り攻撃する。", "Steps close to a monster and attacks at a time.");
    
		if (cast)
		{
			if (!rush_attack(NULL)) return NULL;
		}
		break;

	case 19:
		if (name) return _("赤流渦", "Bloody Maelstrom");
		if (desc) return _("自分自身も傷を作りつつ、その傷が深いほど大きい威力で全方向の敵を攻撃できる。生きていないモンスターには効果がない。", 
			"Attacks all adjacent monsters with power corresponding to your cut status. Then increases your cut status. No effect to unliving monsters.");
    
		if (cast)
		{
			int y = 0, x = 0;

			cave_type       *c_ptr;
			monster_type    *m_ptr;
	
			if (p_ptr->cut < 300)
				set_cut(p_ptr->cut + 300);
			else
				set_cut(p_ptr->cut * 2);
	
			for (dir = 0; dir < 8; dir++)
			{
				y = p_ptr->y + ddy_ddd[dir];
				x = p_ptr->x + ddx_ddd[dir];
				c_ptr = &cave[y][x];
	
				/* Get the monster */
				m_ptr = &m_list[c_ptr->m_idx];
	
				/* Hack -- attack monsters */
				if (c_ptr->m_idx && (m_ptr->ml || cave_have_flag_bold(y, x, FF_PROJECT)))
				{
					if (!monster_living(&r_info[m_ptr->r_idx]))
					{
						char m_name[80];
	
						monster_desc(m_name, m_ptr, 0);
						msg_format(_("%sには効果がない！", "%s is unharmed!"), m_name);
					}
					else py_attack(y, x, HISSATSU_SEKIRYUKA);
				}
			}
		}
		break;

	case 20:
		if (name) return _("激震撃", "Earthquake Blow");
		if (desc) return _("地震を起こす。", "Shakes dungeon structure, and results in random swapping of floors and walls.");
    
		if (cast)
		{
			int y,x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_QUAKE);
			else
				earthquake(p_ptr->y, p_ptr->x, 10);
		}
		break;

	case 21:
		if (name) return _("地走り", "Crack");
		if (desc) return _("衝撃波のビームを放つ。", "Fires a beam of shock wave.");
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
			if (!get_aim_dir(&dir)) return NULL;
			msg_print(_("武器を大きく振り下ろした。", "You swing your weapon downward."));
			for (i = 0; i < 2; i++)
			{
				int damage;
	
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage *= p_ptr->num_blow[i];
				total_damage += damage / 200;
				if (i) total_damage = total_damage*7/10;
			}
			fire_beam(GF_FORCE, dir, total_damage);
		}
		break;

	case 22:
		if (name) return _("気迫の雄叫び", "War Cry");
		if (desc) return _("視界内の全モンスターに対して轟音の攻撃を行う。さらに、近くにいるモンスターを怒らせる。", 
			"Damages all monsters in sight with sound. Aggravate nearby monsters.");
    
		if (cast)
		{
			msg_print(_("雄叫びをあげた！", "You roar out!"));
			project_hack(GF_SOUND, randint1(plev * 3));
			aggravate_monsters(0);
		}
		break;

	case 23:
		if (name) return _("無双三段", "Musou-Sandan");
		if (desc) return _("強力な3段攻撃を繰り出す。", "Attacks with powerful 3 strikes.");
    
		if (cast)
		{
			int i;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			for (i = 0; i < 3; i++)
			{
				POSITION y, x;
				POSITION ny, nx;
				MONSTER_IDX m_idx;
				cave_type *c_ptr;
				monster_type *m_ptr;
	
				y = p_ptr->y + ddy[dir];
				x = p_ptr->x + ddx[dir];
				c_ptr = &cave[y][x];
	
				if (c_ptr->m_idx)
					py_attack(y, x, HISSATSU_3DAN);
				else
				{
					msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
					return NULL;
				}
	
				if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
				{
					return "";
				}
	
				/* Monster is dead? */
				if (!c_ptr->m_idx) break;
	
				ny = y + ddy[dir];
				nx = x + ddx[dir];
				m_idx = c_ptr->m_idx;
				m_ptr = &m_list[m_idx];
	
				/* Monster cannot move back? */
				if (!monster_can_enter(ny, nx, &r_info[m_ptr->r_idx], 0))
				{
					/* -more- */
					if (i < 2) msg_print(NULL);
					continue;
				}
	
				c_ptr->m_idx = 0;
				cave[ny][nx].m_idx = m_idx;
				m_ptr->fy = ny;
				m_ptr->fx = nx;
	
				update_mon(m_idx, TRUE);
	
				/* Redraw the old spot */
				lite_spot(y, x);
	
				/* Redraw the new spot */
				lite_spot(ny, nx);
	
				/* Player can move forward? */
				if (player_can_enter(c_ptr->feat, 0))
				{
					/* Move the player */
					if (!move_player_effect(y, x, MPE_FORGET_FLOW | MPE_HANDLE_STUFF | MPE_DONT_PICKUP)) break;
				}
				else
				{
					break;
				}

				/* -more- */
				if (i < 2) msg_print(NULL);
			}
		}
		break;

	case 24:
		if (name) return _("吸血鬼の牙", "Vampire's Fang");
		if (desc) return _("攻撃した相手の体力を吸いとり、自分の体力を回復させる。生命を持たないモンスターには通じない。", 
			"Attacks with vampiric strikes which absorbs HP from a monster and gives them to you. No effect to unliving monsters.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_DRAIN);
			else
			{
					msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
		}
		break;

	case 25:
		if (name) return _("幻惑", "Moon Dazzling");
		if (desc) return _("視界内の起きている全モンスターに朦朧、混乱、眠りを与えようとする。", "Attempts to stun, confuse and sleep all waking monsters.");
    
		if (cast)
		{
			msg_print(_("武器を不規則に揺らした．．．", "You irregularly wave your weapon..."));
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
			project_hack(GF_ENGETSU, plev * 4);
		}
		break;

	case 26:
		if (name) return _("百人斬り", "Hundred Slaughter");
		if (desc) return _("連続して入身でモンスターを攻撃する。攻撃するたびにMPを消費。MPがなくなるか、モンスターを倒せなかったら百人斬りは終了する。", 
			"Performs a series of rush attacks. The series continues while killing each monster in a time and SP remains.");
    
		if (cast)
		{
			const int mana_cost_per_monster = 8;
			bool is_new = TRUE;
			bool mdeath;

			do
			{
				if (!rush_attack(&mdeath)) break;
				if (is_new)
				{
					/* Reserve needed mana point */
					p_ptr->csp -= technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
					is_new = FALSE;
				}
				else
					p_ptr->csp -= mana_cost_per_monster;

				if (!mdeath) break;
				command_dir = 0;

				p_ptr->redraw |= PR_MANA;
				handle_stuff();
			}
			while (p_ptr->csp > mana_cost_per_monster);

			if (is_new) return NULL;
	
			/* Restore reserved mana */
			p_ptr->csp += technic_info[REALM_HISSATSU - MIN_TECHNIC][26].smana;
		}
		break;

	case 27:
		if (name) return _("天翔龍閃", "Dragonic Flash");
		if (desc) return _("視界内の場所を指定して、その場所と自分の間にいる全モンスターを攻撃し、その場所に移動する。", 
			"Runs toward given location while attacking all monsters on the path.");
    
		if (cast)
		{
			POSITION y, x;

			if (!tgt_pt(&x, &y)) return NULL;

			if (!cave_player_teleportable_bold(y, x, 0L) ||
			    (distance(y, x, p_ptr->y, p_ptr->x) > MAX_SIGHT / 2) ||
			    !projectable(p_ptr->y, p_ptr->x, y, x))
			{
				msg_print(_("失敗！", "You cannot move to that place!"));
				break;
			}
			if (p_ptr->anti_tele)
			{
				msg_print(_("不思議な力がテレポートを防いだ！", "A mysterious force prevents you from teleporting!"));
				break;
			}
			project(0, 0, y, x, HISSATSU_ISSEN, GF_ATTACK, PROJECT_BEAM | PROJECT_KILL, -1);
			teleport_player_to(y, x, 0L);
		}
		break;

	case 28:
		if (name) return _("二重の剣撃", "Twin Slash");
		if (desc) return _("1ターンで2度攻撃を行う。", "double attacks at a time.");
    
		if (cast)
		{
			int x, y;
	
			if (!get_rep_dir(&dir, FALSE)) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
			{
				py_attack(y, x, 0);
				if (cave[y][x].m_idx)
				{
					handle_stuff();
					py_attack(y, x, 0);
				}
			}
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "You don't see any monster in this direction"));
				return NULL;
			}
		}
		break;

	case 29:
		if (name) return _("虎伏絶刀勢", "Kofuku-Zettousei");
		if (desc) return _("強力な攻撃を行い、近くの場所にも効果が及ぶ。", "Performs a powerful attack which even effect nearby monsters.");
    
		if (cast)
		{
			int total_damage = 0, basedam, i;
			int y, x;
			u32b flgs[TR_FLAG_SIZE];
			object_type *o_ptr;
	
			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (d_info[dungeon_type].flags1 & DF1_NO_MELEE)
			{
				msg_print(_("なぜか攻撃することができない。", "Something prevent you from attacking."));
				return "";
			}
			msg_print(_("武器を大きく振り下ろした。", "You swing your weapon downward."));
			for (i = 0; i < 2; i++)
			{
				int damage;
				if (!buki_motteruka(INVEN_RARM+i)) break;
				o_ptr = &inventory[INVEN_RARM+i];
				basedam = (o_ptr->dd * (o_ptr->ds + 1)) * 50;
				damage = o_ptr->to_d * 100;
				object_flags(o_ptr, flgs);
				if ((o_ptr->name1 == ART_VORPAL_BLADE) || (o_ptr->name1 == ART_CHAINSWORD))
				{
					/* vorpal blade */
					basedam *= 5;
					basedam /= 3;
				}
				else if (have_flag(flgs, TR_VORPAL))
				{
					/* vorpal flag only */
					basedam *= 11;
					basedam /= 9;
				}
				damage += basedam;
				damage += p_ptr->to_d[i] * 100;
				damage *= p_ptr->num_blow[i];
				total_damage += (damage / 100);
			}
			project(0, (cave_have_flag_bold(y, x, FF_PROJECT) ? 5 : 0), y, x, total_damage * 3 / 2, GF_METEOR, PROJECT_KILL | PROJECT_JUMP | PROJECT_ITEM, -1);
		}
		break;

	case 30:
		if (name) return _("慶雲鬼忍剣", "Keiun-Kininken");
		if (desc) return _("自分もダメージをくらうが、相手に非常に大きなダメージを与える。アンデッドには特に効果がある。", 
			"Attacks a monster with extremely powerful damage. But you also takes some damages. Hurts a undead monster greatly.");
    
		if (cast)
		{
			int y, x;

			if (!get_rep_dir2(&dir)) return NULL;
			if (dir == 5) return NULL;

			y = p_ptr->y + ddy[dir];
			x = p_ptr->x + ddx[dir];

			if (cave[y][x].m_idx)
				py_attack(y, x, HISSATSU_UNDEAD);
			else
			{
				msg_print(_("その方向にはモンスターはいません。", "There is no monster."));
				return NULL;
			}
			take_hit(DAMAGE_NOESCAPE, 100 + randint1(100), _("慶雲鬼忍剣を使った衝撃", "exhaustion on using Keiun-Kininken"), -1);
		}
		break;

	case 31:
		if (name) return _("切腹", "Harakiri");
		if (desc) return _("「武士道とは、死ぬことと見つけたり。」", "'Busido is found in death'");

		if (cast)
		{
			int i;
			if (!get_check(_("本当に自殺しますか？", "Do you really want to commit suicide? "))) return NULL;
				/* Special Verification for suicide */
			prt(_("確認のため '@' を押して下さい。", "Please verify SUICIDE by typing the '@' sign: "), 0, 0);
	
			flush();
			i = inkey();
			prt("", 0, 0);
			if (i != '@') return NULL;
			if (p_ptr->total_winner)
			{
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
				p_ptr->total_winner = TRUE;
			}
			else
			{
				msg_print(_("武士道とは、死ぬことと見つけたり。", "Meaning of Bushi-do is found in the death."));
				take_hit(DAMAGE_FORCE, 9999, "Seppuku", -1);
			}
		}
		break;
	}

	return "";
}

/*!
 * @brief 呪術領域の武器呪縛の対象にできる武器かどうかを返す。 / An "item_tester_hook" for offer
 * @param o_ptr オブジェクト構造体の参照ポインタ
 * @return 呪縛可能な武器ならばTRUEを返す
 */
static bool item_tester_hook_weapon_except_bow(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}

/*!
 * @brief 呪術領域の各処理に使える呪われた装備かどうかを返す。 / An "item_tester_hook" for offer
 * @param o_ptr オブジェクト構造体の参照ポインタ
 * @return 使える装備ならばTRUEを返す
 */
static bool item_tester_hook_cursed(object_type *o_ptr)
{
	return (bool)(object_is_cursed(o_ptr));
}

/*!
 * @brief 呪術領域魔法の各処理を行う
 * @param spell 魔法ID
 * @param mode 処理内容 (SPELL_NAME / SPELL_DESC / SPELL_INFO / SPELL_CAST / SPELL_CONT / SPELL_STOP)
 * @return SPELL_NAME / SPELL_DESC / SPELL_INFO 時には文字列ポインタを返す。SPELL_CAST / SPELL_CONT / SPELL_STOP 時はNULL文字列を返す。
 */
static cptr do_hex_spell(SPELL_IDX spell, BIT_FLAGS mode)
{
	bool name = (mode == SPELL_NAME) ? TRUE : FALSE;
	bool desc = (mode == SPELL_DESC) ? TRUE : FALSE;
	bool info = (mode == SPELL_INFO) ? TRUE : FALSE;
	bool cast = (mode == SPELL_CAST) ? TRUE : FALSE;
	bool cont = (mode == SPELL_CONT) ? TRUE : FALSE;
	bool stop = (mode == SPELL_STOP) ? TRUE : FALSE;

	bool add = TRUE;

	PLAYER_LEVEL plev = p_ptr->lev;
	HIT_POINT power;

	switch (spell)
	{
	/*** 1st book (0-7) ***/
	case 0:
		if (name) return _("邪なる祝福", "Evily blessing");
		if (desc) return _("祝福により攻撃精度と防御力が上がる。", "Attempts to increase +to_hit of a weapon and AC");
		if (cast)
		{
			if (!p_ptr->blessed)
			{
				msg_print(_("高潔な気分になった！", "You feel righteous!"));
			}
		}
		if (stop)
		{
			if (!p_ptr->blessed)
			{
				msg_print(_("高潔な気分が消え失せた。", "The prayer has expired."));
			}
		}
		break;

	case 1:
		if (name) return _("軽傷の治癒", "Cure light wounds");
		if (desc) return _("HPや傷を少し回復させる。", "Heals cut and HP a little.");
		if (info) return info_heal(1, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(1, 10));
			set_cut(p_ptr->cut - 10);
		}
		break;

	case 2:
		if (name) return _("悪魔のオーラ", "Demonic aura");
		if (desc) return _("炎のオーラを身にまとい、回復速度が速くなる。", "Gives fire aura and regeneration.");
		if (cast)
		{
			msg_print(_("体が炎のオーラで覆われた。", "You have enveloped by fiery aura!"));
		}
		if (stop)
		{
			msg_print(_("炎のオーラが消え去った。", "Fiery aura disappeared."));
		}
		break;

	case 3:
		if (name) return _("悪臭霧", "Stinking mist");
		if (desc) return _("視界内のモンスターに微弱量の毒のダメージを与える。", "Deals few damages of poison to all monsters in your sight.");
		power = plev / 2 + 5;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_POIS, randint1(power));
		}
		break;

	case 4:
		if (name) return _("腕力強化", "Extra might");
		if (desc) return _("術者の腕力を上昇させる。", "Attempts to increase your strength.");
		if (cast)
		{
			msg_print(_("何だか力が湧いて来る。", "You feel you get stronger."));
		}
		break;

	case 5:
		if (name) return _("武器呪縛", "Curse weapon");
		if (desc) return _("装備している武器を呪う。", "Curses your weapon.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr q, s;
			char o_name[MAX_NLEN];
			object_type *o_ptr;
			u32b f[TR_FLAG_SIZE];

			item_tester_hook = item_tester_hook_weapon_except_bow;
			q = _("どれを呪いますか？", "Which weapon do you curse?");
			s = _("武器を装備していない。", "You wield no weapons.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_desc(o_name, o_ptr, OD_NAME_ONLY);
			object_flags(o_ptr, f);

			if (!get_check(format(_("本当に %s を呪いますか？", "Do you curse %s, really？"), o_name))) return FALSE;

			if (!one_in_(3) &&
				(object_is_artifact(o_ptr) || have_flag(f, TR_BLESSED)))
			{
				msg_format(_("%s は呪いを跳ね返した。", "%s resists the effect."), o_name);
				if (one_in_(3))
				{
					if (o_ptr->to_d > 0)
					{
						o_ptr->to_d -= randint1(3) % 2;
						if (o_ptr->to_d < 0) o_ptr->to_d = 0;
					}
					if (o_ptr->to_h > 0)
					{
						o_ptr->to_h -= randint1(3) % 2;
						if (o_ptr->to_h < 0) o_ptr->to_h = 0;
					}
					if (o_ptr->to_a > 0)
					{
						o_ptr->to_a -= randint1(3) % 2;
						if (o_ptr->to_a < 0) o_ptr->to_a = 0;
					}
					msg_format(_("%s は劣化してしまった。", "Your %s was disenchanted!"), o_name);
				}
			}
			else
			{
				int curse_rank = 0;
				msg_format(_("恐怖の暗黒オーラがあなたの%sを包み込んだ！", "A terrible black aura blasts your %s!"), o_name);
				o_ptr->curse_flags |= (TRC_CURSED);

				if (object_is_artifact(o_ptr) || object_is_ego(o_ptr))
				{

					if (one_in_(3)) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
					if (one_in_(666))
					{
						o_ptr->curse_flags |= (TRC_TY_CURSE);
						if (one_in_(666)) o_ptr->curse_flags |= (TRC_PERMA_CURSE);

						add_flag(o_ptr->art_flags, TR_AGGRAVATE);
						add_flag(o_ptr->art_flags, TR_VORPAL);
						add_flag(o_ptr->art_flags, TR_VAMPIRIC);
						msg_print(_("血だ！血だ！血だ！", "Blood, Blood, Blood!"));
						curse_rank = 2;
					}
				}

				o_ptr->curse_flags |= get_curse(curse_rank, o_ptr);
			}

			p_ptr->update |= (PU_BONUS);
			add = FALSE;
		}
		break;

	case 6:
		if (name) return _("邪悪感知", "Evil detection");
		if (desc) return _("周囲の邪悪なモンスターを感知する。", "Detects evil monsters.");
		if (info) return info_range(MAX_SIGHT);
		if (cast)
		{
			msg_print(_("邪悪な生物の存在を感じ取ろうとした。", "You attend to the presence of evil creatures."));
		}
		break;

	case 7:
		if (name) return _("我慢", "Patience");
		if (desc) return _("数ターン攻撃を耐えた後、受けたダメージを地獄の業火として周囲に放出する。", 
			"Bursts hell fire strongly after patients any damage while few turns.");
		power = MIN(200, (HEX_REVENGE_POWER(p_ptr) * 2));
		if (info) return info_damage(0, 0, power);
		if (cast)
		{
			int a = 3 - (p_ptr->pspeed - 100) / 10;
			MAGIC_NUM2 r = 3 + randint1(3) + MAX(0, MIN(3, a));

			if (HEX_REVENGE_TURN(p_ptr) > 0)
			{
				msg_print(_("すでに我慢をしている。", "You are already patienting."));
				return NULL;
			}

			HEX_REVENGE_TYPE(p_ptr) = 1;
			HEX_REVENGE_TURN(p_ptr) = r;
			HEX_REVENGE_POWER(p_ptr) = 0;
			msg_print(_("じっと耐えることにした。", "You decide to patient all damages."));
			add = FALSE;
		}
		if (cont)
		{
			int rad = 2 + (power / 50);

			HEX_REVENGE_TURN(p_ptr)--;

			if ((HEX_REVENGE_TURN(p_ptr) <= 0) || (power >= 200))
			{
				msg_print(_("我慢が解かれた！", "Time for end of patioence!"));
				if (power)
				{
					project(0, rad, p_ptr->y, p_ptr->x, power, GF_HELL_FIRE,
						(PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL), -1);
				}
				if (p_ptr->wizard)
				{
					msg_format(_("%d点のダメージを返した。", "You return %d damages."), power);
				}

				/* Reset */
				HEX_REVENGE_TYPE(p_ptr) = 0;
				HEX_REVENGE_TURN(p_ptr) = 0;
				HEX_REVENGE_POWER(p_ptr) = 0;
			}
		}
		break;

	/*** 2nd book (8-15) ***/
	case 8:
		if (name) return _("氷の鎧", "Ice armor");
		if (desc) return _("氷のオーラを身にまとい、防御力が上昇する。", "Gives fire aura and bonus to AC.");
		if (cast)
		{
			msg_print(_("体が氷の鎧で覆われた。", "You have enveloped by ice armor!"));
		}
		if (stop)
		{
			msg_print(_("氷の鎧が消え去った。", "Ice armor disappeared."));
		}
		break;

	case 9:
		if (name) return _("重傷の治癒", "Cure serious wounds");
		if (desc) return _("体力や傷を多少回復させる。", "Heals cut and HP more.");
		if (info) return info_heal(2, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(2, 10));
			set_cut((p_ptr->cut / 2) - 10);
		}
		break;

	case 10:
		if (name) return _("薬品吸入", "Inhail potion");
		if (desc) return _("呪文詠唱を中止することなく、薬の効果を得ることができる。", "Quaffs a potion without canceling of casting a spell.");
		if (cast)
		{
			CASTING_HEX_FLAGS(p_ptr) |= (1L << HEX_INHAIL);
			do_cmd_quaff_potion();
			CASTING_HEX_FLAGS(p_ptr) &= ~(1L << HEX_INHAIL);
			add = FALSE;
		}
		break;

	case 11:
		if (name) return _("衰弱の霧", "Hypodynamic mist");
		if (desc) return _("視界内のモンスターに微弱量の衰弱属性のダメージを与える。", 
			"Deals few damages of hypodynamia to all monsters in your sight.");
		power = (plev / 2) + 5;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_HYPODYNAMIA, randint1(power));
		}
		break;

	case 12:
		if (name) return _("魔剣化", "Swords to runeswords");
		if (desc) return _("武器の攻撃力を上げる。切れ味を得、呪いに応じて与えるダメージが上昇し、善良なモンスターに対するダメージが2倍になる。", 
			"Gives vorpal ability to your weapon. Increases damages by your weapon acccording to curse of your weapon.");
		if (cast)
		{
#ifdef JP
			msg_print("あなたの武器が黒く輝いた。");
#else
			if (!empty_hands(FALSE))
				msg_print("Your weapons glow bright black.");
			else
				msg_print("Your weapon glows bright black.");
#endif
		}
		if (stop)
		{
#ifdef JP
			msg_print("武器の輝きが消え去った。");
#else
			msg_format("Brightness of weapon%s disappeared.", (empty_hands(FALSE)) ? "" : "s");
#endif
		}
		break;

	case 13:
		if (name) return _("混乱の手", "Touch of confusion");
		if (desc) return _("攻撃した際モンスターを混乱させる。", "Confuses a monster when you attack.");
		if (cast)
		{
			msg_print(_("あなたの手が赤く輝き始めた。", "Your hands glow bright red."));
		}
		if (stop)
		{
			msg_print(_("手の輝きがなくなった。", "Brightness on your hands disappeard."));
		}
		break;

	case 14:
		if (name) return _("肉体強化", "Building up");
		if (desc) return _("術者の腕力、器用さ、耐久力を上昇させる。攻撃回数の上限を 1 増加させる。", 
			"Attempts to increases your strength, dexterity and constitusion.");
		if (cast)
		{
			msg_print(_("身体が強くなった気がした。", "You feel your body is developed more now."));
		}
		break;

	case 15:
		if (name) return _("反テレポート結界", "Anti teleport barrier");
		if (desc) return _("視界内のモンスターのテレポートを阻害するバリアを張る。", "Obstructs all teleportations by monsters in your sight.");
		power = plev * 3 / 2;
		if (info) return info_power(power);
		if (cast)
		{
			msg_print(_("テレポートを防ぐ呪いをかけた。", "You feel anyone can not teleport except you."));
		}
		break;

	/*** 3rd book (16-23) ***/
	case 16:
		if (name) return _("衝撃のクローク", "Cloak of shock");
		if (desc) return _("電気のオーラを身にまとい、動きが速くなる。", "Gives lightning aura and a bonus to speed.");
		if (cast)
		{
			msg_print(_("体が稲妻のオーラで覆われた。", "You have enveloped by electrical aura!"));
		}
		if (stop)
		{
			msg_print(_("稲妻のオーラが消え去った。", "Electrical aura disappeared."));
		}
		break;

	case 17:
		if (name) return _("致命傷の治癒", "Cure critical wounds");
		if (desc) return _("体力や傷を回復させる。", "Heals cut and HP greatry.");
		if (info) return info_heal(4, 10, 0);
		if (cast)
		{
			msg_print(_("気分が良くなってくる。", "You feel better and better."));
		}
		if (cast || cont)
		{
			hp_player(damroll(4, 10));
			set_stun(0);
			set_cut(0);
			set_poisoned(0);
		}
		break;

	case 18:
		if (name) return _("呪力封入", "Recharging");
		if (desc) return _("魔法の道具に魔力を再充填する。", "Recharges a magic device.");
		power = plev * 2;
		if (info) return info_power(power);
		if (cast)
		{
			if (!recharge(power)) return NULL;
			add = FALSE;
		}
		break;

	case 19:
		if (name) return _("死者復活", "Animate Dead");
		if (desc) return _("死体を蘇らせてペットにする。", "Raises corpses and skeletons from dead.");
		if (cast)
		{
			msg_print(_("死者への呼びかけを始めた。", "You start to call deads.!"));
		}
		if (cast || cont)
		{
			animate_dead(0, p_ptr->y, p_ptr->x);
		}
		break;

	case 20:
		if (name) return _("防具呪縛", "Curse armor");
		if (desc) return _("装備している防具に呪いをかける。", "Curse a piece of armour that you wielding.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr q, s;
			char o_name[MAX_NLEN];
			object_type *o_ptr;
			u32b f[TR_FLAG_SIZE];

			item_tester_hook = object_is_armour;
			q = _("どれを呪いますか？", "Which piece of armour do you curse?");
			s = _("防具を装備していない。", "You wield no piece of armours.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_desc(o_name, o_ptr, OD_NAME_ONLY);
			object_flags(o_ptr, f);

			if (!get_check(format(_("本当に %s を呪いますか？", "Do you curse %s, really？"), o_name))) return FALSE;

			if (!one_in_(3) &&
				(object_is_artifact(o_ptr) || have_flag(f, TR_BLESSED)))
			{
				msg_format(_("%s は呪いを跳ね返した。", "%s resists the effect."), o_name);
				if (one_in_(3))
				{
					if (o_ptr->to_d > 0)
					{
						o_ptr->to_d -= randint1(3) % 2;
						if (o_ptr->to_d < 0) o_ptr->to_d = 0;
					}
					if (o_ptr->to_h > 0)
					{
						o_ptr->to_h -= randint1(3) % 2;
						if (o_ptr->to_h < 0) o_ptr->to_h = 0;
					}
					if (o_ptr->to_a > 0)
					{
						o_ptr->to_a -= randint1(3) % 2;
						if (o_ptr->to_a < 0) o_ptr->to_a = 0;
					}
					msg_format(_("%s は劣化してしまった。", "Your %s was disenchanted!"), o_name);
				}
			}
			else
			{
				int curse_rank = 0;
				msg_format(_("恐怖の暗黒オーラがあなたの%sを包み込んだ！", "A terrible black aura blasts your %s!"), o_name);
				o_ptr->curse_flags |= (TRC_CURSED);

				if (object_is_artifact(o_ptr) || object_is_ego(o_ptr))
				{

					if (one_in_(3)) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
					if (one_in_(666))
					{
						o_ptr->curse_flags |= (TRC_TY_CURSE);
						if (one_in_(666)) o_ptr->curse_flags |= (TRC_PERMA_CURSE);

						add_flag(o_ptr->art_flags, TR_AGGRAVATE);
						add_flag(o_ptr->art_flags, TR_RES_POIS);
						add_flag(o_ptr->art_flags, TR_RES_DARK);
						add_flag(o_ptr->art_flags, TR_RES_NETHER);
						msg_print(_("血だ！血だ！血だ！", "Blood, Blood, Blood!"));
						curse_rank = 2;
					}
				}

				o_ptr->curse_flags |= get_curse(curse_rank, o_ptr);
			}

			p_ptr->update |= (PU_BONUS);
			add = FALSE;
		}
		break;

	case 21:
		if (name) return _("影のクローク", "Cloak of shadow");
		if (desc) return _("影のオーラを身にまとい、敵に影のダメージを与える。", "Gives aura of shadow.");
		if (cast)
		{
			object_type *o_ptr = &inventory[INVEN_OUTER];

			if (!o_ptr->k_idx)
			{
				msg_print(_("クロークを身につけていない！", "You don't ware any cloak."));
				return NULL;
			}
			else if (!object_is_cursed(o_ptr))
			{
				msg_print(_("クロークは呪われていない！", "Your cloak is not cursed."));
				return NULL;
			}
			else
			{
				msg_print(_("影のオーラを身にまとった。", "You have enveloped by shadow aura!"));
			}
		}
		if (cont)
		{
			object_type *o_ptr = &inventory[INVEN_OUTER];

			if ((!o_ptr->k_idx) || (!object_is_cursed(o_ptr)))
			{
				do_spell(REALM_HEX, spell, SPELL_STOP);
				CASTING_HEX_FLAGS(p_ptr) &= ~(1L << spell);
				CASTING_HEX_NUM(p_ptr)--;
				if (!SINGING_SONG_ID(p_ptr)) set_action(ACTION_NONE);
			}
		}
		if (stop)
		{
			msg_print(_("影のオーラが消え去った。", "Shadow aura disappeared."));
		}
		break;

	case 22:
		if (name) return _("苦痛を魔力に", "Pains to mana");
		if (desc) return _("視界内のモンスターに精神ダメージ与え、魔力を吸い取る。", "Deals psychic damages to all monsters in sight, and drains some mana.");
		power = plev * 3 / 2;
		if (info) return info_damage(1, power, 0);
		if (cast || cont)
		{
			project_hack(GF_PSI_DRAIN, randint1(power));
		}
		break;

	case 23:
		if (name) return _("目には目を", "Eye for an eye");
		if (desc) return _("打撃や魔法で受けたダメージを、攻撃元のモンスターにも与える。", "Returns same damage which you got to the monster which damaged you.");
		if (cast)
		{
			msg_print(_("復讐したい欲望にかられた。", "You wish strongly you want to revenge anything."));
		}
		break;

	/*** 4th book (24-31) ***/
	case 24:
		if (name) return _("反増殖結界", "Anti multiply barrier");
		if (desc) return _("その階の増殖するモンスターの増殖を阻止する。", "Obstructs all multiplying by monsters in entire floor.");
		if (cast)
		{
			msg_print(_("増殖を阻止する呪いをかけた。", "You feel anyone can not already multiply."));
		}
		break;

	case 25:
		if (name) return _("全復活", "Restoration");
		if (desc) return _("経験値を徐々に復活し、減少した能力値を回復させる。", "Restores experience and status.");
		if (cast)
		{
			msg_print(_("体が元の活力を取り戻し始めた。", "You feel your lost status starting to return."));
		}
		if (cast || cont)
		{
			bool flag = FALSE;
			int d = (p_ptr->max_exp - p_ptr->exp);
			int r = (p_ptr->exp / 20);
			int i;

			if (d > 0)
			{
				if (d < r)
					p_ptr->exp = p_ptr->max_exp;
				else
					p_ptr->exp += r;

				/* Check the experience */
				check_experience();

				flag = TRUE;
			}
			for (i = A_STR; i < 6; i ++)
			{
				if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
				{
					if (p_ptr->stat_cur[i] < 18)
						p_ptr->stat_cur[i]++;
					else
						p_ptr->stat_cur[i] += 10;

					if (p_ptr->stat_cur[i] > p_ptr->stat_max[i])
						p_ptr->stat_cur[i] = p_ptr->stat_max[i];

					/* Recalculate bonuses */
					p_ptr->update |= (PU_BONUS);

					flag = TRUE;
				}
			}

			if (!flag)
			{
				msg_format(_("%sの呪文の詠唱をやめた。", "Finish casting '%^s'."), do_spell(REALM_HEX, HEX_RESTORE, SPELL_NAME));
				CASTING_HEX_FLAGS(p_ptr) &= ~(1L << HEX_RESTORE);
				if (cont) CASTING_HEX_NUM(p_ptr)--;
				if (CASTING_HEX_NUM(p_ptr)) p_ptr->action = ACTION_NONE;

				/* Redraw status */
				p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
				p_ptr->redraw |= (PR_EXTRA);

				return "";
			}
		}
		break;

	case 26:
		if (name) return _("呪力吸収", "Drain curse power");
		if (desc) return _("呪われた武器の呪いを吸収して魔力を回復する。", "Drains curse on your weapon and heals SP a little.");
		if (cast)
		{
			OBJECT_IDX item;
			cptr s, q;
			u32b f[TR_FLAG_SIZE];
			object_type *o_ptr;

			item_tester_hook = item_tester_hook_cursed;
			q = _("どの装備品から吸収しますか？", "Which cursed equipment do you drain mana from?");
			s = _("呪われたアイテムを装備していない。", "You have no cursed equipment.");

			if (!get_item(&item, q, s, (USE_EQUIP))) return FALSE;

			o_ptr = &inventory[item];
			object_flags(o_ptr, f);

			p_ptr->csp += (p_ptr->lev / 5) + randint1(p_ptr->lev / 5);
			if (have_flag(f, TR_TY_CURSE) || (o_ptr->curse_flags & TRC_TY_CURSE)) p_ptr->csp += randint1(5);
			if (p_ptr->csp > p_ptr->msp) p_ptr->csp = p_ptr->msp;

			if (o_ptr->curse_flags & TRC_PERMA_CURSE)
			{
				/* Nothing */
			}
			else if (o_ptr->curse_flags & TRC_HEAVY_CURSE)
			{
				if (one_in_(7))
				{
					msg_print(_("呪いを全て吸い取った。", "Heavy curse vanished away."));
					o_ptr->curse_flags = 0L;
				}
			}
			else if ((o_ptr->curse_flags & (TRC_CURSED)) && one_in_(3))
			{
				msg_print(_("呪いを全て吸い取った。", "Curse vanished away."));
				o_ptr->curse_flags = 0L;
			}

			add = FALSE;
		}
		break;

	case 27:
		if (name) return _("吸血の刃", "Swords to vampires");
		if (desc) return _("吸血属性で攻撃する。", "Gives vampiric ability to your weapon.");
		if (cast)
		{
#ifdef JP
			msg_print("あなたの武器が血を欲している。");
#else
			if (!empty_hands(FALSE))
				msg_print("Your weapons want more blood now.");
			else
				msg_print("Your weapon wants more blood now.");
#endif
		}
		if (stop)
		{
#ifdef JP
			msg_print("武器の渇望が消え去った。");
#else
			msg_format("Thirsty of weapon%s disappeared.", (empty_hands(FALSE)) ? "" : "s");
#endif
		}
		break;

	case 28:
		if (name) return _("朦朧の言葉", "Word of stun");
		if (desc) return _("視界内のモンスターを朦朧とさせる。", "Stuns all monsters in your sight.");
		power = plev * 4;
		if (info) return info_power(power);
		if (cast || cont)
		{
			stun_monsters(power);
		}
		break;

	case 29:
		if (name) return _("影移動", "Moving into shadow");
		if (desc) return _("モンスターの隣のマスに瞬間移動する。", "Teleports you close to a monster.");
		if (cast)
		{
			int i, dir;
			POSITION y, x;
			bool flag;

			for (i = 0; i < 3; i++)
			{
				if (!tgt_pt(&x, &y)) return FALSE;

				flag = FALSE;

				for (dir = 0; dir < 8; dir++)
				{
					int dy = y + ddy_ddd[dir];
					int dx = x + ddx_ddd[dir];
					if (dir == 5) continue;
					if(cave[dy][dx].m_idx) flag = TRUE;
				}

				if (!cave_empty_bold(y, x) || (cave[y][x].info & CAVE_ICKY) ||
					(distance(y, x, p_ptr->y, p_ptr->x) > plev + 2))
				{
					msg_print(_("そこには移動できない。", "Can not teleport to there."));
					continue;
				}
				break;
			}

			if (flag && randint0(plev * plev / 2))
			{
				teleport_player_to(y, x, 0L);
			}
			else
			{
				msg_print(_("おっと！", "Oops!"));
				teleport_player(30, 0L);
			}

			add = FALSE;
		}
		break;

	case 30:
		if (name) return _("反魔法結界", "Anti magic barrier");
		if (desc) return _("視界内のモンスターの魔法を阻害するバリアを張る。", "Obstructs all magic spell of monsters in your sight.");
		power = plev * 3 / 2;
		if (info) return info_power(power);
		if (cast)
		{
			msg_print(_("魔法を防ぐ呪いをかけた。", "You feel anyone can not cast spells except you."));
		}
		break;

	case 31:
		if (name) return _("復讐の宣告", "Revenge sentence");
		if (desc) return _("数ターン後にそれまで受けたダメージに応じた威力の地獄の劫火の弾を放つ。", 
			"Fires  a ball of hell fire to try revenging after few turns.");
		power = HEX_REVENGE_POWER(p_ptr);
		if (info) return info_damage(0, 0, power);
		if (cast)
		{
			MAGIC_NUM2 r;
			int a = 3 - (p_ptr->pspeed - 100) / 10;
			r = 1 + randint1(2) + MAX(0, MIN(3, a));

			if (HEX_REVENGE_TURN(p_ptr) > 0)
			{
				msg_print(_("すでに復讐は宣告済みだ。", "You already pronounced your revenge."));
				return NULL;
			}

			HEX_REVENGE_TYPE(p_ptr) = 2;
			HEX_REVENGE_TURN(p_ptr) = r;
			msg_format(_("あなたは復讐を宣告した。あと %d ターン。", "You pronounce your revenge. %d turns left."), r);
			add = FALSE;
		}
		if (cont)
		{
			HEX_REVENGE_TURN(p_ptr)--;

			if (HEX_REVENGE_TURN(p_ptr) <= 0)
			{
				int dir;

				if (power)
				{
					command_dir = 0;

					do
					{
						msg_print(_("復讐の時だ！", "Time to revenge!"));
					}
					while (!get_aim_dir(&dir));

					fire_ball(GF_HELL_FIRE, dir, power, 1);

					if (p_ptr->wizard)
					{
						msg_format(_("%d点のダメージを返した。", "You return %d damages."), power);
					}
				}
				else
				{
					msg_print(_("復讐する気が失せた。", "You are not a mood to revenge."));
				}
				HEX_REVENGE_POWER(p_ptr) = 0;
			}
		}
		break;
	}

	/* start casting */
	if ((cast) && (add))
	{
		/* add spell */
		CASTING_HEX_FLAGS(p_ptr) |= 1L << (spell);
		CASTING_HEX_NUM(p_ptr)++;

		if (p_ptr->action != ACTION_SPELL) set_action(ACTION_SPELL);
	}

	/* Redraw status */
	if (!info)
	{
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
		p_ptr->redraw |= (PR_EXTRA | PR_HP | PR_MANA);
	}

	return "";
}


/*!
 * @brief 魔法処理のメインルーチン
 * @param realm 魔法領域のID
 * @param spell 各領域の魔法ID
 * @param mode 求める処理
 * @return 各領域魔法に各種テキストを求めた場合は文字列参照ポインタ、そうでない場合はNULLポインタを返す。
 */
cptr do_spell(REALM_IDX realm, SPELL_IDX spell, BIT_FLAGS mode)
{
	switch (realm)
	{
	case REALM_LIFE:     return do_life_spell(spell, mode);
	case REALM_SORCERY:  return do_sorcery_spell(spell, mode);
	case REALM_NATURE:   return do_nature_spell(spell, mode);
	case REALM_CHAOS:    return do_chaos_spell(spell, mode);
	case REALM_DEATH:    return do_death_spell(spell, mode);
	case REALM_TRUMP:    return do_trump_spell(spell, mode);
	case REALM_ARCANE:   return do_arcane_spell(spell, mode);
	case REALM_CRAFT:    return do_craft_spell(spell, mode);
	case REALM_DAEMON:   return do_daemon_spell(spell, mode);
	case REALM_CRUSADE:  return do_crusade_spell(spell, mode);
	case REALM_MUSIC:    return do_music_spell(spell, mode);
	case REALM_HISSATSU: return do_hissatsu_spell(spell, mode);
	case REALM_HEX:      return do_hex_spell(spell, mode);
	}

	return NULL;
}
