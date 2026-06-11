#pragma once
#include "SwingResult.h"
#include <deque>

/// <summary>
/// ボールの状態を表す列挙型。
/// INACTIVE → Launch() → INCOMING → ApplyHit() → HIT
///                                 → HasPassed() → PASSED
/// </summary>
enum class BallState
{
    INACTIVE, ///< 未発射
    INCOMING, ///< 投球中（投手→打者方向に飛行）
    HIT,      ///< 打球（打者→外野方向に飛行）
    PASSED    ///< 見逃し通過
};

/// <summary>
/// ボールを表すクラス。
/// 投球の直線移動と、打撃後の放物線運動の両方を担当する。
/// </summary>
class Ball
{
public:
    // -------------------------------------------------------
    // 投球レイアウト定数
    // -------------------------------------------------------
    static constexpr float START_X          = 1200.0f; ///< 投球開始 X 座標
    static constexpr float PERFECT_X_OFFSET = 60.0f;  // ピッタリ位置を右へ何px寄せるか
    static constexpr float HIT_ZONE_X       =  200.0f; ///< ヒットゾーン X 座標
    static constexpr float BALL_Y           =  440.0f; ///< ボールの Y 座標（固定ライン）
    static constexpr float PITCH_DURATION   =    2.0f; ///< 投球の飛行時間（秒）
    static constexpr float PASS_GRACE_SEC   =    0.4f; ///< 通過後のミス判定猶予（秒）

    // -------------------------------------------------------
    // 打球物理定数
    // -------------------------------------------------------
    static constexpr float HIT_SPEED_MIN    =  100.0f; ///< 最小打球速度（ピクセル/秒）
    static constexpr float HIT_SPEED_MAX    =  4100.0f; ///< 最大打球速度（ピクセル/秒）
    static constexpr float GRAVITY          =  480.0f; ///< 重力加速度（ピクセル/秒²）
    static constexpr float GROUND_Y         =  540.0f; ///< 地面 Y 座標（ここに落ちたら着地）
    static constexpr float METERS_PER_PX = 0.0045f; ///< ピクセル→メートル換算（要調整）

    // -------------------------------------------------------
    // ホームラン判定定数
    // -------------------------------------------------------
    /// HR 判定はパワーが赤（RED_POWER_THRESHOLD 以上）のときのみ発生。
    /// タイミング判定ごとに異なる確率（%）を使用する。
    /// GRAZE / MISS は対象外（0%）。
    static constexpr int HR_PROB_PERCENT_PERFECT = 60; ///< ピッタリ（黄）のとき 60%
    static constexpr int HR_PROB_PERCENT_SWEET   = 30; ///< 真芯（オレンジ）のとき 30%
    static constexpr int HR_PROB_PERCENT_NORMAL  =  5; ///< ノーマル（緑）のとき 5%

    // -------------------------------------------------------
    // 描画定数
    // -------------------------------------------------------
    static constexpr float BALL_RADIUS      =   14.0f; ///< ボール半径
    static constexpr int   SHADOW_OFFSET_X  =    4;    ///< 影の X オフセット
    static constexpr int   SHADOW_OFFSET_Y  =    4;    ///< 影の Y オフセット
    static constexpr int   SEAM_LINE_INSET  =    3;    ///< 縫い目ラインの内側削り量
    static constexpr int   SEAM_LINE_Y_OFF  =    4;    ///< 縫い目ラインの Y 傾き
    static constexpr int   TRAIL_LENGTH     =   10;    ///< 残像の最大点数

    static constexpr Rgb COLOR_SHADOW    = {  40,  40,  40 }; ///< 影の色
    static constexpr Rgb COLOR_BODY      = { 245, 245, 230 }; ///< ボール本体の色
    static constexpr Rgb COLOR_SEAM      = { 160,  60,  60 }; ///< 縫い目の色
    static constexpr Rgb COLOR_HIT_TRAIL = { 255, 200,  80 }; ///< 打球残像の色

    Ball();

    /// <summary>新しいボールを投球する（INACTIVE → INCOMING）。</summary>
    void Launch();

    /// <summary>
    /// ボールに打撃を与え、打球として飛ばす（INCOMING → HIT）。
    /// ホームラン判定は以下の3条件をすべて満たしたとき 10% の確率で発生する：
    ///   1. powerRatio >= InputManager::RED_POWER_THRESHOLD（赤ゾーン）
    ///   2. result が SWEET または PERFECT（真芯）
    ///   3. 乱択（1/HR_PROBABILITY_DENOM）
    /// </summary>
    /// <param name="launchAngleDeg">打球角度（度）。0=水平、90=真上。</param>
    /// <param name="powerRatio">パワー割合（0.0 ～ 1.0）。</param>
    /// <param name="result">スイング判定結果。HR 判定条件に使用。</param>
    void ApplyHit(float launchAngleDeg, float powerRatio, SwingResult result,
        int hrBonusPercent = 0, bool hrOnOrange = false);

    /// <summary>ボール位置と状態を 1 フレーム分更新する。</summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    void Update(float deltaTime);

    /// <summary>ボールを描画する。INACTIVE のときは何もしない。</summary>
    void Draw() const;

    /// <summary>ボールを INACTIVE にリセットする。</summary>
    void Reset();

    // -------------------------------------------------------
    // 状態取得
    // -------------------------------------------------------
    BallState GetState()  const { return state_; }
    bool IsIncoming()     const { return state_ == BallState::INCOMING; }
    bool IsHit()          const { return state_ == BallState::HIT; }
    bool IsActive()       const { return state_ == BallState::INCOMING || state_ == BallState::HIT; }

    /// <summary>ホームランが確定しているか返す（打撃時に計算済み）。</summary>
    bool IsHomeRun() const { return homeRunConfirmed_; }

    /// <summary>直近の打球の飛距離（メートル）を返す。</summary>
    float GetHitDistanceMeters() const { return hitDistanceM_; }

    /// <summary>
    /// ボールが「演出も含め終了」したか返す。
    /// HIT → 画面外 or 着地、PASSED → 猶予経過、のとき true。
    /// </summary>
    bool IsFinished() const;

    /// <summary>ヒットゾーンを通過してスイング猶予が切れたか返す。</summary>
    bool HasPassed() const;

    /// <summary>
    /// タイミングオフセットを返す（秒）。
    /// 0 = 完璧タイミング。負 = まだ来ていない。正 = 通過後。
    /// </summary>
    float GetTimingOffset() const;

    /// <summary>見逃しが発生したフレームだけ true を返し、読んだら false に戻す。</summary>
    bool ConsumeJustMissed() { const bool m = justMissed_; justMissed_ = false; return m; }

private:
    BallState state_;
    float x_;               ///< 現在 X 座標
    float y_;               ///< 現在 Y 座標
    float vx_;              ///< X 方向速度（HIT 状態）
    float vy_;              ///< Y 方向速度（HIT 状態）
    float incomingElapsed_; ///< INCOMING 状態の経過時間
    bool  homeRunConfirmed_;///< ApplyHit 時に計算したホームラン判定

    // 打球残像バッファ
    struct TrailPoint { float x, y; };
    TrailPoint trailBuf_[TRAIL_LENGTH];
    int        trailHead_;  ///< リングバッファのヘッド位置
    int        trailCount_; ///< 有効な残像点数
    float hitDistanceM_ = 0.0f;  ///< 直近の打球の飛距離（m）。ApplyHit で計算。

    float spinAngle_ = 0.0f;   ///< 縫い目の回転角（描画用）
    static constexpr float SPIN_SPEED = 12.0f;   ///< 回転速度（rad/s）。大きいほど速い
    bool justMissed_ = false;
};
