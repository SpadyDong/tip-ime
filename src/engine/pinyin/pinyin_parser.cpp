#include "pinyin_parser.h"

#include <unordered_set>

namespace tip {

namespace {

const std::unordered_set<std::wstring>& ValidPinyinSet() {
    static std::unordered_set<std::wstring> s_set = {
        L"a", L"ai", L"an", L"ang", L"ao",
        L"ba", L"bai", L"ban", L"bang", L"bao", L"bei", L"ben", L"beng", L"bi", L"bian", L"biao", L"bie", L"bin", L"bing", L"bo", L"bu",
        L"ca", L"cai", L"can", L"cang", L"cao", L"ce", L"cen", L"ceng", L"cha", L"chai", L"chan", L"chang", L"chao", L"che", L"chen", L"cheng", L"chi", L"chong", L"chou", L"chu", L"chuai", L"chuan", L"chuang", L"chui", L"chun", L"chuo", L"ci", L"cong", L"cou", L"cu", L"cuan", L"cui", L"cun", L"cuo",
        L"da", L"dai", L"dan", L"dang", L"dao", L"de", L"dei", L"den", L"deng", L"di", L"dian", L"diao", L"die", L"ding", L"diu", L"dong", L"dou", L"du", L"duan", L"dui", L"dun", L"duo",
        L"e", L"ei", L"en", L"eng", L"er",
        L"fa", L"fan", L"fang", L"fei", L"fen", L"feng", L"fo", L"fou", L"fu",
        L"ga", L"gai", L"gan", L"gang", L"gao", L"ge", L"gei", L"gen", L"geng", L"gong", L"gou", L"gu", L"gua", L"guai", L"guan", L"guang", L"gui", L"gun", L"guo",
        L"ha", L"hai", L"han", L"hang", L"hao", L"he", L"hei", L"hen", L"heng", L"hong", L"hou", L"hu", L"hua", L"huai", L"huan", L"huang", L"hui", L"hun", L"huo",
        L"ji", L"jia", L"jian", L"jiang", L"jiao", L"jie", L"jin", L"jing", L"jiong", L"jiu", L"ju", L"juan", L"jue", L"jun",
        L"ka", L"kai", L"kan", L"kang", L"kao", L"ke", L"kei", L"ken", L"keng", L"kong", L"kou", L"ku", L"kua", L"kuai", L"kuan", L"kuang", L"kui", L"kun", L"kuo",
        L"la", L"lai", L"lan", L"lang", L"lao", L"le", L"lei", L"leng", L"li", L"lia", L"lian", L"liang", L"liao", L"lie", L"lin", L"ling", L"liu", L"long", L"lou", L"lu", L"luan", L"lue", L"lun", L"luo", L"lv",
        L"ma", L"mai", L"man", L"mang", L"mao", L"me", L"mei", L"men", L"meng", L"mi", L"mian", L"miao", L"mie", L"min", L"ming", L"miu", L"mo", L"mou", L"mu",
        L"na", L"nai", L"nan", L"nang", L"nao", L"ne", L"nei", L"nen", L"neng", L"ni", L"nian", L"niang", L"niao", L"nie", L"nin", L"ning", L"niu", L"nong", L"nou", L"nu", L"nuan", L"nue", L"nuo", L"nv",
        L"o", L"ou",
        L"pa", L"pai", L"pan", L"pang", L"pao", L"pei", L"pen", L"peng", L"pi", L"pian", L"piao", L"pie", L"pin", L"ping", L"po", L"pou", L"pu",
        L"qi", L"qia", L"qian", L"qiang", L"qiao", L"qie", L"qin", L"qing", L"qiong", L"qiu", L"qu", L"quan", L"que", L"qun",
        L"ran", L"rang", L"rao", L"re", L"ren", L"reng", L"ri", L"rong", L"rou", L"ru", L"ruan", L"rui", L"run", L"ruo",
        L"sa", L"sai", L"san", L"sang", L"sao", L"se", L"sen", L"seng", L"sha", L"shai", L"shan", L"shang", L"shao", L"she", L"shei", L"shen", L"sheng", L"shi", L"shou", L"shu", L"shua", L"shuai", L"shuan", L"shuang", L"shui", L"shun", L"shuo", L"si", L"song", L"sou", L"su", L"suan", L"sui", L"sun", L"suo",
        L"ta", L"tai", L"tan", L"tang", L"tao", L"te", L"tei", L"teng", L"ti", L"tian", L"tiao", L"tie", L"ting", L"tong", L"tou", L"tu", L"tuan", L"tui", L"tun", L"tuo",
        L"wa", L"wai", L"wan", L"wang", L"wei", L"wen", L"weng", L"wo", L"wu",
        L"xi", L"xia", L"xian", L"xiang", L"xiao", L"xie", L"xin", L"xing", L"xiong", L"xiu", L"xu", L"xuan", L"xue", L"xun",
        L"ya", L"yan", L"yang", L"yao", L"ye", L"yi", L"yin", L"ying", L"yo", L"yong", L"you", L"yu", L"yuan", L"yue", L"yun",
        L"za", L"zai", L"zan", L"zang", L"zao", L"ze", L"zei", L"zen", L"zeng", L"zha", L"zhai", L"zhan", L"zhang", L"zhao", L"zhe", L"zhei", L"zhen", L"zheng", L"zhi", L"zhong", L"zhou", L"zhu", L"zhua", L"zhuai", L"zhuan", L"zhuang", L"zhui", L"zhun", L"zhuo", L"zi", L"zong", L"zou", L"zu", L"zuan", L"zui", L"zun", L"zuo"
    };
    return s_set;
}

} // namespace

std::vector<std::wstring> PinyinParser::Segment(const std::wstring& rawPinyin) {
    std::vector<std::wstring> result;
    size_t i = 0;
    while (i < rawPinyin.size()) {
        // Greedy longest match
        size_t maxLen = std::min(static_cast<size_t>(6), rawPinyin.size() - i);
        bool found = false;
        for (size_t len = maxLen; len > 0; --len) {
            std::wstring sub = rawPinyin.substr(i, len);
            if (IsValidPinyin(sub)) {
                result.push_back(sub);
                i += len;
                found = true;
                break;
            }
        }
        if (!found) {
            // Treat single char as isolated syllable
            result.push_back(rawPinyin.substr(i, 1));
            ++i;
        }
    }
    return result;
}

bool PinyinParser::IsValidPinyin(const std::wstring& pinyin) {
    return ValidPinyinSet().find(pinyin) != ValidPinyinSet().end();
}

std::vector<std::wstring> PinyinParser::GetJianpinCombinations(const std::wstring& rawPinyin) {
    // TODO: generate jianpin combinations (e.g., "zhongguo" -> "zg")
    return { rawPinyin };
}

} // namespace tip
