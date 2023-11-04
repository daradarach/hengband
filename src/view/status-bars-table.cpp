#include "view/status-bars-table.h"
#include "term/term-color-types.h"

stat_bar stat_bars[MAX_STAT_BARS] = { { TERM_YELLOW, _("つ", "Ts"), _("つよし", "Tsuyoshi") }, { TERM_VIOLET, _("幻", "Ha"), _("幻覚", "Halluc") },
    { TERM_L_DARK, _("盲", "Bl"), _("盲目", "Blind") }, { TERM_RED, _("痺", "Pa"), _("麻痺", "Paralyzed") },
    { TERM_VIOLET, _("乱", "Cf"), _("混乱", "Confused") }, { TERM_GREEN, _("毒", "Po"), _("毒", "Poisoned") },
    { TERM_BLUE, _("恐", "Af"), _("恐怖", "Afraid") }, { TERM_L_BLUE, _("浮", "Lv"), _("浮遊", "Levit") }, { TERM_SLATE, _("反", "Rf"), _("反射", "Reflect") },
    { TERM_SLATE, _("壁", "Pw"), _("壁抜け", "PassWall") }, { TERM_L_DARK, _("幽", "Wr"), _("幽体", "Wraith") },
    { TERM_SLATE, _("邪", "Ev"), _("防邪", "PrtEvl") }, { TERM_VIOLET, _("変", "Kw"), _("変わり身", "Kawarimi") },
    { TERM_YELLOW, _("魔", "Md"), _("魔法鎧", "MgcArm") }, { TERM_L_UMBER, _("伸", "Eh"), _("伸び", "Expand") },
    { TERM_WHITE, _("石", "Ss"), _("石肌", "StnSkn") }, { TERM_L_BLUE, _("分", "Ms"), _("分身", "MltShdw") },
    { TERM_SLATE, _("防", "Rm"), _("魔法防御", "ResMag") }, { TERM_YELLOW, _("究", "Ul"), _("究極", "Ultima") },
    { TERM_YELLOW, _("無", "Iv"), _("無敵", "Invuln") }, { TERM_L_GREEN, _("酸", "IAc"), _("酸免疫", "ImmAcid") },
    { TERM_GREEN, _("酸", "Ac"), _("耐酸", "Acid") }, { TERM_L_BLUE, _("電", "IEl"), _("電免疫", "ImmElec") }, { TERM_BLUE, _("電", "El"), _("耐電", "Elec") },
    { TERM_L_RED, _("火", "IFi"), _("火免疫", "ImmFire") }, { TERM_RED, _("火", "Fi"), _("耐火", "Fire") },
    { TERM_WHITE, _("冷", "ICo"), _("冷免疫", "ImmCold") }, { TERM_SLATE, _("冷", "Co"), _("耐冷", "Cold") }, { TERM_GREEN, _("毒", "Po"), _("耐毒", "Pois") },
    { TERM_L_DARK, _("獄", "Nt"), _("耐地獄", "Nthr") }, { TERM_L_BLUE, _("時", "Ti"), _("耐時間", "Time") },
    { TERM_L_DARK, _("鏡", "Mr"), _("鏡オーラ", "Mirr") }, { TERM_L_RED, _("オ", "SFi"), _("火オーラ", "SFire") },
    { TERM_WHITE, _("闘", "Fo"), _("闘気", "Force") }, { TERM_WHITE, _("聖", "Ho"), _("聖オーラ", "Holy") },
    { TERM_VIOLET, _("目", "Ee"), _("目には目", "EyeEye") }, { TERM_WHITE, _("祝", "Bs"), _("祝福", "Bless") }, { TERM_WHITE, _("勇", "He"), _("勇", "Hero") },
    { TERM_RED, _("狂", "Br"), _("狂乱", "Berserk") }, { TERM_L_RED, _("火", "BFi"), _("魔剣火", "BFire") },
    { TERM_WHITE, _("冷", "BCo"), _("魔剣冷", "BCold") }, { TERM_L_BLUE, _("電", "BEl"), _("魔剣電", "BElec") },
    { TERM_SLATE, _("酸", "BAc"), _("魔剣酸", "BAcid") }, { TERM_L_GREEN, _("毒", "BPo"), _("魔剣毒", "BPois") },
    { TERM_RED, _("乱", "TCf"), _("混乱打撃", "TchCnf") }, { TERM_L_BLUE, _("視", "Se"), _("透明視", "SInv") },
    { TERM_ORANGE, _("テ", "Te"), _("テレパシ", "Telepa") }, { TERM_L_BLUE, _("回", "Rg"), _("回復", "Regen") },
    { TERM_L_RED, _("赤", "If"), _("赤外", "Infr") }, { TERM_UMBER, _("隠", "Sl"), _("隠密", "Stealth") },
    { TERM_YELLOW, _("隠", "Stlt"), _("超隠密", "Stealth") }, { TERM_WHITE, _("帰", "Rc"), _("帰還", "Recall") },
    { TERM_WHITE, _("現", "Al"), _("現実変容", "Alter") }, { TERM_WHITE, _("オ", "SCo"), _("氷オーラ", "SCold") },
    { TERM_BLUE, _("オ", "SEl"), _("電オーラ", "SElec") }, { TERM_L_DARK, _("オ", "SSh"), _("影オーラ", "SShadow") },
    { TERM_YELLOW, _("腕", "EMi"), _("腕力強化", "ExMight") }, { TERM_RED, _("肉", "Bu"), _("肉体強化", "BuildUp") },
    { TERM_L_DARK, _("殖", "AMl"), _("反増殖", "AntiMulti") }, { TERM_ORANGE, _("テ", "AT"), _("反テレポ", "AntiTele") },
    { TERM_RED, _("魔", "AM"), _("反魔法", "AntiMagic") }, { TERM_SLATE, _("我", "Pa"), _("我慢", "Patience") },
    { TERM_SLATE, _("宣", "Rv"), _("宣告", "Revenge") }, { TERM_L_DARK, _("剣", "Rs"), _("魔剣化", "RuneSword") },
    { TERM_RED, _("吸", "Vm"), _("吸血打撃", "Vampiric") }, { TERM_WHITE, _("回", "Cu"), _("回復", "Cure") },
    { TERM_L_DARK, _("感", "ET"), _("邪悪感知", "EvilTele") }, { TERM_VIOLET, _("視", "NSi"), _("暗視", "NgtSgt") },
    { TERM_WHITE, _("破", "Ho"), _("破邪", "BHol") },
    { 0, nullptr, nullptr } };
