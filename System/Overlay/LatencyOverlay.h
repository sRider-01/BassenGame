#pragma once
#include "../../Game/Swing/SwingResult.h"
#include <windows.h>
#include <deque>

/// <summary>
/// キー入力から発音までのレイテンシを計測して画面に表示するクラス。
/// QueryPerformanceCounter を使い、マイクロ秒精度で計測する。
/// Ninja Strike の 0.364ms 計測手法を移植したもの。
/// </summary>
class LatencyOverlay
{
public:
    // -------------------------------------------------------
    // 計測設定定数
    // -------------------------------------------------------
    static constexpr int HISTORY_COUNT = 10; ///< 保持する過去サンプル数

    // -------------------------------------------------------
    // オーバーレイ表示定数
    // -------------------------------------------------------
    static constexpr int OVERLAY_X          = 10; ///< 表示左上 X 座標
    static constexpr int OVERLAY_Y          = 10; ///< 表示左上 Y 座標
    static constexpr int LINE_HEIGHT        = 16; ///< 行の高さ（ピクセル）
    static constexpr int GRAPH_Y_OFFSET     = 52; ///< ヘッダ下端からグラフ開始までの Y オフセット
    static constexpr int GRAPH_WIDTH        = 200; ///< グラフの横幅（ピクセル）
    static constexpr int GRAPH_BAR_HEIGHT   =   6; ///< 1 サンプルのバー高さ（ピクセル）
    static constexpr int GRAPH_BAR_SPACING  =   2; ///< バー間の縦スペース（ピクセル）
    static constexpr int STR_BUF_SIZE       = 128; ///< 文字列フォーマットバッファサイズ

    // -------------------------------------------------------
    // グラフ色分け閾値
    // -------------------------------------------------------
    static constexpr double GRAPH_MAX_MS   = 20.0; ///< グラフの最大表示値（ms）
    static constexpr double LATENCY_GOOD_MS =  2.0; ///< 緑（良好）の上限（ms）
    static constexpr double LATENCY_WARN_MS =  5.0; ///< 黄（警告）の上限（ms）。超えると赤。

    // -------------------------------------------------------
    // 表示色定数
    // -------------------------------------------------------
    static constexpr Rgb COLOR_HEADER   = { 180, 255, 180 }; ///< ヘッダ文字の色
    static constexpr Rgb COLOR_LATEST   = { 100, 255, 100 }; ///< 最新値の色
    static constexpr Rgb COLOR_AVERAGE  = { 100, 200, 100 }; ///< 平均値の色
    static constexpr Rgb COLOR_WAITING  = { 120, 120, 120 }; ///< 計測待ちの色
    static constexpr Rgb COLOR_BAR_GOOD = {   0, 220,   0 }; ///< 良好バーの色
    static constexpr Rgb COLOR_BAR_WARN = { 200, 200,   0 }; ///< 警告バーの色
    static constexpr Rgb COLOR_BAR_BAD  = { 220,  80,   0 }; ///< 危険バーの色
    static constexpr Rgb COLOR_BAR_BG   = {  80,  80,  80 }; ///< バー背景の色

    LatencyOverlay();

    /// <summary>
    /// キーが押された瞬間に呼ぶ。入力時刻をタイムスタンプに記録する。
    /// </summary>
    void RecordInput();

    /// <summary>
    /// PlaySoundMem の直前に呼ぶ。発音時刻を記録し、
    /// RecordInput との差分をヒストリに追加する。
    /// </summary>
    void RecordSound();

    /// <summary>レイテンシ計測結果をオーバーレイとして画面に描画する。</summary>
    void Draw() const;

private:
    LARGE_INTEGER freq_;      ///< パフォーマンスカウンタ周波数
    LONGLONG      inputTick_; ///< キー入力時のカウンタ値
    LONGLONG      soundTick_; ///< 発音時のカウンタ値

    std::deque<double> latencyHistory_; ///< 過去 HISTORY_COUNT サンプルのレイテンシ（ms）
};
