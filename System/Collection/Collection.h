#pragma once
#include "../../Game/Swing/SwingResult.h"
#include "../HRItem/HRItemBank.h"
#include "../SaveData/SaveData.h"
#include <vector>

class Collection
{
public:
    // 表示定数はそのまま残す
    static constexpr int OVERLAY_X = 1100;
    static constexpr int OVERLAY_Y = 10;
    static constexpr int LINE_HEIGHT = 16;
    static constexpr Rgb COLOR_TITLE = { 255, 220,  60 };
    static constexpr Rgb COLOR_STATS = { 200, 200, 200 };
    static constexpr Rgb COLOR_PRIZE_NEW = { 255, 200,   0 };
    static constexpr Rgb COLOR_PRIZE_OLD = { 140, 140, 140 };
    static constexpr int BROWSE_COLS = 5;
    static constexpr int BROWSE_START_X = 90;
    static constexpr int BROWSE_START_Y = 130;
    static constexpr int BROWSE_CELL_W = 220;
    static constexpr int BROWSE_CELL_H = 120;
    static constexpr int BROWSE_ICON = 72;

    Collection();

    /// <summary>HRItemBankからアイテムデータを読み込み、セーブデータも復元する。</summary>
    void Load(HRItemBank& bank);

    /// <summary>セーブデータをファイルに書き出す。</summary>
    void Save() const;

    void RecordHit(SwingResult result);
    void RecordMiss();
    /// <summary>
    /// ホームランを記録し、新たに解放されたアイテムのインデックスを返す。
    /// 解放なしの場合は -1 を返す。
    /// </summary>
    int RecordHomeRun();

    /// <summary>打撃成績のみリセットする。取得済みアイテムとbankは保持。</summary>
    void ResetStats();

    /// <summary>指定インデックスのアイテム名を返す。範囲外なら nullptr。</summary>
    const wchar_t* GetItemName(int index) const;

    const wchar_t* GetCurrentTitle() const;

    int GetHRCount()      const { return hrCount_; }
    int GetPerfectCount() const { return perfectCount_; }
    int GetTotalHits()    const { return totalHits_; }
    int GetTotalSwings()  const { return totalSwings_; }
    void  AddDistance(float meters) { totalDistanceM_ += meters; }
    float GetTotalDistanceM() const { return totalDistanceM_; }

    void DrawHud() const;
    void DrawCollectionScreen() const;             // ゲームオーバー後の結果画面
    void DrawCollectionBrowse() const;             // スタート画面から見るコレクション画面

    void ResetBrowse();          ///< コレクション画面に入るとき選択を初期化
    bool UpdateBrowse();         ///< 矢印で選択移動。戻るキーが押されたら true

    void DebugUnlockAll();   ///< 【デバッグ用】全アイテムをアンロック

private:
    HRItemBank* bank_;          ///< アイテム定義（所有しない、外部から渡す）
    std::vector<bool>  obtained_;      ///< 取得済みフラグ

    int hrCount_;
    int perfectCount_;
    int sweetCount_;
    int normalCount_;
    int grazeCount_;
    int totalHits_;
    int totalSwings_;
    float totalDistanceM_ = 0.0f;  ///< セッション中の飛距離合計（m）

    int  browseSel_ = 0;
    bool browsePrevL_ = false;
    bool browsePrevR_ = false;
    bool browsePrevU_ = false;
    bool browsePrevD_ = false;
    bool browsePrevBack_ = false;
    std::vector<int> sessionPrizes_;
};