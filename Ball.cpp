#include "DxLib.h"
#include "Ball.h"
#include "InputManager.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>  // rand()

static constexpr float PI = 3.14159265f;

Ball::Ball()
{
    Reset();
}

void Ball::Launch()
{
    state_            = BallState::INCOMING;
    x_                = START_X;
    y_                = BALL_Y;
    vx_               = 0.0f;
    vy_               = 0.0f;
    incomingElapsed_  = 0.0f;
    homeRunConfirmed_ = false;
    trailHead_        = 0;
    trailCount_       = 0;
}

void Ball::ApplyHit(float launchAngleDeg, float powerRatio, SwingResult result,
    int hrBonusPercent, bool hrOnOrange)
{
    const float clampedPower = std::clamp(powerRatio, 0.0f, 1.0f);
    const float hitSpeed     = HIT_SPEED_MIN + clampedPower * (HIT_SPEED_MAX - HIT_SPEED_MIN);
    const float angleRad     = launchAngleDeg * PI / 180.0f;

    // 打球方向：右向き(+X)、上向き(-Y)
    vx_ = hitSpeed * cosf(angleRad);
    vy_ = -hitSpeed * sinf(angleRad);

    // 着地までの飛距離を計算してメートル換算で保持
    // 打点(BALL_Y)から地面(GROUND_Y)へ落ちるまでの時間 t を解き、水平距離 = vx_ * t
    const float dropPx = GROUND_Y - BALL_Y;                    // 落差(px)=100
    const float disc = vy_ * vy_ + 2.0f * GRAVITY * dropPx;
    const float tLand = (-vy_ + sqrtf(disc)) / GRAVITY;       // 着地までの時間(秒)
    hitDistanceM_ = vx_ * tLand * METERS_PER_PX;               // 水平距離→m

    // ホームラン判定（隠し要素）
    // 条件1：パワーが赤ゾーン（RED_POWER_THRESHOLD 以上）が必須
    // 条件2：タイミング判定ごとに異なる確率で乱択
    //   PERFECT（黄）: 60%  SWEET（オレンジ）: 30%  NORMAL（緑）: 5%
    //   GRAZE / MISS は対象外

    const bool isPowerRed = (clampedPower >= InputManager::RED_POWER_THRESHOLD);

    int hrProbabilityPercent = 0;
    if (isPowerRed)
    {
        switch (result)
        {
        case SwingResult::PERFECT: hrProbabilityPercent = HR_PROB_PERCENT_PERFECT; break;
        case SwingResult::SWEET:   hrProbabilityPercent = HR_PROB_PERCENT_SWEET;   break;
        case SwingResult::NORMAL:  hrProbabilityPercent = HR_PROB_PERCENT_NORMAL;  break;
        default:                   hrProbabilityPercent = 0;                        break;
        }
    }

    // 老師特性：オレンジ(SWEET)パワーでもHR抽選を許可
    // ＝ 赤でなくても SWEET 判定なら最低限の確率を与える
    if (!isPowerRed && hrOnOrange && result == SwingResult::SWEET)
    {
        hrProbabilityPercent = HR_PROB_PERCENT_SWEET;  // 赤と同等のSWEET確率を付与
    }

    // 老師特性：HR確率に一律ボーナス（抽選対象のときのみ）
    if (hrProbabilityPercent > 0)
    {
        hrProbabilityPercent += hrBonusPercent;
    }

    homeRunConfirmed_ = (hrProbabilityPercent > 0)
        && ((rand() % 100) < hrProbabilityPercent);

    // ホームランは飛距離に 1.5倍 ボーナス
    if (homeRunConfirmed_)
    {
        hitDistanceM_ *= 1.5f;
    }

    state_ = BallState::HIT;
}

void Ball::Update(float deltaTime)
{
    switch (state_)
    {
    case BallState::INCOMING:
    {
        incomingElapsed_ += deltaTime;
        spinAngle_ += SPIN_SPEED * deltaTime;   // 毎フレーム回す
        const float normalizedTime = incomingElapsed_ / PITCH_DURATION;
        x_ = START_X + (HIT_ZONE_X - START_X) * normalizedTime;
        y_ = BALL_Y;
        break;
    }

    case BallState::HIT:
    {
        // 残像を記録（リングバッファ）
        trailBuf_[trailHead_] = { x_, y_ };
        trailHead_  = (trailHead_ + 1) % TRAIL_LENGTH;
        trailCount_ = std::min(trailCount_ + 1, TRAIL_LENGTH);

        // 放物線物理
        x_ += vx_ * deltaTime;
        y_ += vy_ * deltaTime;
        vy_ += GRAVITY * deltaTime;
        break;
    }

    case BallState::PASSED:
    {
        incomingElapsed_ += deltaTime;
        // 通過後もボールが飛んでいく演出（速度を少し落として表示）
        x_ -= (START_X - HIT_ZONE_X) / PITCH_DURATION * deltaTime * 0.5f;
        break;
    }

    case BallState::INACTIVE:
        break;
    }

    // 着地 or 画面外で演出終了
    if (state_ == BallState::HIT && IsFinished())
    {
        state_ = BallState::INACTIVE;
    }

    // PASSED の猶予経過で INACTIVE に
    if (state_ == BallState::PASSED && HasPassed())
    {
        state_ = BallState::INACTIVE;
    }

    // 見逃し：INCOMING のまま猶予時間を超えたら、見逃しを通知して INACTIVE に
    if (state_ == BallState::INCOMING && incomingElapsed_ > PITCH_DURATION + PASS_GRACE_SEC)
    {
        justMissed_ = true;             // ★見逃しが確定した瞬間に通知
        state_ = BallState::INACTIVE;
    }
}

void Ball::Draw() const
{
    if (state_ == BallState::INACTIVE) return;

    // 打球残像
    if (state_ == BallState::HIT && trailCount_ > 0)
    {
        for (int i = 0; i < trailCount_; ++i)
        {
            const int    idx   = (trailHead_ - 1 - i + TRAIL_LENGTH) % TRAIL_LENGTH;
            const float  alpha = 1.0f - static_cast<float>(i + 1) / (TRAIL_LENGTH + 1);
            const int    r     = static_cast<int>(COLOR_HIT_TRAIL.r * alpha);
            const int    g     = static_cast<int>(COLOR_HIT_TRAIL.g * alpha);
            const int    b     = static_cast<int>(COLOR_HIT_TRAIL.b * alpha);
            const int    rad   = std::max(2, static_cast<int>(BALL_RADIUS * alpha * 0.6f));
            DrawCircle(
                static_cast<int>(trailBuf_[idx].x),
                static_cast<int>(trailBuf_[idx].y),
                rad, GetColor(r, g, b), TRUE);
        }
    }

    const int centerX = static_cast<int>(x_);
    const int centerY = static_cast<int>(y_);
    const int radius  = static_cast<int>(BALL_RADIUS);

    // 影（INCOMING のみ）
    if (state_ == BallState::INCOMING)
    {
        DrawCircle(
            centerX + SHADOW_OFFSET_X,
            centerY + SHADOW_OFFSET_Y,
            radius,
            GetColor(COLOR_SHADOW.r, COLOR_SHADOW.g, COLOR_SHADOW.b), TRUE);
    }

    // ボール本体
    const Rgb bodyColor = (state_ == BallState::HIT)
        ? Rgb{ 255, 230, 100 }   // 打球は明るい黄色に
        : COLOR_BODY;
    DrawCircle(centerX, centerY, radius,
               GetColor(bodyColor.r, bodyColor.g, bodyColor.b), TRUE);

    // 縫い目（INCOMING のみ・回転表示）
    if (state_ == BallState::INCOMING)
    {
        DrawCircle(centerX, centerY, radius,
            GetColor(COLOR_SEAM.r, COLOR_SEAM.g, COLOR_SEAM.b), FALSE);

        // 回転する縫い目を2本（90度ずらしで十字）
        const float r = static_cast<float>(radius) * 0.8f;
        for (int k = 0; k < 2; ++k)
        {
            const float a = spinAngle_ + k * 3.14159265f / 2.0f; // 0度と90度
            const int dx = static_cast<int>(cosf(a) * r);
            const int dy = static_cast<int>(sinf(a) * r);
            DrawLine(centerX - dx, centerY - dy,
                centerX + dx, centerY + dy,
                GetColor(COLOR_SEAM.r, COLOR_SEAM.g, COLOR_SEAM.b));
        }
    }
}

void Ball::Reset()
{
    state_            = BallState::INACTIVE;
    x_                = START_X;
    y_                = BALL_Y;
    vx_               = 0.0f;
    vy_               = 0.0f;
    incomingElapsed_  = 0.0f;
    homeRunConfirmed_ = false;
    trailHead_        = 0;
    trailCount_       = 0;
    spinAngle_ = 0.0f;
    justMissed_ = false;
}

bool Ball::HasPassed() const
{
    // 見逃し：ヒットゾーンを PASS_GRACE_SEC 以上通り過ぎた
    return (state_ == BallState::PASSED || state_ == BallState::INCOMING)
        && (incomingElapsed_ > PITCH_DURATION + PASS_GRACE_SEC);
}

bool Ball::IsFinished() const
{
    switch (state_)
    {
    case BallState::HIT:
        return (x_ > 1280.0f)            // 右端を越えた
            || (x_ <  -100.0f)           // 左端（まず起きない）
            || (y_ >  GROUND_Y);         // 地面に落ちた
    case BallState::PASSED:
        return HasPassed();
    case BallState::INACTIVE:
        return true;
    default:
        return false;
    }
}

float Ball::GetTimingOffset() const
{
    const float pxPerSec = (START_X - HIT_ZONE_X) / PITCH_DURATION;
    return incomingElapsed_ - PITCH_DURATION + PERFECT_X_OFFSET / pxPerSec;
}

