#include "DxLib.h"
#include "InputManager.h"
#include <cmath>

InputManager::InputManager()
    : prevHeld_(false)
    , currentlyHeld_(false)
    , swingReleased_(false)
    , anyKeyTriggered_(false)
    , chargeTime_(0.0f)
{
}

void InputManager::Update(float deltaTime, float chargeSpeedMul, float chargeHoldSec)
{
    chargeHoldSec_ = chargeHoldSec;   // ★保存（GetChargeRatioで使う）

    const bool nowHeld = (CheckHitKey(KEY_INPUT_SPACE) != 0)
        || (CheckHitKey(KEY_INPUT_RETURN) != 0);

    swingReleased_   = prevHeld_ && !nowHeld;  // 前フレームON→このフレームOFF = 離した瞬間
    anyKeyTriggered_ = !prevHeld_ && nowHeld;  // 前フレームOFF→このフレームON = 押した瞬間

    if (nowHeld)
    {
        // 上限なく累積する（GetChargeRatio 内で fmod によりサイクル位置を取り出す）
        chargeTime_ += deltaTime * chargeSpeedMul;
    }
    else
    {
        if (!prevHeld_)
        {
            // 押されていない状態が継続中 → 次回押下時に 0 から開始するためリセット
            chargeTime_ = 0.0f;
        }
        // swingReleased_ が true のフレームは chargeTime_ を保持し、
        // GetChargeRatio() が離した瞬間のパワーを返せるようにする
    }

    currentlyHeld_ = nowHeld;
    prevHeld_       = nowHeld;
}

float InputManager::GetChargeRatio() const
{
    if (chargeTime_ <= 0.0f) return 0.0f;

    // 1サイクル = 上昇(CYCLE_PERIOD_SEC) + 最大キープ(chargeHoldSec_)
    const float cycleLen = CYCLE_PERIOD_SEC + chargeHoldSec_;
    const float t = fmodf(chargeTime_, cycleLen);

    // 上昇区間：0→1.0
    if (t < CYCLE_PERIOD_SEC)
        return t / CYCLE_PERIOD_SEC;

    // キープ区間：1.0 のまま停滞
    return 1.0f;
}
