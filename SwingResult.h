#pragma once

/// <summary>
/// RGB 色成分を保持する軽量構造体。
/// constexpr として定数色を定義するために使用する。
/// </summary>
struct Rgb
{
    int r; ///< 赤成分 (0-255)
    int g; ///< 緑成分 (0-255)
    int b; ///< 青成分 (0-255)
};

/// <summary>
/// スイング判定の結果を表す列挙型。
/// 内側の窓ほど精度が高い。
/// </summary>
enum class SwingResult
{
    NONE,       ///< 未判定（初期値）
    MISS,       ///< 空振り
    GRAZE,      ///< カス当たり
    NORMAL,     ///< ノーマル
    SWEET,      ///< 真芯
    PERFECT,    ///< ピッタリ（最高評価）
    WEAK_POWER  ///< パワー不足（タイミングが良くても力が足りない）
};

// -------------------------------------------------------
// 判定ごとの表示色定数
// -------------------------------------------------------
static constexpr Rgb RESULT_COLOR_NONE       = { 200, 200, 200 };
static constexpr Rgb RESULT_COLOR_MISS       = { 120,  80,  80 };
static constexpr Rgb RESULT_COLOR_GRAZE      = {  80, 120, 200 };
static constexpr Rgb RESULT_COLOR_NORMAL     = { 235, 140,   0 };
static constexpr Rgb RESULT_COLOR_SWEET      = { 245, 215,   0 };
static constexpr Rgb RESULT_COLOR_PERFECT    = { 220,  30,  30 };
static constexpr Rgb RESULT_COLOR_WEAK_POWER = { 140, 140, 160 }; ///< パワー不足（くすんだ青灰）

/// <summary>
/// 判定結果に対応する日本語名を返す。
/// </summary>
/// <param name="result">表示する判定結果</param>
/// <returns>判定名のワイド文字列。NONE は空文字列。</returns>
inline const wchar_t* SwingResultName(SwingResult result)
{
    switch (result)
    {
    case SwingResult::MISS:       return L"\x7A7A\x632F\x308A\xFF01";                   // 空振り！
    case SwingResult::GRAZE:      return L"\x30AB\x30B9\x5F53\x305F\x308A";             // カス当たり
    case SwingResult::NORMAL:     return L"\x30CE\x30FC\x30DE\x30EB";                   // ノーマル
    case SwingResult::SWEET:      return L"\x771F\x82AF\xFF01\xFF01";                   // 真芯！！
    case SwingResult::PERFECT:    return L"\x30D4\x30C3\x30BF\x30EA\xFF01\xFF01\xFF01"; // ピッタリ！！！
    case SwingResult::WEAK_POWER: return L"\x30D1\x30EF\x30FC\x4E0D\x8DB3\x2026";      // パワー不足…
    default:                      return L"";
    }
}

/// <summary>
/// 判定結果に対応する RGB 色定数を返す。
/// </summary>
/// <param name="result">色を取得する判定結果</param>
/// <returns>対応する Rgb 構造体。NONE のときは灰色。</returns>
inline Rgb SwingResultColor(SwingResult result)
{
    switch (result)
    {
    case SwingResult::MISS:       return RESULT_COLOR_MISS;
    case SwingResult::GRAZE:      return RESULT_COLOR_GRAZE;
    case SwingResult::NORMAL:     return RESULT_COLOR_NORMAL;
    case SwingResult::SWEET:      return RESULT_COLOR_SWEET;
    case SwingResult::PERFECT:    return RESULT_COLOR_PERFECT;
    case SwingResult::WEAK_POWER: return RESULT_COLOR_WEAK_POWER;
    default:                      return RESULT_COLOR_NONE;
    }
}
