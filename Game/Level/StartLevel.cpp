#include "DxLib.h"
#include "StartLevel.h"
#include <cwchar>

static constexpr int SCREEN_W = 1280;
static constexpr int SCREEN_H = 720;
static constexpr int TITLE_Y = 200;
static constexpr int MENU_Y = 400;     // メニュー先頭のY
static constexpr int MENU_SPACING = 56;  // 項目の行間

StartLevel::StartLevel() {}

void StartLevel::Load()
{
    titleHandle_ = LoadGraph(L"Assets\\Image\\Title.png");
}

StartAction StartLevel::Update()
{
    const bool upNow = (CheckHitKey(KEY_INPUT_UP) != 0);
    const bool downNow = (CheckHitKey(KEY_INPUT_DOWN) != 0);
    const bool enterNow = (CheckHitKey(KEY_INPUT_RETURN) != 0);   // Enter


    const bool upTrig = !prevUpHeld_ && upNow;
    const bool downTrig = !prevDownHeld_ && downNow;
    const bool enterTrig = !prevEnterHeld_ && enterNow;

    prevUpHeld_ = upNow;
    prevDownHeld_ = downNow;
    prevEnterHeld_ = enterNow;

    // 選択移動（上下でループ）
    if (upTrig)   selected_ = (selected_ - 1 + MENU_COUNT) % MENU_COUNT;
    if (downTrig) selected_ = (selected_ + 1) % MENU_COUNT;

    // 決定
    if (enterTrig)
    {
        switch (selected_)
        {
        case 0: return StartAction::PLAY;
        case 1: return StartAction::COLLECTION;
        case 2: return StartAction::VOLUME;
        case 3: return StartAction::HINT;
        }
    }
    return StartAction::NONE;
}

void StartLevel::Draw() const
{
    // 背景（Title.png を全画面に）
    if (titleHandle_ != -1)
        DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, titleHandle_, FALSE);
    else
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(20, 40, 80), TRUE);  // 画像が無い時の保険

    // タイトル
    SetFontSize(72);
    const wchar_t* title = L"伝説の超！！";
    const int titleW = GetDrawStringWidth(title, static_cast<int>(wcslen(title)), FALSE);
    const int titleX = (SCREEN_W - titleW) / 2 - 190;   // 中央から左へ
    DrawString(titleX + 4, TITLE_Y + 4, title, GetColor(0, 0, 0));          // 影
    DrawString(titleX, TITLE_Y, title, GetColor(255, 140, 0));      // オレンジ

    // メニュー項目
    const wchar_t* items[MENU_COUNT] = {
        L"ゲームスタート",
        L"コレクション",
        L"音量設定",
        L"遊び方（ヒント）",
    };

    SetFontSize(32);
    for (int i = 0; i < MENU_COUNT; ++i)
    {
        const bool sel = (i == selected_);
        const int y = MENU_Y + MENU_SPACING * i;
        const int w = GetDrawStringWidth(items[i], static_cast<int>(wcslen(items[i])), FALSE);
        const int x = (SCREEN_W - w) / 2;

        const unsigned int textCol = sel
            ? GetColor(255, 220, 60)    // 選択中：黄
            : GetColor(200, 200, 200);  // 非選択：明るいグレー

        // 影（黒を少しずらして読みやすく）
        DrawString(x + 2, y + 2, items[i], GetColor(0, 0, 0));

        // 本体を 1px ずらして二重描き → 擬似ボールド
        DrawString(x, y, items[i], textCol);
        DrawString(x + 1, y, items[i], textCol);

        // 選択中はカーソル「▶」も太めに
        if (sel)
        {
            DrawString(x - 40, y, L"\x25B6", textCol);
            DrawString(x - 40 + 1, y, L"\x25B6", textCol);
        }
    }

    // 操作ガイド
    SetFontSize(18);
    const wchar_t* g = L"\x2191\x2193 で選択　Enter で決定";  // ↑↓ で選択 Enter で決定
    const int gw = GetDrawStringWidth(g, static_cast<int>(wcslen(g)), FALSE);
    DrawString((SCREEN_W - gw) / 2, MENU_Y + MENU_SPACING * MENU_COUNT + 20,
        g, GetColor(140, 200, 255));

    SetFontSize(16);
}