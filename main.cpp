#include "DxLib.h"
#include "Ball.h"
#include "InputManager.h"
#include "SwingJudge.h"
#include "AudioBank.h"
#include "LatencyOverlay.h"
#include "SwingResult.h"
#include "PitchMachine.h"
#include "Collection.h"
#include "HitCommentBank.h"
#include "Player.h"
#include "StartLevel.h"
#include "CharacterSelect.h"
#include "CharaTrait.h"
#include <cwchar>
#include <algorithm>
#include <ctime>

// -------------------------------------------------------
// 画面設定
// -------------------------------------------------------
static constexpr int   SCREEN_W    = 1280; ///< 画面幅（ピクセル）
static constexpr int   SCREEN_H    =  720; ///< 画面高さ（ピクセル）
static constexpr int   COLOR_DEPTH =   32; ///< 色深度（ビット）

// -------------------------------------------------------
// フレームレート制御
// -------------------------------------------------------
static constexpr float MAX_DELTA_TIME = 0.1f; ///< dt スパイク防止クランプ上限（秒）

// -------------------------------------------------------
// フォントサイズ
// -------------------------------------------------------
static constexpr int FONT_SIZE_RESULT   = 56; ///< 判定テキストの大きいフォント
static constexpr int FONT_SIZE_SUBTEXT  = 24; ///< 判定サブテキストのフォント
static constexpr int FONT_SIZE_GUIDE    = 20; ///< 操作ガイドのフォント
static constexpr int FONT_SIZE_DEFAULT  = 16; ///< 通常フォント（各処理後に必ず戻す）
static constexpr int FONT_SIZE_HINT     = 12; ///< 画面隅ヒントのフォント
static constexpr int FONT_SIZE_HR       = 64; ///< ホームランテキストのフォント

// -------------------------------------------------------
// 判定テキスト表示
// -------------------------------------------------------
static constexpr int   RESULT_TEXT_Y           = 240; ///< 判定テキストの Y 座標
static constexpr int   RESULT_SUBTEXT_Y_OFFSET =  64; ///< サブテキストのオフセット
static constexpr int   RESULT_SHADOW_OFFSET    =   3; ///< テキスト影のずらし量
static constexpr float RESULT_DISPLAY_SEC      = 2.0f; ///< 判定テキストの表示時間（秒）
bool lastWasMissLook = false;   // 見逃し（見出しを出さない）か

// -------------------------------------------------------
// パワーメーター表示
// -------------------------------------------------------
static constexpr int   POWER_BAR_X    = 100;  ///< パワーメーター左上 X
static constexpr int   POWER_BAR_Y    = 630;  ///< パワーメーター左上 Y
static constexpr int   POWER_BAR_W    = 200;  ///< パワーメーターの横幅
static constexpr int   POWER_BAR_H    =  18;  ///< パワーメーターの高さ

// -------------------------------------------------------
// ホームラン演出
// -------------------------------------------------------
static constexpr float HR_FLASH_SEC   = 0.4f;  ///< HR フラッシュ時間（秒）
static constexpr float HR_DISPLAY_SEC = 3.0f;  ///< HR テキスト表示時間（秒）

// -------------------------------------------------------
// バッター描画
// -------------------------------------------------------
static constexpr int BATTER_BAT_HALF_W    =  5;
static constexpr int BATTER_BAT_HALF_H    = 30;
static constexpr int BATTER_HEAD_OFFSET_X = 10;
static constexpr int BATTER_HEAD_OFFSET_Y = 35;
static constexpr int BATTER_HEAD_RADIUS   = 18;

// -------------------------------------------------------
// ピッチングレーン
// -------------------------------------------------------
static constexpr int LANE_MARKER_HALF_H = 40;

// -------------------------------------------------------
// 画面隅ヒント
// -------------------------------------------------------
static constexpr int HINT_OFFSET_FROM_RIGHT  = 100;
static constexpr int HINT_OFFSET_FROM_BOTTOM =  20;

// -------------------------------------------------------
// 色定数
// -------------------------------------------------------
static constexpr Rgb COLOR_BG_SKY        = { 137, 207, 240 };
static constexpr Rgb COLOR_BG_FLOOR      = {  30,  25,  20 };
static constexpr Rgb COLOR_LANE_LINE     = {  40,  40,  60 };
static constexpr Rgb COLOR_LANE_MARKER   = {  60,  80, 100 };
static constexpr Rgb COLOR_BAT           = { 180, 180, 140 };
static constexpr Rgb COLOR_BATTER_BODY   = { 220, 180, 120 };
static constexpr Rgb COLOR_SHADOW_TEXT   = {   0,   0,   0 };
static constexpr Rgb COLOR_PERFECT_SUB   = { 255, 220,   0 };
static constexpr Rgb COLOR_SWEET_SUB     = { 255, 160,   0 };
static constexpr Rgb COLOR_GUIDE_TEXT    = { 200, 200, 200 };
static constexpr Rgb COLOR_HINT_TEXT     = { 100, 100, 100 };
static constexpr Rgb COLOR_HR_TEXT       = { 255, 220,  50 };
static constexpr Rgb COLOR_POWER_EMPTY   = {  60,  60,  60 };
static constexpr Rgb COLOR_POWER_LOW     = {  60, 120, 220 };
static constexpr Rgb COLOR_POWER_MID     = { 220, 200,  20 };
static constexpr Rgb COLOR_POWER_FULL    = { 255,  60,  20 };
static constexpr Rgb COLOR_POWER_OUTLINE = { 160, 160, 160 };
static constexpr Rgb COLOR_POWER_LABEL   = { 180, 180, 180 };
static constexpr Rgb COLOR_HR_PRIZE      = { 255, 200,   0 };

// -------------------------------------------------------
// ゲーム状態
// -------------------------------------------------------

/// <summary>ゲームのフェーズを表す列挙型。</summary>
enum class GameState
{
    START,      ///< スタート画面
    CHAR_SELECT,///< キャラ選択画面
    PLAYING,    ///< 通常プレイ中
    COLLECTION, ///< コレクション詳細画面
    GAME_OVER,  ///< 全球使用後のコレクション画面
    VOLUME,   // 音量設定画面
    HINT,     // ヒント画面
};

/// <summary>打撃操作のサブフェーズ。</summary>
enum class SwingPhase
{
    IDLE,     ///< 待機（次の打撃待ち）
    CHARGING, ///< フェーズ1：Space押しでパワーチャージ中
    TIMING,   ///< フェーズ2：パワー確定済み、再度Spaceでタイミング入力
};

// -------------------------------------------------------
// 前方宣言
// -------------------------------------------------------
static void DrawBackground(int netHandle);
static void DrawBatter();
static void DrawPowerMeter(float chargeRatio, bool isCharging);
static void DrawHrEffect(float hrTimer);
static void DrawResultText(SwingResult result, float remainingRatio,
    const wchar_t* comment, float distanceMeters, bool hideHeadline);

// -------------------------------------------------------
// DrawBackground
// -------------------------------------------------------

/// <summary>バッティングセンター風の背景を描画する。</summary>
static void DrawBackground(int netHandle)
{
    // 青ベタ塗り（黒→青）
    DrawBox(0, 0, SCREEN_W, SCREEN_H,
        GetColor(COLOR_BG_SKY.r, COLOR_BG_SKY.g, COLOR_BG_SKY.b), TRUE);
    DrawBox(0, SCREEN_H * 3 / 4, SCREEN_W, SCREEN_H,
        GetColor(COLOR_BG_FLOOR.r, COLOR_BG_FLOOR.g, COLOR_BG_FLOOR.b), TRUE);

    // 網（黒透過PNG）を全画面に重ねる → 隙間から青が見える
    // 第4引数 TRUE が透過を有効にする
    if (netHandle != -1)
    {
        const int playerFeetY = Player::DRAW_CENTER_Y
            + static_cast<int>(Player::SPRITE_SIZE * Player::DRAW_SCALE / 2);
        const int netTop = 60;
        const int netBottom = playerFeetY - 80;

        // 画像全体を、画面の (0,netTop)〜(SCREEN_W,netBottom) に引き伸ばして描く
        DrawExtendGraph(0, netTop, SCREEN_W, netBottom, netHandle, TRUE);
    }

}

// -------------------------------------------------------
// DrawBatter
// -------------------------------------------------------

/// <summary>打者の簡易シルエットをヒットゾーンに描画する。</summary>
static void DrawBatter()
{
    const int hitX = static_cast<int>(Ball::HIT_ZONE_X);
    const int hitY = static_cast<int>(Ball::BALL_Y);

    DrawBox(hitX - BATTER_BAT_HALF_W, hitY - BATTER_BAT_HALF_H,
            hitX + BATTER_BAT_HALF_W, hitY + BATTER_BAT_HALF_H,
            GetColor(COLOR_BAT.r, COLOR_BAT.g, COLOR_BAT.b), TRUE);
    DrawCircle(
        hitX - BATTER_HEAD_OFFSET_X,
        hitY + BATTER_HEAD_OFFSET_Y,
        BATTER_HEAD_RADIUS,
        GetColor(COLOR_BATTER_BODY.r, COLOR_BATTER_BODY.g, COLOR_BATTER_BODY.b), TRUE);
}

// -------------------------------------------------------
// DrawPowerMeter
// -------------------------------------------------------

/// <summary>
/// パワー充填メーターを描画する。
/// 充填量に応じて青→黄→赤にグラデーションする。
/// </summary>
/// <param name="chargeRatio">充填割合（0.0 ～ 1.0）</param>
/// <param name="isCharging">充填中かどうか（点滅制御に使用）</param>
static void DrawPowerMeter(float chargeRatio, bool isCharging)
{
    // ラベル「POWER」
    SetFontSize(12);
    DrawString(POWER_BAR_X, POWER_BAR_Y - 16,
               L"POWER", GetColor(COLOR_POWER_LABEL.r, COLOR_POWER_LABEL.g, COLOR_POWER_LABEL.b));
    SetFontSize(FONT_SIZE_DEFAULT);

    // 背景
    DrawBox(POWER_BAR_X, POWER_BAR_Y,
            POWER_BAR_X + POWER_BAR_W, POWER_BAR_Y + POWER_BAR_H,
            GetColor(COLOR_POWER_EMPTY.r, COLOR_POWER_EMPTY.g, COLOR_POWER_EMPTY.b), TRUE);

    // バー本体
    if (chargeRatio > 0.0f)
    {
        const int fillWidth = static_cast<int>(POWER_BAR_W * chargeRatio);

        // 充填量に応じた色（低→青、中→黄、高→赤）
        const Rgb& barColor = (chargeRatio < 0.5f)                              ? COLOR_POWER_LOW
                            : (chargeRatio < InputManager::RED_POWER_THRESHOLD) ? COLOR_POWER_MID
                                                                                 : COLOR_POWER_FULL;
        DrawBox(POWER_BAR_X, POWER_BAR_Y,
                POWER_BAR_X + fillWidth, POWER_BAR_Y + POWER_BAR_H,
                GetColor(barColor.r, barColor.g, barColor.b), TRUE);
    }

    // 枠線
    DrawBox(POWER_BAR_X, POWER_BAR_Y,
            POWER_BAR_X + POWER_BAR_W, POWER_BAR_Y + POWER_BAR_H,
            GetColor(COLOR_POWER_OUTLINE.r, COLOR_POWER_OUTLINE.g, COLOR_POWER_OUTLINE.b), FALSE);

    // FULL 表示
    if (chargeRatio >= 1.0f && isCharging)
    {
        SetFontSize(14);
        DrawString(POWER_BAR_X + POWER_BAR_W + 6, POWER_BAR_Y,
                   L"FULL!", GetColor(COLOR_POWER_FULL.r, COLOR_POWER_FULL.g, COLOR_POWER_FULL.b));
        SetFontSize(FONT_SIZE_DEFAULT);
    }
}

// -------------------------------------------------------
// DrawResultText
// -------------------------------------------------------

/// <summary>
/// 判定テキストとコメントをフェードアウトつきで画面中央に描画する。
/// </summary>
/// <param name="result">表示する判定結果</param>
/// <param name="remainingRatio">残り表示割合（1.0=始まり、0.0=非表示）</param>
/// <param name="comment">CSV から取得したコメント文字列。nullptr なら非表示。</param>
static void DrawResultText(SwingResult result, float remainingRatio,
    const wchar_t* comment, float distanceMeters, bool hideHeadline)
{
    if (result == SwingResult::NONE || remainingRatio <= 0.0f) return;

    // 当たったかどうか（飛距離を出す対象か）
    const bool isContact =
        (result == SwingResult::GRAZE || result == SwingResult::NORMAL ||
            result == SwingResult::SWEET || result == SwingResult::PERFECT);

    // 色とフェード（見出し・コメント共通で使う）
    const Rgb   baseColor = SwingResultColor(result);
    const float alpha = std::min(1.0f, remainingRatio * 2.0f);
    const unsigned int fadedColor = GetColor(
        static_cast<int>(baseColor.r * alpha),
        static_cast<int>(baseColor.g * alpha),
        static_cast<int>(baseColor.b * alpha));

    const int textY = RESULT_TEXT_Y;

    // 見出し（見逃し時は出さない）
    if (!hideHeadline)
    {
        wchar_t headline[64] = {};
        if (isContact && distanceMeters > 0.0f)
            swprintf_s(headline, L"%.1f m", distanceMeters);
        else
            wcscpy_s(headline, SwingResultName(result));

        SetFontSize(FONT_SIZE_RESULT);
        const int textW = GetDrawStringWidth(headline, static_cast<int>(wcslen(headline)), FALSE);
        const int textX = (SCREEN_W - textW) / 2;

        DrawString(textX + RESULT_SHADOW_OFFSET, textY + RESULT_SHADOW_OFFSET, headline,
            GetColor(COLOR_SHADOW_TEXT.r, COLOR_SHADOW_TEXT.g, COLOR_SHADOW_TEXT.b));
        DrawString(textX, textY, headline, fadedColor);
    }

    // コメント（サブテキスト）
    if (comment != nullptr && wcslen(comment) > 0)
    {
        SetFontSize(FONT_SIZE_SUBTEXT);
        const int commentW = GetDrawStringWidth(comment, static_cast<int>(wcslen(comment)), FALSE);
        DrawString(
            (SCREEN_W - commentW) / 2,
            textY + RESULT_SUBTEXT_Y_OFFSET,
            comment,
            GetColor(
                static_cast<int>(baseColor.r * alpha * 0.85f),
                static_cast<int>(baseColor.g * alpha * 0.85f),
                static_cast<int>(baseColor.b * alpha * 0.85f)));
    }

    SetFontSize(FONT_SIZE_DEFAULT);
}
// -------------------------------------------------------
// DrawHrEffect
// -------------------------------------------------------

/// <summary>
/// ホームラン演出（フラッシュ＋大テキスト）を描画する。
/// </summary>
/// <param name="hrTimer">HR 演出の残り時間（秒）。0 になると消える。</param>
static void DrawHrEffect(float hrTimer)
{
    if (hrTimer <= 0.0f) return;

    // 画面フラッシュ（演出開始直後だけ）
    if (hrTimer > HR_DISPLAY_SEC - HR_FLASH_SEC)
    {
        const float flashRatio = (hrTimer - (HR_DISPLAY_SEC - HR_FLASH_SEC)) / HR_FLASH_SEC;
        const int   flashAlpha = static_cast<int>(flashRatio * 180);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashAlpha);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 240, 100), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // HR テキスト
    const float alpha = std::min(1.0f, hrTimer / 0.5f);
    SetFontSize(FONT_SIZE_HR);

    const wchar_t* hrText = L"HOME RUN!!";
    const int hrW = GetDrawStringWidth(hrText, static_cast<int>(wcslen(hrText)), FALSE);

    // 現在位置(SCREEN_H/2 - 60 = 300)と画面最上部(0)の中間 = 150
    const int hrBaseY = (SCREEN_H / 2 - 60) / 2;

    DrawString((SCREEN_W - hrW) / 2 + 4, hrBaseY + 4,
        hrText, GetColor(0, 0, 0));
    DrawString((SCREEN_W - hrW) / 2, hrBaseY,
        hrText,
        GetColor(
            static_cast<int>(COLOR_HR_TEXT.r * alpha),
            static_cast<int>(COLOR_HR_TEXT.g * alpha),
            static_cast<int>(COLOR_HR_TEXT.b * alpha)));

    SetFontSize(FONT_SIZE_DEFAULT);
}

// -------------------------------------------------------
// DrawPrizeUnlock
// -------------------------------------------------------

/// <summary>
/// 新しい景品が解放されたときに画面下部に通知を描画する。
/// </summary>
/// <param name="prizeIndex">解放された景品のインデックス（-1 なら非表示）</param>
/// <param name="displayRatio">表示割合（1.0=始まり、0.0=消える）</param>
static void DrawPrizeUnlock(const wchar_t* itemName, float displayRatio)
{
    if (itemName == nullptr || displayRatio <= 0.0f) return;

    const float alpha = std::min(1.0f, displayRatio * 3.0f);
    const int   a = static_cast<int>(alpha * 220);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
    DrawBox(300, 580, 980, 640, GetColor(30, 20, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    SetFontSize(18);
    wchar_t buf[128];
    swprintf_s(buf, L"景品ゲット！ → %s", itemName); // ← 直接使う
    const int textW = GetDrawStringWidth(buf, static_cast<int>(wcslen(buf)), FALSE);
    DrawString((SCREEN_W - textW) / 2, 595, buf,
        GetColor(
            static_cast<int>(COLOR_HR_PRIZE.r * alpha),
            static_cast<int>(COLOR_HR_PRIZE.g * alpha),
            static_cast<int>(COLOR_HR_PRIZE.b * alpha)));
    SetFontSize(FONT_SIZE_DEFAULT);
}

static void DrawVolumeUI(const AudioBank& audio, int sel)
{
    const int barW = 360, barH = 24, gap = 70, labelW = 70;
    const int totalW = labelW + barW;
    const int baseX = (SCREEN_W - totalW) / 2;          // 横中央
    const int baseY = SCREEN_H / 2 - gap / 2;           // 縦中央（2項目ぶん）

    const wchar_t* labels[2] = { L"BGM", L"SE" };
    const int vols[2] = { audio.GetBgmVolume(), audio.GetSeVolume() };

    SetFontSize(24);
    for (int i = 0; i < 2; ++i)
    {
        const int yy = baseY + i * gap;
        const bool seld = (i == sel);

        DrawString(baseX, yy - 2, labels[i],
            seld ? GetColor(255, 220, 60) : GetColor(180, 180, 180));

        const int bx = baseX + labelW;
        DrawBox(bx, yy, bx + barW, yy + barH, GetColor(50, 50, 55), TRUE);
        DrawBox(bx, yy, bx + barW * vols[i] / 255, yy + barH,
            GetColor(80, 180, 220), TRUE);
        DrawBox(bx, yy, bx + barW, yy + barH,
            seld ? GetColor(255, 220, 60) : GetColor(140, 140, 140), FALSE);

        // 数値（％）
        wchar_t pct[16];
        swprintf_s(pct, L"%d", vols[i] * 100 / 255);
        DrawString(bx + barW + 12, yy, pct, GetColor(200, 200, 200));
    }
    SetFontSize(18);
    const wchar_t* g = L"\x2191\x2193:\x9078\x629E  \x2190\x2192:\x97F3\x91CF";
    const int gw = GetDrawStringWidth(g, static_cast<int>(wcslen(g)), FALSE);
    DrawString((SCREEN_W - gw) / 2, baseY + gap * 2 + 10, g, GetColor(150, 150, 150));
    SetFontSize(FONT_SIZE_DEFAULT);
}

// -------------------------------------------------------
// WinMain
// -------------------------------------------------------
#pragma warning(suppress: 28251)
/// <summary>エントリポイント。DxLib の初期化・メインループ・終了処理を担う。</summary>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    SetGraphMode(SCREEN_W, SCREEN_H, COLOR_DEPTH);
    ChangeWindowMode(TRUE);
    SetMainWindowText(L"BassenGame");
    SetAlwaysRunFlag(TRUE);

    if (DxLib_Init() == -1) return -1;
    srand(static_cast<unsigned int>(time(nullptr)));

    const int netHandle = LoadGraph(L"Assets\\Image\\BackNet.png");
    const int hintHandle = LoadGraph(L"Assets\\Hint\\Hint.png");

    SetDrawScreen(DX_SCREEN_BACK);

    AudioBank    audioBank;
    audioBank.Load();

    Player player;

    HRItemBank   hrItemBank;      
    hrItemBank.Load();

    HitCommentBank commentBank;
    commentBank.Load(); // 失敗してもゲームは続行（コメントが空になるだけ）
    

    Ball           ball;
    InputManager   inputManager;
    SwingJudge     swingJudge;
    LatencyOverlay latencyOverlay;
    PitchMachine   pitchMachine;
    Collection     collection;
    collection.Load(hrItemBank);
    StartLevel     startLevel;
    startLevel.Load();
    CharacterSelect charSelect;
    charSelect.Load();
    player.Load(charSelect.GetSelectedPlayerConfig());

    GameState      gameState        = GameState::START;
    CharaTrait charaTrait;   // 選択キャラの特性（CHAR_SELECT確定時に設定）
    audioBank.PlayBgm(0);

    SwingResult    lastResult       = SwingResult::NONE;
    float lastDistanceM = 0.0f;  // 直近のヒットの飛距離（m）
    const wchar_t* lastComment      = nullptr; // 直前のスイングで選ばれたコメント
    float          resultTimer      = 0.0f;    // 判定テキスト残り表示時間（秒）
    float          hrTimer          = 0.0f;    // HR 演出残り時間（秒）
    int            latestPrizeIndex = -1;      // 最後に解放された景品インデックス
    int            volSel = 0;   // 0=BGM, 1=SE
    float seTestTimer = 0.0f;   // SE試聴の間引き用
    bool  volPrevEnter = true;
    float          prizeDisplayTimer= 0.0f;   // 景品通知残り表示時間（秒）
    SwingPhase     swingPhase       = SwingPhase::IDLE;  // 打撃サブフェーズ
    float          lockedPower      = 0.0f;              // フェーズ1で確定したパワー量

    // dt 計算用タイマー
    LARGE_INTEGER timerFreq, timerNow;
    QueryPerformanceFrequency(&timerFreq);
    QueryPerformanceCounter(&timerNow);
    LONGLONG prevTick = timerNow.QuadPart;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_F4) == 0)
    {
        // ---- dt 計算 ----
        QueryPerformanceCounter(&timerNow);
        float deltaTime = static_cast<float>(timerNow.QuadPart - prevTick)
                        / static_cast<float>(timerFreq.QuadPart);
        if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
        prevTick = timerNow.QuadPart;

        // ---- 入力更新（キー状態の更新は毎フレーム必要）----
        inputManager.Update(deltaTime, charaTrait.chargeSpeedMul, charaTrait.chargeHoldSec);

        // スイング入力（チャージSE・攻撃アニメ）はプレイ中のみ処理する
        if (gameState == GameState::PLAYING)
        {
            // フェーズ1：IDLE → CHARGING（Space押した瞬間）
            if (swingPhase == SwingPhase::IDLE && inputManager.SwingJustPressed())
            {
                player.StartAttack();
                audioBank.PlayChargeSe();
                swingPhase = SwingPhase::CHARGING;
            }
            // フェーズ1終了：CHARGING → TIMING（Space離した瞬間＝パワー確定）
            if (swingPhase == SwingPhase::CHARGING && inputManager.SwingReleased())
            {
                lockedPower = inputManager.GetChargeRatio();
                swingPhase = SwingPhase::TIMING; // チャージSEはスイングまで鳴らし続ける
                pitchMachine.TriggerImmediatePitch(); // フェーズ2移行と同時に即投球
            }
        }

        // プレイヤーアニメはゲーム状態に関わらず毎フレーム更新
        // （GAME_OVER 画面でも Attack 残りを最後まで再生させる）
        if (gameState != GameState::PLAYING)
        {
            player.Update(deltaTime, inputManager.IsCharging());
        }

        // ---- タイマー更新 ----
        if (resultTimer > 0.0f)      resultTimer      -= deltaTime;
        if (hrTimer > 0.0f)          hrTimer          -= deltaTime;
        if (prizeDisplayTimer > 0.0f) prizeDisplayTimer -= deltaTime;

        // ================================================================
        // ゲームロジック
        // ================================================================
        switch (gameState)
        {
        case GameState::START:
        {
            const StartAction action = startLevel.Update();
            if (action == StartAction::PLAY)
            {
                gameState = GameState::CHAR_SELECT;
            }
            else if (action == StartAction::COLLECTION)
            {
                collection.ResetBrowse();
                gameState = GameState::COLLECTION;
            }
            else if (action == StartAction::VOLUME)
            {
                volSel = 0;
                volPrevEnter = true;
                gameState = GameState::VOLUME;
            }
            else if (action == StartAction::HINT)
            {
                gameState = GameState::HINT;
            }
            break;
        }
        case GameState::VOLUME:
        {
            if (CheckHitKey(KEY_INPUT_UP))   volSel = 0;
            if (CheckHitKey(KEY_INPUT_DOWN)) volSel = 1;

            int step = static_cast<int>(255 * deltaTime * 0.6f);  // ★速度を落とした(後述のFB対応)
            if (step < 1) step = 1;

            int cur = (volSel == 0) ? audioBank.GetBgmVolume() : audioBank.GetSeVolume();
            const int before = cur;
            if (CheckHitKey(KEY_INPUT_LEFT))  cur -= step;
            if (CheckHitKey(KEY_INPUT_RIGHT)) cur += step;
            cur = std::clamp(cur, 0, 255);

            if (cur != before)
            {
                if (volSel == 0) audioBank.SetBgmVolume(cur);
                else
                {
                    audioBank.SetSeVolume(cur);
                    seTestTimer -= deltaTime;
                    if (seTestTimer <= 0.0f)
                    {
                        audioBank.PlayRandomSe();
                        seTestTimer = 0.05f;
                    }
                }
            }

            // 戻る：BackSpace（押しっぱなしでOK）か、Enterの「押した瞬間」
            const bool enterNow = (CheckHitKey(KEY_INPUT_RETURN) != 0);
            const bool enterTrig = !volPrevEnter && enterNow;   // 前OFF→今ON
            volPrevEnter = enterNow;
            if (CheckHitKey(KEY_INPUT_ESCAPE) || enterTrig)
                gameState = GameState::START;

            break;   // ← もともとある break はそのまま残す
        }
        case GameState::HINT:
            if (CheckHitKey(KEY_INPUT_ESCAPE))
                gameState = GameState::START;   // BackSpace で戻る
            break;
        case GameState::COLLECTION:
            if (collection.UpdateBrowse())          // BackSpace で true
            {
                gameState = GameState::START;
                audioBank.PlayBgm(0);   // タイトルBGMに戻す
            }
            break;
        case GameState::CHAR_SELECT:
        {
            if (charSelect.Update(deltaTime))
            {
                audioBank.LoadSeForCharacter(charSelect.GetSelectedSeDir());
                player.Load(charSelect.GetSelectedPlayerConfig());
                charaTrait = CharaTrait::FromIndex(charSelect.GetSelectedIndex());  // ★特性決定
                gameState = GameState::PLAYING;
                audioBank.PlayBgm(1);
            }
            break;
        }
        case GameState::PLAYING:
        {
            // ---- 投球マシン更新 ----
            // フェーズ2（TIMING）になるまで投球しない
            const bool canPitch = (swingPhase == SwingPhase::TIMING);
            pitchMachine.Update(deltaTime, ball, canPitch);

            // ---- スイング / 見逃し処理（ball.Update() より前に判定する）----
            // ball.Update() 内で INCOMING → INACTIVE の自動遷移が起きるため、
            // その前に状態を確認しなければ見逃し検知が機能しない。
            // フェーズ2：TIMING 中に再度 Space を押した瞬間＝タイミング確定
            if (ball.IsIncoming() && swingPhase == SwingPhase::TIMING && inputManager.SwingJustPressed())
            {
                latencyOverlay.RecordInput();
                const float timingOffset = ball.GetTimingOffset();
                const float powerRatio   = lockedPower;  // フェーズ1で確定済みのパワーを使用
                const bool  isPowerRed   = (powerRatio >= InputManager::RED_POWER_THRESHOLD);
                swingPhase = SwingPhase::IDLE;
                lockedPower = 0.0f;
                audioBank.StopChargeSe();

                lastResult = swingJudge.Judge(timingOffset, charaTrait.perfectWidthMul);
                resultTimer = RESULT_DISPLAY_SEC;
                lastWasMissLook = false;

                if (lastResult == SwingResult::MISS)
                {
                    // 空振り（パワーに関わらず）
                    collection.RecordMiss();
                    lastComment = commentBank.GetRandomComment(SwingResult::MISS);
                    audioBank.Play(SwingResult::MISS, latencyOverlay);
                }
                else if (powerRatio < InputManager::MIN_HIT_POWER)
                {
                    // タイミングは合っているがパワー不足 → ヒットにしない
                    lastResult  = SwingResult::WEAK_POWER;
                    lastComment = commentBank.GetRandomComment(SwingResult::WEAK_POWER);
                    collection.RecordMiss();
                    audioBank.Play(SwingResult::WEAK_POWER, latencyOverlay);
                    // ボールは通過させる（ApplyHit を呼ばない）
                }
                else
                {
                    // パワー赤 + 良タイミング → 正規のヒット
                    lastComment = commentBank.GetRandomComment(lastResult);
                    const float launchAngle = swingJudge.CalcLaunchAngleDeg(timingOffset);
                    ball.ApplyHit(launchAngle, powerRatio, lastResult,
                        charaTrait.hrBonusPercent, charaTrait.hrOnOrange);
                    lastDistanceM = ball.GetHitDistanceMeters();
                    collection.AddDistance(lastDistanceM);
                    collection.RecordHit(lastResult);

                    // HR 確定時は HomeRun.mp3 のみ、それ以外は Hit.mp3
                    if (ball.IsHomeRun())
                    {
                        latencyOverlay.RecordSound();
                        audioBank.PlayHomeRun();
                    }
                    else
                    {
                        audioBank.Play(lastResult, latencyOverlay);
                    }
                }
            }
            

            // ---- ボール更新 ----
            ball.Update(deltaTime);

            // ---- 見逃し（Update内で確定したフレームを拾う）----
            if (ball.ConsumeJustMissed())
            {
                swingPhase = SwingPhase::IDLE;
                lockedPower = 0.0f;
                audioBank.StopChargeSe();
                collection.RecordMiss();
                lastResult = SwingResult::MISS;
                lastComment = L"Spaceキーを押してスイング！";
                lastWasMissLook = true;            // ★見逃し → 見出しなし
                resultTimer = RESULT_DISPLAY_SEC;
            }

            // ---- HR 確定チェック ----
            if (ball.IsHit() && ball.IsHomeRun() && hrTimer <= 0.0f)
            {
                hrTimer = HR_DISPLAY_SEC;
                latestPrizeIndex = collection.RecordHomeRun();
                if (latestPrizeIndex >= 0)
                {
                    prizeDisplayTimer = 4.0f;
                }
            }

            // ---- プレイヤーアニメ更新 ----
            // CHARGING中はSpace押し状態でループ、TIMING中もパワーが確定するまでループ継続
            const bool isChargingForAnim =
                (swingPhase == SwingPhase::CHARGING && inputManager.IsCharging())
                || (swingPhase == SwingPhase::TIMING);
            player.Update(deltaTime, isChargingForAnim);

            // セッション終了チェック
            if (pitchMachine.IsSessionOver() && !ball.IsActive())
            {
                gameState = GameState::GAME_OVER;

                audioBank.StopBgm();

                audioBank.PlayResultBgm();
            }
            break;
        }

        case GameState::GAME_OVER:
            // R でスタート画面へ戻る
            if (CheckHitKey(KEY_INPUT_R))
            {
                ball.Reset();
                pitchMachine.Reset();
                collection.ResetStats();
                lastResult = SwingResult::NONE;
                lastComment = nullptr;
                resultTimer = 0.0f;
                hrTimer = 0.0f;
                swingPhase = SwingPhase::IDLE;
                lockedPower = 0.0f;
                audioBank.StopBgm();
                audioBank.PlayBgm(0);
                gameState = GameState::START;
            }
            // それ以外のキーでリスタート（従来どおり）
            else if (inputManager.AnyKeyTriggered())
            {
                ball.Reset();
                pitchMachine.Reset();
                collection.ResetStats();
                lastResult = SwingResult::NONE;
                lastComment = nullptr;
                resultTimer = 0.0f;
                hrTimer = 0.0f;
                swingPhase = SwingPhase::IDLE;
                lockedPower = 0.0f;
                gameState = GameState::PLAYING;
                audioBank.PlayBgm(1);
            }
            break;
        }

        // ================================================================
        // 描画
        // ================================================================
        ClearDrawScreen();

        if (gameState == GameState::START)            
        {
            startLevel.Draw();
        }
        else if (gameState == GameState::CHAR_SELECT)
        {
            charSelect.Draw();
        }
        else if (gameState == GameState::COLLECTION)   
            collection.DrawCollectionBrowse();
        else if (gameState == GameState::VOLUME)
        {
            DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(20, 40, 80), TRUE);
            SetFontSize(40);
            DrawString(80, 80, L"音量設定", GetColor(255, 220, 60));
            DrawVolumeUI(audioBank, volSel);   // 既存のヘルパーを流用
            SetFontSize(18);
            DrawString(80, SCREEN_H - 50, L"Esc でもどる", GetColor(150, 150, 150));
            SetFontSize(16);
        }
        else if (gameState == GameState::HINT)
        {
            DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(10, 10, 20), TRUE);
            if (hintHandle != -1)
            {
                int iw, ih; GetGraphSize(hintHandle, &iw, &ih);
                const float scale = (std::min)((SCREEN_W * 0.9f) / iw, (SCREEN_H * 0.85f) / ih);
                const int dw = static_cast<int>(iw * scale);
                const int dh = static_cast<int>(ih * scale);
                DrawExtendGraph((SCREEN_W - dw) / 2, (SCREEN_H - dh) / 2 - 20,
                    (SCREEN_W + dw) / 2, (SCREEN_H + dh) / 2 - 20,
                    hintHandle, TRUE);
            }
            SetFontSize(18);
            DrawString(80, SCREEN_H - 40, L"Esc でもどる", GetColor(160, 160, 160));
            SetFontSize(16);
        }
        else                                     
        {

            DrawBackground(netHandle);
            // TIMING フェーズではパワーを lockedPower で固定表示
            const float displayPower = (swingPhase == SwingPhase::TIMING)
                ? lockedPower : inputManager.GetChargeRatio();
            player.Draw(displayPower);
            ball.Draw();
            swingJudge.DrawWindow(ball);
            if (!charaTrait.hidePowerGauge)
                DrawPowerMeter(displayPower, swingPhase == SwingPhase::CHARGING);

            // 操作案内テキスト（飛距離表示と同じ位置に擬似ボールドで表示）
            // 判定テキストが表示されていないときだけ出す
            if (resultTimer <= 0.0f)
            {
                const wchar_t* guideText = nullptr;
                unsigned int   guideColor = 0;
                if (swingPhase == SwingPhase::IDLE || swingPhase == SwingPhase::CHARGING)
                {
                    guideText  = L"Spaceキーを押してパワーチャージ！";
                    guideColor = GetColor(255, 220, 60);
                }
                else if (swingPhase == SwingPhase::TIMING)
                {
                    guideText  = L"Spaceでスイング！";
                    guideColor = GetColor(100, 220, 255);
                }

                if (guideText != nullptr)
                {
                    SetFontSize(25);
                    const int tw = GetDrawStringWidth(guideText,
                        static_cast<int>(wcslen(guideText)), FALSE);
                    const int tx = (SCREEN_W - tw) / 2;
                    // 影
                    DrawString(tx + RESULT_SHADOW_OFFSET, RESULT_TEXT_Y + RESULT_SHADOW_OFFSET,
                        guideText, GetColor(0, 0, 0));
                    // 本体を 1px ずらして重ねて擬似ボールド
                    DrawString(tx,     RESULT_TEXT_Y, guideText, guideColor);
                    DrawString(tx + 1, RESULT_TEXT_Y, guideText, guideColor);
                    SetFontSize(FONT_SIZE_DEFAULT);
                }
            }
            DrawResultText(lastResult, resultTimer / RESULT_DISPLAY_SEC,
                lastComment, lastDistanceM, lastWasMissLook);
            DrawHrEffect(hrTimer);
            DrawPrizeUnlock(collection.GetItemName(latestPrizeIndex), prizeDisplayTimer / 4.0f);
            pitchMachine.Draw();
            /*collection.DrawHud();
            latencyOverlay.Draw();*/

            // コレクション画面（ゲームオーバー時に上に重ねる）
            if (gameState == GameState::GAME_OVER)
            {
                collection.DrawCollectionScreen();

                // リスタート / 戻る のヒント
                SetFontSize(FONT_SIZE_GUIDE);
                const wchar_t* line1 = L"SPACE でもう一度";
                const wchar_t* line2 = L"R でスタート画面へ";
                const int w1 = GetDrawStringWidth(line1, static_cast<int>(wcslen(line1)), FALSE);
                const int w2 = GetDrawStringWidth(line2, static_cast<int>(wcslen(line2)), FALSE);
                const unsigned int col = GetColor(
                    COLOR_GUIDE_TEXT.r, COLOR_GUIDE_TEXT.g, COLOR_GUIDE_TEXT.b);
                DrawString((SCREEN_W - w1) / 2, SCREEN_H - 90, line1, col);
                DrawString((SCREEN_W - w2) / 2, SCREEN_H - 60, line2, col);
                SetFontSize(FONT_SIZE_DEFAULT);

            }
        }
        // ESC ヒント
        SetFontSize(FONT_SIZE_HINT);
        DrawString(SCREEN_W - HINT_OFFSET_FROM_RIGHT, SCREEN_H - HINT_OFFSET_FROM_BOTTOM,
                   L"F4: Quit",
                   GetColor(COLOR_HINT_TEXT.r, COLOR_HINT_TEXT.g, COLOR_HINT_TEXT.b));
        SetFontSize(FONT_SIZE_DEFAULT);

        ScreenFlip();
    }

    DxLib::DeleteGraph(netHandle);
    DxLib::DeleteGraph(hintHandle);
    DxLib::DxLib_End();
    return 0;
}
