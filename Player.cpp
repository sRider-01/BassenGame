#include "DxLib.h"
#include "Player.h"
#include <cwchar>   // swprintf_s
#include <cmath>

// シート画像の実寸から1枚の幅を自動計算して分割ロード（横1列前提）
static void LoadSheetAuto(const wchar_t* path, int frameCount,
    int fallbackW, int fallbackH, int* outHandles)
{
    int fw = fallbackW, fh = fallbackH;
    const int tmp = LoadGraph(path);
    if (tmp != -1)
    {
        int sheetW = 0, sheetH = 0;
        GetGraphSize(tmp, &sheetW, &sheetH);
        DeleteGraph(tmp);
        if (frameCount > 0 && sheetW > 0) fw = sheetW / frameCount; // 実幅 ÷ 枚数
        if (sheetH > 0)                   fh = sheetH;
    }
    LoadDivGraph(path, frameCount, frameCount, 1, fw, fh, outHandles);
}

Player::Player()
    : state_(AnimState::IDLE)
    , currentFrame_(0)
    , frameTimer_(0.0f)
    , auraTimer_(0.0f)
    , idleFrameCount_(0)
    , attackFrameCount_(0)
    , drawScale_(DRAW_SCALE)
{
}

Player::~Player()
{
    Unload();
}

int Player::LoadFrame(const wchar_t* dir, const wchar_t* prefix, int index)
{
    wchar_t path[512];
    swprintf_s(path, L"%s%s%05d.png", dir, prefix, index);
    return LoadGraph(path);
}

void Player::Unload()
{
    for (int h : idleHandles_)   if (h != -1) DeleteGraph(h);
    for (int h : attackHandles_) if (h != -1) DeleteGraph(h);
    idleHandles_.clear();
    attackHandles_.clear();
}

void Player::Load(const PlayerSpriteConfig& config)
{
    Unload();

    idleFrameCount_ = config.idleFrameCount;
    attackFrameCount_ = config.attackFrameCount;
    drawScale_ = config.drawScale;

    idleHandles_.assign(idleFrameCount_, -1);
    if (config.idleIsSpriteSheet)
        LoadSheetAuto(config.idleDir, idleFrameCount_,
                      config.idleFrameW, config.idleFrameH, idleHandles_.data());
    else
        for (int i = 0; i < idleFrameCount_; ++i)
            idleHandles_[i] = LoadFrame(config.idleDir, config.idlePrefix, i);

    attackHandles_.assign(attackFrameCount_, -1);
    if (config.attackIsSpriteSheet)
        LoadSheetAuto(config.attackDir, attackFrameCount_,
                      config.attackFrameW, config.attackFrameH, attackHandles_.data());
    else
        for (int i = 0; i < attackFrameCount_; ++i)
            attackHandles_[i] = LoadFrame(config.attackDir, config.attackPrefix, i);

    // 溜めループ範囲をクランプ（0 <= start <= end <= 最終フレーム）
    if (attackFrameCount_ > 0)
    {
        chargeLoopEnd_ = std::min(std::max(config.chargeLoopEnd, 0), attackFrameCount_ - 1);
        chargeLoopStart_ = std::min(std::max(config.chargeLoopStart, 0), chargeLoopEnd_);
    }
    else
    {
        chargeLoopStart_ = chargeLoopEnd_ = 0;
    }
}

void Player::StartAttack()
{
    state_        = AnimState::ATTACK;
    currentFrame_ = 0;
    frameTimer_   = 1.0f / ATTACK_FPS; // フレーム 0 を最初の 1 インターバル分表示してから進める
}

int Player::GetCurrentHandle() const
{
    const std::vector<int>& frames =
        (state_ == AnimState::ATTACK) ? attackHandles_ : idleHandles_;
    if (frames.empty()) return -1;

    int idx = currentFrame_;
    if (idx < 0) idx = 0;
    if (idx >= static_cast<int>(frames.size()))
        idx = static_cast<int>(frames.size()) - 1;   // 範囲外で消えないようクランプ

    int h = frames[idx];
    if (h == -1 && !idleHandles_.empty())
        h = idleHandles_[0];                          // 万一ロード失敗でも消さない
    return h;
}

void Player::Update(float deltaTime, bool isChargingSpace)
{
    if (state_ == AnimState::IDLE)
    {
        // Idle：ループ再生
        const float interval = 1.0f / IDLE_FPS;
        frameTimer_ -= deltaTime;
        while (frameTimer_ <= 0.0f)
        {
            frameTimer_ += interval;
            ++currentFrame_;
            if (currentFrame_ >= idleFrameCount_)
                currentFrame_ = 0;
        }
        return;
    }

    // ---- Attack 状態 ----
    // 溜め中（ループ中）は遅いFPS、離した後の本再生は通常FPS
    const float interval = 1.0f / (isChargingSpace ? CHARGE_LOOP_FPS : ATTACK_FPS);
    frameTimer_ -= deltaTime;

    while (frameTimer_ <= 0.0f)
    {
        frameTimer_ += interval;
        ++currentFrame_;

        if (isChargingSpace)
        {
            if (currentFrame_ > chargeLoopEnd_)
                currentFrame_ = chargeLoopStart_;
        }
        else
        {
            if (currentFrame_ >= attackFrameCount_)
            {
                state_ = AnimState::IDLE;
                currentFrame_ = 0;
                frameTimer_ = 1.0f / IDLE_FPS;
                break;
            }
        }
    }

    auraTimer_ += deltaTime;
}

void Player::Draw(float chargeRatio) const
{
    const int handle = GetCurrentHandle();
    if (handle == -1) return;

    const int    cx = DRAW_CENTER_X;
    const int    cy = DRAW_CENTER_Y;
    const double sc = static_cast<double>(drawScale_);
    const int    flip = FLIP_HORIZONTAL ? TRUE : FALSE;

    // ハロー（輪郭発光）：スプライトをずらして ADD ブレンドで重ねる
    // スプライト画像の alpha を使うので背景に滲まない
    if (chargeRatio > 0.0f)
    {
        static constexpr int   HALO_STEPS = 16;
        static constexpr float HALO_MAX_RADIUS = 24.0f;
        static constexpr int   HALO_LAYERS = 3;

        // 全体の明滅パルス
        const float pulse = 1.0f + 0.25f * sinf(auraTimer_ * 10.0f);
        const int   perAlpha = static_cast<int>(
            chargeRatio * TINT_MAX_ALPHA / HALO_STEPS * 5 * pulse);

        SetDrawBright(TINT_COLOR_R, TINT_COLOR_G, TINT_COLOR_B);
        SetDrawBlendMode(DX_BLENDMODE_ADD, perAlpha);

        for (int layer = 1; layer <= HALO_LAYERS; ++layer)
        {
            const float baseRadius = chargeRatio * HALO_MAX_RADIUS * layer / HALO_LAYERS;

            for (int i = 0; i < HALO_STEPS; ++i)
            {
                const float baseAngle = 2.0f * 3.14159265f * i / HALO_STEPS;

                // サンプルごとに位相をずらしたゆらぎ（炎の不規則な形状）
                const float flicker = 1.0f + 0.2f * sinf(
                    auraTimer_ * 10.0f + i * 1.7f + layer * 2.3f);

                // 上方向への流れ（気が上へ昇る）
                const float upDrift = chargeRatio * 15.0f
                    * sinf(auraTimer_ * 7.0f + i * 0.9f + layer);

                const int dx = static_cast<int>(baseRadius * flicker * cosf(baseAngle));
                const int dy = static_cast<int>(baseRadius * flicker * sinf(baseAngle) - upDrift);
                DrawRotaGraph(cx + dx, cy + dy, sc, 0.0, handle, TRUE, flip);
            }
        }

        SetDrawBright(255, 255, 255);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // 本体を最後に通常描画
    DrawRotaGraph(cx, cy, sc, 0.0, handle, TRUE, flip);
}




