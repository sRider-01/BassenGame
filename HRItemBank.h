#pragma once
#include <vector>
#include <string>

struct HRItem
{
    std::wstring name;       ///< アイテム名
    std::wstring comment;    ///< 説明コメント
    int          hrRequired; ///< 必要HR数（CSV行インデックス+1）
    int          imageHandle;///< 画像ハンドル（-1=未ロード）
};

class HRItemBank
{
public:
    static constexpr wchar_t CSV_PATH[] = L"Assets\\CSV\\HRItem.csv";
    static constexpr const wchar_t* IMAGE_DIR = L"Assets\\HRItemImage\\";
    static constexpr wchar_t IMAGE_PREFIX[] = L"item_";

    HRItemBank();
    ~HRItemBank();

    /// <summary>CSVを読み込み、画像もロードする。DxLib_Init()後に呼ぶ。</summary>
    bool Load();

    int  GetItemCount() const { return static_cast<int>(items_.size()); }
    const HRItem& GetItem(int index) const { return items_[index]; }

private:
    std::vector<HRItem> items_;
};