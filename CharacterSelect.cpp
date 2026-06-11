#include "DxLib.h"
#include "CharacterSelect.h"
#include <cstdio>
#include <cwchar>

static constexpr int SCREEN_W = 1280;
static constexpr int SCREEN_H = 720;
static constexpr float IDLE_FPS = 10.0f; 
static constexpr float SPRITE_DRAW_SIZE = 120.0f;

struct SeMapEntry
{
    const wchar_t* name;
    const wchar_t* seDir;
};

static const SeMapEntry SE_MAP[] =
{
    { L"シノビ",       L"Assets\\SE\\Shinobi\\" }, // シノビ
    { L"ウィザードマン", L"Assets\\SE\\Wizard\\"  }, // 魔法使い
    { L"老師",             L"Assets\\SE\\Roushi\\" }, // 老師
};
static constexpr int SE_MAP_COUNT = 3;

struct CharaSpriteConfig
{
    const wchar_t* charaName;
    const wchar_t* idlePath;
    const wchar_t* prefix;
    int  frameCount;
    bool isSpriteSheet;
    int  frameWidth;
    int  frameHeight;
    float drawSize;
    int   drawOffsetY;

    // ゲーム中プレイヤースプライト
    const wchar_t* playerIdleDir;
    const wchar_t* playerIdlePrefix;
    int            playerIdleCount;
    bool           playerIdleIsSheet;
    int            playerIdleW;
    int            playerIdleH;
    const wchar_t* playerAttackDir;
    const wchar_t* playerAttackPrefix;
    int            playerAttackCount;
    bool           playerAttackIsSheet;
    int            playerAttackW;
    int            playerAttackH;
    float          playerDrawScale;
};

struct ChargeLoopRange { int start; int end; };
static const ChargeLoopRange CHARGE_LOOPS[] = 
{
    { 12, 12 }, // シノビ：12で停止（従来どおり）
    {  5,  7 }, // ウィザード：5..7をループ
    {  1,  1 }, // 老師：仮（後で調整）
};


static const CharaSpriteConfig SPRITE_CONFIGS[] =
{
    { L"シノビ",
      L"Assets\\Player\\Idle\\", L"ninja_10KDStudios_idle_",
      16, false, 0, 0, 160.0f, -20,
      L"Assets\\Player\\Idle\\",   L"ninja_10KDStudios_idle_",   16, false, 0, 0,
      L"Assets\\Player\\Attack\\", L"ninja_10KDStudios_attack_", 67, false, 0, 0,
      1.5f },

    { L"ウィザードマン",
      L"Assets\\Player\\Wizard\\Idle.png", nullptr,
      8, true, 128, 128, 260.0f, -60,
      L"Assets\\Player\\Wizard\\Idle.png",         nullptr, 8,  true, 128, 128,
      L"Assets\\Player\\Wizard\\Magic_sphere.png", nullptr, 16, true, 170, 128,
      2.5f},

    { L"老師",
      L"Assets\\Player\\Roushi\\IDLE.png", nullptr,
      10, true, 96, 96, 260.0f, -20,
      L"Assets\\Player\\Roushi\\IDLE.png",   nullptr, 10, true, 96, 96,
      L"Assets\\Player\\Roushi\\ATTACK.png", nullptr,  7, true, 96, 96,
      3.5f},
};
static constexpr int SPRITE_CONFIG_COUNT = 3;

CharacterSelect::CharacterSelect()
    : selectedIndex_(0)
    , prevLeftHeld_(false)
    , prevRightHeld_(false)
    , prevSpaceHeld_(true)
{
}

CharacterSelect::~CharacterSelect()
{
    for (auto& c : characters_)
        for (int h : c.idleHandles)
            if (h != -1) DeleteGraph(h);
}

void CharacterSelect::LoadSprites(CharaData& data, const wchar_t* idlePath,
    const wchar_t* prefix, int frameCount,
    bool isSpriteSheet, int frameW, int frameH)
{
    data.frameCount = frameCount;
    data.animTimer = 1.0f / IDLE_FPS;
    data.currentFrame = 0;
    data.idleHandles.assign(frameCount, -1);

    if (isSpriteSheet)
    {
        LoadDivGraph(idlePath, frameCount, frameCount, 1,
            frameW, frameH, data.idleHandles.data());
    }
    else
    {
        wchar_t path[512];
        for (int i = 0; i < frameCount; ++i)
        {
            swprintf_s(path, L"%s%s%05d.png", idlePath, prefix, i);
            data.idleHandles[i] = LoadGraph(path);
        }
    }
}

bool CharacterSelect::Load()
{
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, CSV_PATH, L"r, ccs=UTF-8") != 0 || fp == nullptr)
        return false;

    wchar_t buf[512];
    bool isFirst = true;
    int  rowIndex = 0;

    while (fgetws(buf, 512, fp) != nullptr)
    {
        std::wstring line(buf);
        while (!line.empty() && (line.back() == L'\n' || line.back() == L'\r'))
            line.pop_back();
        if (line.empty()) continue;
        if (isFirst) { isFirst = false; continue; }

        // 行全体のクォート除去
        if (line.size() >= 2 && line.front() == L'"' && line.back() == L'"')
            line = line.substr(1, line.size() - 2);

        const auto comma = line.find(L',');
        if (comma == std::wstring::npos) continue;

        CharaData data;
        data.name = line.substr(0, comma);
        data.comment = line.substr(comma + 1);
        if (rowIndex < SE_MAP_COUNT)
            data.seDir = SE_MAP[rowIndex].seDir;
        else
            data.seDir = L"Assets\\SE\\Shinobi\\";

        // スプライト読み込み（行番号で直接対応）
        if (rowIndex < SPRITE_CONFIG_COUNT)
        {
            const auto& cfg = SPRITE_CONFIGS[rowIndex];
            LoadSprites(data, cfg.idlePath, cfg.prefix,
                cfg.frameCount, cfg.isSpriteSheet,
                cfg.frameWidth, cfg.frameHeight);
            data.drawSize = cfg.drawSize;
            data.drawOffsetY = cfg.drawOffsetY;
        }

        characters_.push_back(data);
        ++rowIndex;
    }

    fclose(fp);
    return !characters_.empty();
}

bool CharacterSelect::Update(float deltaTime)
{
    if (characters_.empty()) return false;

    const bool leftNow = (CheckHitKey(KEY_INPUT_LEFT) != 0);
    const bool rightNow = (CheckHitKey(KEY_INPUT_RIGHT) != 0);
    const bool spaceNow = (CheckHitKey(KEY_INPUT_SPACE) != 0)
        || (CheckHitKey(KEY_INPUT_RETURN) != 0);   // Space または Enter

    const int count = static_cast<int>(characters_.size());
    if (!prevLeftHeld_ && leftNow)
        selectedIndex_ = (selectedIndex_ - 1 + count) % count;
    if (!prevRightHeld_ && rightNow)
        selectedIndex_ = (selectedIndex_ + 1) % count;

    // 選択中キャラのみアニメ更新
    CharaData& sel = characters_[selectedIndex_];
    if (sel.frameCount > 0)
    {
        sel.animTimer -= deltaTime;
        while (sel.animTimer <= 0.0f)
        {
            sel.animTimer += 1.0f / IDLE_FPS;
            sel.currentFrame = (sel.currentFrame + 1) % sel.frameCount;
        }
    }

    const bool confirmed = !prevSpaceHeld_ && spaceNow;
    prevLeftHeld_ = leftNow;
    prevRightHeld_ = rightNow;
    prevSpaceHeld_ = spaceNow;
    return confirmed;
}

const wchar_t* CharacterSelect::GetSelectedSeDir() const
{
    if (characters_.empty()) return L"Assets\\SE\\Shinobi\\";
    return characters_[selectedIndex_].seDir;
}

PlayerSpriteConfig CharacterSelect::GetSelectedPlayerConfig() const
{
    // デフォルト（シノビ）
    const CharaSpriteConfig& cfg = SPRITE_CONFIGS[0];
    int idx = (selectedIndex_ >= 0 && selectedIndex_ < SPRITE_CONFIG_COUNT)
        ? selectedIndex_ : 0;
    const CharaSpriteConfig& selected = SPRITE_CONFIGS[idx];

    PlayerSpriteConfig pc;
    pc.idleDir = selected.playerIdleDir;
    pc.idlePrefix = selected.playerIdlePrefix;
    pc.idleFrameCount = selected.playerIdleCount;
    pc.idleIsSpriteSheet = selected.playerIdleIsSheet;
    pc.idleFrameW = selected.playerIdleW;
    pc.idleFrameH = selected.playerIdleH;
    pc.attackDir = selected.playerAttackDir;
    pc.attackPrefix = selected.playerAttackPrefix;
    pc.attackFrameCount = selected.playerAttackCount;
    pc.attackIsSpriteSheet = selected.playerAttackIsSheet;
    pc.attackFrameW = selected.playerAttackW;
    pc.attackFrameH = selected.playerAttackH;
    pc.drawScale = selected.playerDrawScale;
    pc.chargeLoopStart = CHARGE_LOOPS[idx].start;
    pc.chargeLoopEnd = CHARGE_LOOPS[idx].end;
    return pc;
}


/// <summary>最大幅で自動折り返ししながら文字列を中央揃えで描画する。</summary>
static void DrawWrappedString(int cx, int y, const wchar_t* text,
    unsigned int color, int maxWidth)
{
    static constexpr int LINE_HEIGHT = 22; // フォントサイズ16 + 行間

    std::wstring str(text);
    std::wstring currentLine;
    int lineY = y;

    for (wchar_t ch : str)
    {
        std::wstring test = currentLine + ch;
        const int testW = GetDrawStringWidth(
            test.c_str(), static_cast<int>(test.size()), FALSE);

        if (testW > maxWidth && !currentLine.empty())
        {
            // 現在行を中央揃えで描画して改行
            const int lineW = GetDrawStringWidth(
                currentLine.c_str(), static_cast<int>(currentLine.size()), FALSE);
            DrawString(cx - lineW / 2, lineY, currentLine.c_str(), color);
            currentLine = ch;
            lineY += LINE_HEIGHT;
        }
        else
        {
            currentLine += ch;
        }
    }

    // 残りの文字を描画
    if (!currentLine.empty())
    {
        const int lineW = GetDrawStringWidth(
            currentLine.c_str(), static_cast<int>(currentLine.size()), FALSE);
        DrawString(cx - lineW / 2, lineY, currentLine.c_str(), color);
    }
}

void CharacterSelect::Draw() const
{
    DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(10, 10, 30), TRUE);

    // タイトル
    SetFontSize(36);
    const wchar_t* title = L"\x30AD\x30E3\x30E9\x30AF\x30BF\x30FC\x9078\x629E"; // キャラクター選択
    const int titleW = GetDrawStringWidth(title, static_cast<int>(wcslen(title)), FALSE);
    DrawString((SCREEN_W - titleW) / 2, 60, title, GetColor(255, 220, 60));

    // キャラクター一覧
    const int count = static_cast<int>(characters_.size());
    const int cardW = 300;
    const int totalW = cardW * count + 40 * (count - 1);
    int x = (SCREEN_W - totalW) / 2 + cardW / 2;

    for (int i = 0; i < count; ++i)
    {
        const bool isSelected = (i == selectedIndex_);
        const auto& c = characters_[i];

        // 選択枠
        DrawBox(x - cardW / 2, 160, x + cardW / 2, 520,
            isSelected ? GetColor(255, 200, 0) : GetColor(60, 60, 60), FALSE);

        // スプライト（選択中はアニメ、非選択は静止）
        const int frameIdx = isSelected ? c.currentFrame : 0;
        if (!c.idleHandles.empty() && c.idleHandles[frameIdx] != -1)
        {
            int imgW, imgH;
            GetGraphSize(c.idleHandles[frameIdx], &imgW, &imgH);
            const float size = (c.drawSize > 0.0f) ? c.drawSize : SPRITE_DRAW_SIZE;
            const float scale = size / static_cast<float>(imgH);
            DrawRotaGraph(x, 270 + c.drawOffsetY, static_cast<double>(scale),
                0.0, c.idleHandles[frameIdx], TRUE, FALSE);
        }
        // キャラクター名（スプライトの下）
        SetFontSize(28);
        const int nameW = GetDrawStringWidth(c.name.c_str(),
            static_cast<int>(c.name.size()), FALSE);
        DrawString(x - nameW / 2, 345, c.name.c_str(),         // ← 名前の位置
            isSelected ? GetColor(255, 220, 60) : GetColor(140, 140, 140));
        // コメント（名前の下）
        SetFontSize(14);
        DrawWrappedString(x, 390, c.comment.c_str(),           // ← コメントを名前の下へ
            isSelected ? GetColor(210, 210, 210) : GetColor(90, 90, 90),
            cardW - 40);

        x += cardW + 40;
    }

    // 操作ガイド
    SetFontSize(20);
    const wchar_t* guide = L"\x2190 \x2192 \x3067\x9078\x629E\x3000\x3000SPACE \x3067\x6C7A\x5B9A";
    const int guideW = GetDrawStringWidth(guide, static_cast<int>(wcslen(guide)), FALSE);
    DrawString((SCREEN_W - guideW) / 2, 620, guide, GetColor(180, 180, 180));

    SetFontSize(16);
}