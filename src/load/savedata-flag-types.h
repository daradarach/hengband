﻿#pragma once

enum savedata_item_flag_type {
	SAVE_ITEM_PVAL = 0x00000001,
	SAVE_ITEM_DISCOUNT = 0x00000002,
	SAVE_ITEM_NUMBER = 0x00000004,
	SAVE_ITEM_NAME1 = 0x00000008,
	SAVE_ITEM_NAME2 = 0x00000010,
	SAVE_ITEM_TIMEOUT = 0x00000020,
	SAVE_ITEM_TO_H = 0x00000040,
	SAVE_ITEM_TO_D = 0x00000080,
	SAVE_ITEM_TO_A = 0x00000100,
	SAVE_ITEM_AC = 0x00000200,
	SAVE_ITEM_DD = 0x00000400,
	SAVE_ITEM_DS = 0x00000800,
	SAVE_ITEM_IDENT = 0x00001000,
	SAVE_ITEM_MARKED = 0x00002000,
	SAVE_ITEM_SMITH = 0x00004000,
	SAVE_ITEM_00008000 = 0x00008000, //<! 未使用
	SAVE_ITEM_00010000 = 0x00010000, //<! 未使用
	SAVE_ITEM_00020000 = 0x00020000, //<! 未使用
	SAVE_ITEM_CURSE_FLAGS = 0x00040000,
	SAVE_ITEM_HELD_M_IDX = 0x00080000,
	SAVE_ITEM_XTRA1 = 0x00100000,
	SAVE_ITEM_ACTIVATION_ID = 0x00200000,
	SAVE_ITEM_XTRA3 = 0x00400000,
	SAVE_ITEM_XTRA4 = 0x00800000,
	SAVE_ITEM_XTRA5 = 0x01000000,
	SAVE_ITEM_FEELING = 0x02000000,
	SAVE_ITEM_INSCRIPTION = 0x04000000,
	SAVE_ITEM_ART_NAME = 0x08000000,
	SAVE_ITEM_ART_FLAGS = 0x10000000,
	SAVE_ITEM_STACK_IDX = 0x20000000,
};

enum savedata_monster_flag_type {
	SAVE_MON_AP_R_IDX = 0x00000001,
	SAVE_MON_SUB_ALIGN = 0x00000002,
	SAVE_MON_CSLEEP = 0x00000004,
	SAVE_MON_FAST = 0x00000008,
	SAVE_MON_SLOW = 0x00000010,
	SAVE_MON_STUNNED = 0x00000020,
	SAVE_MON_CONFUSED = 0x00000040,
	SAVE_MON_MONFEAR = 0x00000080,
	SAVE_MON_TARGET_Y = 0x00000100,
	SAVE_MON_TARGET_X = 0x00000200,
	SAVE_MON_INVULNER = 0x00000400,
	SAVE_MON_SMART = 0x00000800,
	SAVE_MON_EXP = 0x00001000,
	SAVE_MON_MFLAG2 = 0x00002000,
	SAVE_MON_NICKNAME = 0x00004000,
	SAVE_MON_PARENT = 0x00008000,
};
