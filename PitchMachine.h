#pragma once
#include "Ball.h"

/// <summary>
/// 投球マシンを管理するクラス。
/// 一定間隔で自動的にボールを投球し、セッションの球数を管理する。
/// リアルなバッティングセンターのように、プレイヤーの行動に関係なく
/// 決まったリズムでボールを供給する。
/// </summary>
class PitchMachine
{
public:
    // -------------------------------------------------------
    // ゲーム設定定数
    // -------------------------------------------------------
    static constexpr int   TOTAL_BALLS          = 10;   ///< 1セッションの投球数
    static constexpr float INTER_PITCH_DELAY    = 1.5f; ///< 前の球が終わってから次が来るまでの時間（秒）
    static constexpr float INITIAL_PITCH_DELAY  = 3.0f; ///< セッション開始時の最初の投球までの待機時間（秒）

    // -------------------------------------------------------
    // 表示定数
    // -------------------------------------------------------
    static constexpr int DISPLAY_X           = 20;  ///< 球数表示 X 座標
    static constexpr int DISPLAY_Y           = 680; ///< 球数表示 Y 座標
    static constexpr Rgb COLOR_BALL_ACTIVE   = { 255, 220,  60 }; ///< 残り球の色
    static constexpr Rgb COLOR_BALL_USED     = {  60,  60,  60 }; ///< 使用済み球の色
    static constexpr Rgb COLOR_LABEL         = { 200, 200, 200 }; ///< ラベル文字の色

    PitchMachine();

    /// <summary>
    /// 投球マシンの状態を更新する。
    /// canPitch が false のときはタイマーを止め投球しない。
    /// Ball が INACTIVE かつ canPitch かつタイマーが切れたら ball.Launch() を呼ぶ。
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    /// <param name="ball">管理するボール（状態に応じて Launch を呼ぶ）</param>
    /// <param name="canPitch">投球を許可するか（false のとき待機タイマーを凍結）</param>
    void Update(float deltaTime, Ball& ball, bool canPitch = true);

    /// <summary>
    /// 次の投球を即座に行えるよう待機タイマーを 0 にリセットする。
    /// フェーズ2移行時に呼び出して、すぐに投球させる。
    /// </summary>
    void TriggerImmediatePitch() { waitTimer_ = 0.0f; }

    /// <summary>セッションが終了（全球使用済み）したか返す。</summary>
    bool IsSessionOver() const;

    /// <summary>残り球数を返す。</summary>
    int GetRemainingBalls() const { return remainingBalls_; }

    /// <summary>画面下部に残り球数インジケーターを描画する。</summary>
    void Draw() const;

    /// <summary>ピッチマシンをリセットする（新セッション開始時）。</summary>
    void Reset();

private:
    int   remainingBalls_; ///< 残り投球数
    float waitTimer_;      ///< 次の投球までの待機タイマー（秒）
    bool  sessionOver_;    ///< セッション終了フラグ
};
