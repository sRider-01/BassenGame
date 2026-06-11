#pragma once

enum class StartAction
{
    NONE,
    PLAY,
    COLLECTION,
    VOLUME,
    HINT,
};

class StartLevel
{
public:
    StartLevel();

    StartAction Update();   ///< ↑↓で選択、Enterで決定。決定された項目を返す
    void Draw() const;

    static constexpr int MENU_COUNT = 4;   ///< 開始/コレクション/音量設定/ヒント

    void Load();   ///< タイトル画像などの読み込み（DxLib_Init 後に1回呼ぶ）

private:
    int  selected_ = 0;     ///< 現在の選択項目(0..3)
    bool prevUpHeld_ = false;
    bool prevDownHeld_ = false;
    bool prevEnterHeld_ = false;
    int titleHandle_ = -1;   ///< タイトル背景画像のハンドル
};