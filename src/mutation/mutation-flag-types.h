#pragma once

enum class PlayerMutationType {
    SPIT_ACID = 0, /*!< 突然変異: 酸の唾 */
    BR_FIRE = 1, /*!< 突然変異: 炎のブレス */
    HYPN_GAZE = 2, /*!< 突然変異: 催眠睨み */
    TELEKINES = 3, /*!< 突然変異: 念動力 */
    VTELEPORT = 4, /*!< 突然変異: テレポート / Voluntary teleport */
    MIND_BLST = 5, /*!< 突然変異: 精神攻撃 */
    RADIATION = 6, /*!< 突然変異: 放射能 */
    VAMPIRISM = 7, /*!< 突然変異: 吸血 */
    SMELL_MET = 8, /*!< 突然変異: 金属嗅覚 */
    SMELL_MON = 9, /*!< 突然変異: 敵臭嗅覚 */
    BLINK = 10, /*!< 突然変異: ショート・テレポート */
    EAT_ROCK = 11, /*!< 突然変異: 岩喰い */
    SWAP_POS = 12, /*!< 突然変異: 位置交換 */
    SHRIEK = 13, /*!< 突然変異: 叫び */
    ILLUMINE = 14, /*!< 突然変異: 照明 */
    DET_CURSE = 15, /*!< 突然変異: 呪い感知 */
    BERSERK = 16, /*!< 突然変異: 狂戦士化 */
    POLYMORPH = 17, /*!< 突然変異: 変身 */
    MIDAS_TCH = 18, /*!< 突然変異: ミダスの手 */
    GROW_MOLD = 19, /*!< 突然変異: カビ発生 */
    RESIST = 20, /*!< 突然変異: エレメント耐性 */
    EARTHQUAKE = 21, /*!< 突然変異: 地震 */
    EAT_MAGIC = 22, /*!< 突然変異: 魔力喰い */
    WEIGH_MAG = 23, /*!< 突然変異: 魔力感知 */
    STERILITY = 24, /*!< 突然変異: 増殖阻止 */
    HIT_AND_AWAY = 25, /*!< 突然変異: ヒットアンドアウェイ */
    DAZZLE = 26, /*!< 突然変異: 眩惑 */
    LASER_EYE = 27, /*!< 突然変異: レーザー・アイ */
    RECALL = 28, /*!< 突然変異: 帰還 */
    BANISH = 29, /*!< 突然変異: 邪悪消滅 */
    COLD_TOUCH = 30, /*!< 突然変異: 凍結の手 */
    LAUNCHER = 31, /*!< 突然変異: アイテム投げ */

    BERS_RAGE = 32, /*!< 突然変異: 狂戦士化の発作 */
    COWARDICE = 33, /*!< 突然変異: 臆病 */
    RTELEPORT = 34, /*!< 突然変異: ランダムテレポート / Random teleport, instability */
    ALCOHOL = 35, /*!< 突然変異: アルコール分泌 */
    HALLU = 36, /*!< 突然変異: 幻覚を引き起こす精神錯乱 */
    FLATULENT = 37, /*!< 突然変異: 猛烈な屁 */
    SCOR_TAIL = 38, /*!< 突然変異: サソリの尻尾 */
    HORNS = 39, /*!< 突然変異: ツノ */
    BEAK = 40, /*!< 突然変異: クチバシ */
    ATT_DEMON = 41, /*!< 突然変異: デーモンを引き付ける */
    PROD_MANA = 42, /*!< 突然変異: 制御できない魔力のエネルギー */
    SPEED_FLUX = 43, /*!< 突然変異: ランダムな加減速 */
    BANISH_ALL = 44, /*!< 突然変異: ランダムなモンスター消滅 */
    EAT_LIGHT = 45, /*!< 突然変異: 光源喰い */
    TRUNK = 46, /*!< 突然変異: 象の鼻 */
    ATT_ANIMAL = 47, /*!< 突然変異: 動物を引き寄せる */
    TENTACLES = 48, /*!< 突然変異: 邪悪な触手 */
    RAW_CHAOS = 49, /*!< 突然変異: 純カオス */
    NORMALITY = 50, /*!< 突然変異: ランダムな変異の消滅 */
    WRAITH = 51, /*!< 突然変異: ランダムな幽体化 */
    POLY_WOUND = 52, /*!< 突然変異: ランダムな傷の変化 */
    WASTING = 53, /*!< 突然変異: 衰弱 */
    ATT_DRAGON = 54, /*!< 突然変異: ドラゴンを引き寄せる */
    WEIRD_MIND = 55, /*!< 突然変異: ランダムなテレパシー */
    NAUSEA = 56, /*!< 突然変異: 落ち着きの無い胃 */
    CHAOS_GIFT = 57, /*!< 突然変異: カオスパトロン */
    WALK_SHAD = 58, /*!< 突然変異: ランダムな現実変容 */
    WARNING = 59, /*!< 突然変異: 警告 */
    INVULN = 60, /*!< 突然変異: ランダムな無敵化 */
    SP_TO_HP = 61, /*!< 突然変異: ランダムなMPからHPへの変換 */
    HP_TO_SP = 62, /*!< 突然変異: ランダムなHPからMPへの変換 */
    DISARM = 63, /*!< 突然変異: ランダムな武器落とし */

    HYPER_STR = 64, /*!< 突然変異: 超人的な力 */
    PUNY = 65, /*!< 突然変異: 虚弱 */
    HYPER_INT = 66, /*!< 突然変異: 生体コンピュータ */
    MORONIC = 67, /*!< 突然変異: 精神薄弱 */
    RESILIENT = 68, /*!< 突然変異: 弾力のある体 */
    XTRA_FAT = 69, /*!< 突然変異: 異常な肥満 */
    ALBINO = 70, /*!< 突然変異: アルビノ */
    FLESH_ROT = 71, /*!< 突然変異: 腐敗した肉体 */
    SILLY_VOI = 72, /*!< 突然変異: 間抜けなキーキー声 */
    BLANK_FAC = 73, /*!< 突然変異: のっぺらぼう */
    ILL_NORM = 74, /*!< 突然変異: 幻影に覆われた体 */
    XTRA_EYES = 75, /*!< 突然変異: 第三の目 */
    MAGIC_RES = 76, /*!< 突然変異: 魔法防御 */
    XTRA_NOIS = 77, /*!< 突然変異: 騒音 */
    INFRAVIS = 78, /*!< 突然変異: 赤外線視力 */
    XTRA_LEGS = 79, /*!< 突然変異: 追加の脚 */
    SHORT_LEG = 80, /*!< 突然変異: 短い脚 */
    ELEC_TOUC = 81, /*!< 突然変異: 電撃オーラ */
    FIRE_BODY = 82, /*!< 突然変異: 火炎オーラ */
    WART_SKIN = 83, /*!< 突然変異: イボ肌 */
    SCALES = 84, /*!< 突然変異: 鱗肌 */
    IRON_SKIN = 85, /*!< 突然変異: 鉄の肌 */
    WINGS = 86, /*!< 突然変異: 翼 */
    FEARLESS = 87, /*!< 突然変異: 恐れ知らず */
    REGEN = 88, /*!< 突然変異: 急回復 */
    ESP = 89, /*!< 突然変異: テレパシー */
    LIMBER = 90, /*!< 突然変異: しなやかな肉体 */
    ARTHRITIS = 91, /*!< 突然変異: 関節の痛み */
    BAD_LUCK = 92, /*!< 突然変異: 黒いオーラ(不運) */
    VULN_ELEM = 93, /*!< 突然変異: 元素攻撃弱点 */
    MOTION = 94, /*!< 突然変異: 正確で力強い動作 */
    GOOD_LUCK = 95, /*!< 突然変異: 白いオーラ(幸運) */
    MAX,
};
