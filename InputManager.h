#pragma once

/// <summary>
/// キーボード入力を管理するクラス。
/// スペースキーの「押し続け＝パワー溜め、離す＝スイング」を実現する。
///
/// パワーはコサイン波で循環する（0.0→1.0→0.0→1.0…）。
/// 長押しし続けると青→黄→赤→青とループするため、
/// 「ひたすら溜めれば最強」にならず、赤のピークで離す判断が要求される。
/// </summary>
class InputManager
{
public:
    /// パワーが 0→ピーク→0 する 1 サイクルの時間（秒）
    static constexpr float CYCLE_PERIOD_SEC = 1.5f;

    /// この値以上のパワーを「赤ゾーン」と判定する（0.0〜1.0）
    static constexpr float RED_POWER_THRESHOLD = 0.85f;

    static constexpr float MIN_HIT_POWER = 0.30f;  // これ未満だけパワー不足

    InputManager();

    /// <summary>
    /// 毎フレーム呼び出して入力状態を更新する。
    /// ゲームループの先頭で 1 回だけ呼ぶこと。
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime, float chargeSpeedMul = 1.0f, float chargeHoldSec = 0.0f);

    /// <summary>
    /// このフレームにスイング（キーを離す）が発生したか返す。
    /// 判定トリガーとして使用する。
    /// </summary>
    bool SwingReleased() const { return swingReleased_; }

    /// <summary>スペースキーを押し続けているか（充填中）を返す。</summary>
    bool IsCharging() const { return currentlyHeld_; }

    void Update(float deltaTime, float chargeSpeedMul = 1.0f);

    /// <summary>
    /// 現在のパワー割合を返す（0.0 ～ 1.0）。
    /// 押し始め 0.0 → CYCLE_PERIOD_SEC/2 秒後に 1.0（赤ピーク）→
    /// CYCLE_PERIOD_SEC 秒後に再び 0.0（青）… とコサイン波で循環する。
    /// キーを離すと次回押下時に 0.0 からリスタートする。
    /// </summary>
    float GetChargeRatio() const;

    /// <summary>このフレームにスペースキーが押された瞬間か返す（Attack アニメ開始用）。</summary>
    bool SwingJustPressed() const { return anyKeyTriggered_; }

    /// <summary>何かキーが押されているか（UI 操作用）を返す。</summary>
    bool AnyKeyTriggered() const { return anyKeyTriggered_; }

private:
    bool  prevHeld_;        ///< 前フレームの押下状態
    bool  currentlyHeld_;   ///< 現フレームの押下状態
    bool  swingReleased_;   ///< このフレームにキーが離されたか
    bool  anyKeyTriggered_; ///< このフレームに任意のキーが押されたか
    float chargeTime_;      ///< 押し続けた累積時間（秒）。離すとリセット。
    float chargeHoldSec_ = 0.0f;   ///< 最大パワーでの停滞時間（キャラ特性）
};
