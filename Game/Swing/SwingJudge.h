#pragma once
#include "SwingResult.h"
#include "../Pitch/Ball.h"

/// <summary>
/// スイングのタイミング判定と打球角度の計算を行うクラス。
/// 画面下部に判定ウィンドウバーも描画する。
/// </summary>
class SwingJudge
{
public:
    // -------------------------------------------------------
    // タイミング窓（片側・秒）
    // -------------------------------------------------------
    static constexpr float WIN_PERFECT_SEC = 0.020f; ///< ピッタリ窓の半幅（秒）
    static constexpr float WIN_SWEET_SEC   = 0.060f; ///< 真芯窓の半幅（秒）
    static constexpr float WIN_NORMAL_SEC  = 0.150f; ///< ノーマル窓の半幅（秒）
    static constexpr float WIN_GRAZE_SEC   = 0.300f; ///< カス当たり窓の半幅（秒）

    // -------------------------------------------------------
    // 打球角度（度）
    // -------------------------------------------------------
    /// タイミング 0（完璧）のときの打球角度（度）
    static constexpr float LAUNCH_ANGLE_PERFECT_DEG = 50.0f;
    /// タイミング早め限界（-WIN_GRAZE_SEC）での打球角度（低弾道）
    static constexpr float LAUNCH_ANGLE_EARLY_DEG   = 10.0f;
    /// タイミング遅め限界（+WIN_GRAZE_SEC）での打球角度（高弾道・ポップフライ）
    static constexpr float LAUNCH_ANGLE_LATE_DEG    = 80.0f;

    // -------------------------------------------------------
    // 判定バー レイアウト定数
    // -------------------------------------------------------
    static constexpr int   BAR_CENTER_X          = 640; ///< バー中央 X 座標
    static constexpr int   BAR_CENTER_Y          = 660; ///< バー中央 Y 座標
    static constexpr int   BAR_HEIGHT            =  20; ///< バーの高さ
    static constexpr int   BAR_WIDTH             = 800; ///< バーの総幅
    static constexpr int   CURSOR_HALF_W         =   3; ///< カーソルの半幅
    static constexpr int   CURSOR_EXTEND         =   4; ///< カーソルのバー上下はみ出し量
    static constexpr int   LABEL_OFFSET_Y        =   4; ///< ラベルのバー下端からの Y オフセット
    static constexpr int   LABEL_LATE_X_OFFSET   = -24; ///< 「遅い」ラベルの右端からの X オフセット
    static constexpr float BAR_HALF_RANGE_SEC    = 0.50f; ///< バーが表示する時間範囲（片側）

    // -------------------------------------------------------
    // 判定バー 色定数
    // -------------------------------------------------------
    static constexpr Rgb BAR_COLOR_MISS    = {  80,  40,  40 };
    static constexpr Rgb BAR_COLOR_GRAZE   = {  60,  80, 160 };
    static constexpr Rgb BAR_COLOR_NORMAL = { 235, 140,   0 };   // 緑 → オレンジ
    static constexpr Rgb BAR_COLOR_SWEET = { 245, 215,   0 };   // オレンジ → 黄
    static constexpr Rgb BAR_COLOR_PERFECT = { 220,  30,  30 };   // 黄 → 赤
    static constexpr Rgb BAR_COLOR_OUTLINE = { 200, 200, 200 };
    static constexpr Rgb BAR_COLOR_CURSOR  = { 255, 255, 255 };
    static constexpr Rgb LABEL_COLOR       = { 200, 200, 200 };

    /// <summary>
    /// タイミングオフセットから判定結果を返す。
    /// </summary>
    /// <param name="offset">タイミングオフセット（秒）。0 が完璧。</param>
    SwingResult Judge(float offset) const;

    /// <summary>
    /// タイミングオフセットから打球角度を算出する（度）。
    /// 早い → 低弾道、完璧 → 50°、遅い → 高弾道。
    /// MISS（窓の外）の場合も角度は返す（呼び出し側でスキップすること）。
    /// </summary>
    /// <param name="offset">タイミングオフセット（秒）</param>
    /// <returns>打球角度（度）。低弾道ほど小さく、高弾道ほど大きい。</returns>
    float CalcLaunchAngleDeg(float offset) const;

    /// <summary>
    /// 画面下部に判定ウィンドウバーを描画する。
    /// カーソルはボールが INCOMING のときのみ表示する。
    /// </summary>
    /// <param name="ball">現在のボール状態</param>
    void DrawWindow(const Ball& ball) const;

    SwingResult Judge(float offset, float perfectWidthMul = 1.0f) const;
};
