#pragma once
#include <vector>

/// <summary>取得済みアイテムフラグをファイルに保存・読み込みする。</summary>
class SaveData
{
public:
    static constexpr wchar_t SAVE_PATH[] = L"save.dat";

    /// <summary>取得フラグ配列をファイルに書き出す。</summary>
    static void Save(const std::vector<bool>& obtained);

    /// <summary>ファイルから取得フラグ配列を読み込む。</summary>
    static std::vector<bool> Load(int itemCount);
};