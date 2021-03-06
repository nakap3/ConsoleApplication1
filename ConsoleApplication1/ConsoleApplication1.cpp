#include <vector>
#include <algorithm>
#include <Windows.h>

#include "ToolsAndData/MonsterData_generated.h"
#include "flatbuffers/util.h"


#define USE_FLATBUFFERS 1 


/// モンスターデータ
struct MonsterData
{
    const char* label;      //< ラベル
    const char* name;       //< 名前
    int hp;                 //< 体力
    int ap;                 //< 攻撃力
    int dp;                 //< 防御力
};

static constexpr MonsterData s_monsterList[] =
{
    { "demon",      "デーモン", 	800,    40, 	25  },
    { "dragon", 	"ドラゴン", 	900,	45, 	10  },
    { "ghost",  	"ゴースト", 	500,    20, 	25  },
    { "hapry",  	"ハーピィ", 	600,	30, 	20  },
    { "ninja",	    "ニンジャ",	    400,	85, 	20  },
    { "slime",	    "スライム",	    200,	10,	    90  },
    { "vampire",	"バンパイア",	700,	15,	    60  },
    { "zombie",	    "ゾンビ",      	300,	20,	    30  }
};

// モンスターを強い順に並べなさい
// 
// どのモンスターが強いかは、実際に戦わせて決めます。
// 戦闘は 1 vs 1 で行い、先に力尽きた方が負けです。（力尽きる＝ヒットポイントが0以下になる）
// ステータスの意味は以下の通りです。
// ・hp  残体力
// ・ap  攻撃力（この値がそのまま基礎タメージ値になります）
// ・dp  防御力（この値の分だけ、受ける基礎ダメージの割合を減らします）
// 例）攻撃側の ap が 20 で、防御側の dp が 30 の時は、防御側は 20 の威力の攻撃を 30% 吸収するんで、 14 ダメージを受けます。
// ダメージ値に小数が出た場合は切り捨てられます。（例：1.5 ⇒ 1）
// 2体は同時に戦います。同時に力尽きた時は、力尽きた時の体力が大きい方が勝ちです。（例：残り体力が -20 と -5になった場合は、-5の方が勝ち）


void Attack(MonsterData& offense, MonsterData& defense)
{
    int damage = offense.ap * (100 - defense.dp) / 100;
    defense.hp -= damage;
    //printf("  %12s の攻撃！ %12s に %3d のダメージ！ 残りHP(%4d)\n", offense.name, defense.name, damage, defense.hp);
}

bool IsStronger(MonsterData& a, MonsterData& b)
{
    while (a.hp > 0 && b.hp > 0)
    {
        Attack(a, b);
        Attack(b, a);
    }

    return a.hp >= b.hp;
}

bool Battle(const MonsterData& a, const MonsterData& b)
{
    //printf("%s vs %s\n", a.name, b.name);

    auto aa = a;
    auto bb = b;
    return IsStronger(aa, bb);
}

int main()
{
#if defined(USE_FLATBUFFERS) && USE_FLATBUFFERS
    // flatbuffersでバイナリにシリアライズされたデータを読み込んで、monsterListを構築する
    std::vector<MonsterData> monsterList;
    std::string binaryData;
    if (flatbuffers::LoadFile("ToolsAndData/MonsterData.mdfb", true, &binaryData))
    {
        auto monsters = Data::GetMonsterList(binaryData.data());
        MonsterData monster;
        for (const auto& it : *monsters->monster_list())
        {
            monster.label = it->label()->c_str();
            monster.name = it->name()->c_str();
            monster.hp = it->hp();
            monster.ap = it->ap();
            monster.dp = it->dp();
            monsterList.push_back(monster);
        }
    }
#else
    std::vector<MonsterData> monsterList(std::begin(s_monsterList), std::end(s_monsterList));
#endif

    SetConsoleOutputCP(65001);

    // ソート前の状態を表示
    for (const auto& monster : monsterList)
    {
        printf("%10s %14s : HP(%4d) ATK(%3d) DEF(%3d)\n", monster.label, monster.name, monster.hp, monster.ap, monster.dp);
    }
    printf("\n");

    // ソート
    std::sort(monsterList.begin(), monsterList.end(), [](const auto& a, const auto& b) { return Battle(a, b); });

    // ソート後の状態を表示
    for (const auto& monster : monsterList)
    {
        printf("%10s %14s : HP(%4d) ATK(%3d) DEF(%3d)\n", monster.label, monster.name, monster.hp, monster.ap, monster.dp);
    }
    printf("\n");
}

