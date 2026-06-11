#include "DxLib.h"
#include "SwingJudge.h"
#include <cmath>
#include <algorithm>

SwingResult SwingJudge::Judge(float offset, float perfectWidthMul) const
{
    const float absOffset = fabsf(offset);
    if (absOffset < WIN_PERFECT_SEC * perfectWidthMul) return SwingResult::PERFECT;  // ★倍率
    if (absOffset < WIN_SWEET_SEC)   return SwingResult::SWEET;
    if (absOffset < WIN_NORMAL_SEC)  return SwingResult::NORMAL;
    if (absOffset < WIN_GRAZE_SEC)   return SwingResult::GRAZE;
    return SwingResult::MISS;
}

float SwingJudge::CalcLaunchAngleDeg(float offset) const
{
    // offset を [-WIN_GRAZE_SEC, +WIN_GRAZE_SEC] に正規化して角度に線形マッピング
    // offset < 0（早い）→ 低弾道、offset > 0（遅い）→ 高弾道
    const float normalizedOffset =
        std::clamp(offset, -WIN_GRAZE_SEC, WIN_GRAZE_SEC) / WIN_GRAZE_SEC;

    // normalizedOffset ∈ [-1, +1]
    // -1 → LAUNCH_ANGLE_EARLY_DEG（低）
    //  0 → LAUNCH_ANGLE_PERFECT_DEG（理想）
    // +1 → LAUNCH_ANGLE_LATE_DEG（高）
    const float angleDeg = LAUNCH_ANGLE_PERFECT_DEG
        + normalizedOffset * (normalizedOffset > 0.0f
            ? (LAUNCH_ANGLE_LATE_DEG    - LAUNCH_ANGLE_PERFECT_DEG)
            : (LAUNCH_ANGLE_PERFECT_DEG - LAUNCH_ANGLE_EARLY_DEG));

    return angleDeg;
}

void SwingJudge::DrawWindow(const Ball& ball) const
{
    auto SecondToPixelX = [&](float seconds) -> int
    {
        return BAR_CENTER_X + static_cast<int>(seconds / BAR_HALF_RANGE_SEC * (BAR_WIDTH / 2));
    };

    const int barLeft = BAR_CENTER_X - BAR_WIDTH  / 2;
    const int barRight= BAR_CENTER_X + BAR_WIDTH  / 2;
    const int barTop  = BAR_CENTER_Y - BAR_HEIGHT / 2;
    const int barBot  = BAR_CENTER_Y + BAR_HEIGHT / 2;

    DrawBox(barLeft, barTop, barRight, barBot,
            GetColor(BAR_COLOR_MISS.r,    BAR_COLOR_MISS.g,    BAR_COLOR_MISS.b),    TRUE);
    DrawBox(SecondToPixelX(-WIN_GRAZE_SEC),   barTop,
            SecondToPixelX( WIN_GRAZE_SEC),   barBot,
            GetColor(BAR_COLOR_GRAZE.r,   BAR_COLOR_GRAZE.g,   BAR_COLOR_GRAZE.b),   TRUE);
    DrawBox(SecondToPixelX(-WIN_NORMAL_SEC),  barTop,
            SecondToPixelX( WIN_NORMAL_SEC),  barBot,
            GetColor(BAR_COLOR_NORMAL.r,  BAR_COLOR_NORMAL.g,  BAR_COLOR_NORMAL.b),  TRUE);
    DrawBox(SecondToPixelX(-WIN_SWEET_SEC),   barTop,
            SecondToPixelX( WIN_SWEET_SEC),   barBot,
            GetColor(BAR_COLOR_SWEET.r,   BAR_COLOR_SWEET.g,   BAR_COLOR_SWEET.b),   TRUE);
    DrawBox(SecondToPixelX(-WIN_PERFECT_SEC), barTop,
            SecondToPixelX( WIN_PERFECT_SEC), barBot,
            GetColor(BAR_COLOR_PERFECT.r, BAR_COLOR_PERFECT.g, BAR_COLOR_PERFECT.b), TRUE);
    DrawBox(barLeft, barTop, barRight, barBot,
            GetColor(BAR_COLOR_OUTLINE.r, BAR_COLOR_OUTLINE.g, BAR_COLOR_OUTLINE.b), FALSE);

    // カーソル（INCOMING のみ表示）
    if (ball.IsIncoming())
    {
        const float clampedOffset =
            std::clamp(ball.GetTimingOffset(), -BAR_HALF_RANGE_SEC, BAR_HALF_RANGE_SEC);
        const int cursorX = SecondToPixelX(clampedOffset);
        DrawBox(cursorX - CURSOR_HALF_W, barTop - CURSOR_EXTEND,
                cursorX + CURSOR_HALF_W, barBot + CURSOR_EXTEND,
                GetColor(BAR_COLOR_CURSOR.r, BAR_COLOR_CURSOR.g, BAR_COLOR_CURSOR.b), TRUE);
    }

    DrawString(barLeft - 2, barBot + LABEL_OFFSET_Y,
               L"\x65E9\x3044",                                          // 早い
               GetColor(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b));
    DrawString(barRight + LABEL_LATE_X_OFFSET, barBot + LABEL_OFFSET_Y,
               L"\x9045\x3044",                                          // 遅い
               GetColor(LABEL_COLOR.r, LABEL_COLOR.g, LABEL_COLOR.b));
    DrawString(BAR_CENTER_X - 4, barBot + LABEL_OFFSET_Y, L"|",
               GetColor(BAR_COLOR_CURSOR.r, BAR_COLOR_CURSOR.g, BAR_COLOR_CURSOR.b));
}
