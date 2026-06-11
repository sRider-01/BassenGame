#include "DxLib.h"
#include "PitchMachine.h"
#include <cwchar>

// 球数インジケーターの1球あたりの表示サイズ
static constexpr int BALL_ICON_RADIUS  = 7;   ///< 球アイコンの半径
static constexpr int BALL_ICON_SPACING = 20;  ///< 球アイコンの間隔

PitchMachine::PitchMachine()
{
    Reset();
}

void PitchMachine::Reset()
{
    remainingBalls_ = TOTAL_BALLS;
    waitTimer_      = INITIAL_PITCH_DELAY;
    sessionOver_    = false;
}

void PitchMachine::Update(float deltaTime, Ball& ball, bool canPitch)
{
    if (sessionOver_) return;

    if (!ball.IsActive())
    {
        // 残り球0はcanPitchに関わらず即座にセッション終了とする
        if (remainingBalls_ == 0)
        {
            sessionOver_ = true;
            return;
        }

        // 投球許可時のみタイマーを進めて投球する（フェーズ1中は凍結）
        if (canPitch)
        {
            waitTimer_ -= deltaTime;

            if (waitTimer_ <= 0.0f)
            {
                ball.Launch();
                --remainingBalls_;
                waitTimer_ = INTER_PITCH_DELAY;
            }
        }
    }
}

bool PitchMachine::IsSessionOver() const
{
    return sessionOver_;
}

void PitchMachine::Draw() const
{
    // 「残り NN球」ラベル
    wchar_t buf[32];
    swprintf_s(buf, L"\x6B8B\x308A %d\x7403", remainingBalls_);  // 「残り N球」
    DrawString(DISPLAY_X, DISPLAY_Y - 20, buf,
               GetColor(COLOR_LABEL.r, COLOR_LABEL.g, COLOR_LABEL.b));

    // 球数アイコン（横並び）
    const int totalDisplayed = TOTAL_BALLS;
    for (int i = 0; i < totalDisplayed; ++i)
    {
        const int iconX = DISPLAY_X + BALL_ICON_RADIUS + i * BALL_ICON_SPACING;
        const int iconY = DISPLAY_Y;

        // 残り球 = アクティブ（明るい）、使用済み = 暗い
        const bool isRemaining = (i < remainingBalls_);
        const Rgb& color = isRemaining ? COLOR_BALL_ACTIVE : COLOR_BALL_USED;
        DrawCircle(iconX, iconY, BALL_ICON_RADIUS,
                   GetColor(color.r, color.g, color.b), TRUE);
    }
}
