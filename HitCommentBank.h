#pragma once
#include "SwingResult.h"
#include <vector>
#include <string>

/// <summary>
/// バッティング結果のコメントを CSV から読み込み管理するクラス。
/// 判定ごとにコメントリストを保持し、ランダムに 1 件返す。
///
/// CSV フォーマット（UTF-8）:
///   result,comment
///   PERFECT,完璧なタイミング！
///   SWEET,真芯でとらえた！
///   ...
/// </summary>
class HitCommentBank
{
public:
    // -------------------------------------------------------
    // CSV ファイルパス定数
    // -------------------------------------------------------
    static constexpr wchar_t CSV_PATH[] = L"Assets\\CSV\\BattingResult.csv";

    // -------------------------------------------------------
    // 内部定数
    // -------------------------------------------------------
    /// 管理する SwingResult の種類数（WEAK_POWER が末尾なので +1）
    static constexpr int RESULT_COUNT =
        static_cast<int>(SwingResult::WEAK_POWER) + 1;

    HitCommentBank();

    /// <summary>
    /// CSV ファイルを読み込み、判定ごとにコメントをグループ化する。
    /// </summary>
    /// <returns>読み込み成功なら true。ファイルが見つからない場合は false。</returns>
    bool Load();

    /// <summary>
    /// 指定した判定に対応するコメントをランダムに 1 件返す。
    /// 該当コメントがない場合は空文字列を返す。
    /// </summary>
    /// <param name="result">コメントを取得する判定結果</param>
    const wchar_t* GetRandomComment(SwingResult result) const;

    /// <summary>CSV のロードに成功しているか返す。</summary>
    bool IsLoaded() const { return isLoaded_; }

private:
    /// 判定インデックスごとのコメントリスト
    std::vector<std::wstring> commentsByResult_[RESULT_COUNT];
    bool isLoaded_;

    /// <summary>CSV の result 列文字列を SwingResult に変換する。</summary>
    /// <param name="str">変換する文字列（例: L"PERFECT"）</param>
    /// <returns>対応する SwingResult。不明な場合は SwingResult::NONE。</returns>
    static SwingResult ParseResultString(const std::wstring& str);
};
