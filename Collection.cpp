#include "DxLib.h"
#include "Collection.h"
#include <cwchar>
#include <algorithm>

Collection::Collection()
    : bank_(nullptr)
    , hrCount_(0)
    , perfectCount_(0)
    , sweetCount_(0)
    , normalCount_(0)
    , grazeCount_(0)
    , totalHits_(0)
    , totalSwings_(0)
    , totalDistanceM_(0.0f)
{
}

void Collection::RecordHit(SwingResult result)
{
    ++totalSwings_;

    switch (result)
    {
    case SwingResult::PERFECT: ++perfectCount_; ++totalHits_; break;
    case SwingResult::SWEET:   ++sweetCount_;   ++totalHits_; break;
    case SwingResult::NORMAL:  ++normalCount_;  ++totalHits_; break;
    case SwingResult::GRAZE:   ++grazeCount_;   ++totalHits_; break;
    default: break;
    }
}

void Collection::RecordMiss()
{
    ++totalSwings_;
}

int Collection::RecordHomeRun()
{
    ++hrCount_;
    if (bank_ == nullptr) return -1;

    const int count = bank_->GetItemCount();
    if (count <= 0) return -1;

    const int riderKeyIndex = count - 1;   // 最後のアイテム = Rider'sキー

    // 他の全アイテム（Rider'sキー以外）を解放済みか判定
    bool othersAllUnlocked = true;
    for (int i = 0; i < count; ++i)
    {
        if (i == riderKeyIndex) continue;
        if (!obtained_[i]) { othersAllUnlocked = false; break; }
    }

    // 抽選対象を作る（Rider'sキーは他が揃うまで除外）
    std::vector<int> pool;
    for (int i = 0; i < count; ++i)
    {
        if (i == riderKeyIndex && !othersAllUnlocked) continue; // まだ出さない
        pool.push_back(i);
    }
    if (pool.empty()) return -1;

    // 重み付き抽選
    int totalWeight = 0;
    for (int i : pool)
        totalWeight += std::max(1, 10 - bank_->GetItem(i).hrRequired);

    int r = rand() % totalWeight;
    int pick = pool.front();
    for (int i : pool)
    {
        const int w = std::max(1, 10 - bank_->GetItem(i).hrRequired);
        if (r < w) { pick = i; break; }
        r -= w;
    }

    const bool isNew = !obtained_[pick];
    obtained_[pick] = true;
    sessionPrizes_.push_back(pick);
    return pick;
}

const wchar_t* Collection::GetCurrentTitle() const
{
    // HR 数 → 打撃力系称号
    if (hrCount_ >= 10) return L"\x4F1D\x8AAC\x306E\x30D0\x30C3\x30BF\x30FC"; // 伝説のバッター
    if (hrCount_ >=  5) return L"\x30DB\x30FC\x30E0\x30E9\x30F3\x738B";        // ホームラン王
    if (hrCount_ >=  3) return L"\x9577\x8DDD\x96E2\x7832";                    // 長距離砲
    if (hrCount_ >=  1) return L"\x30A2\x30FC\x30C1\x30B9\x30C8";              // アーチスト

    // 精度系称号
    if (perfectCount_ >= 5) return L"\x771F\x82AF\x30DE\x30B9\x30BF\x30FC";   // 真芯マスター
    if (perfectCount_ >= 3) return L"\x30B3\x30F3\x30BF\x30AF\x30C8\x738B";   // コンタクト王

    // 安打数系称号
    if (totalHits_ >= 15) return L"\x30D0\x30C3\x30BB\x30F3\x306E\x9054\x4EBA"; // バッセンの達人
    if (totalHits_ >= 10) return L"\x5B89\x5B9A\x6253\x8005";                   // 安定打者
    if (totalHits_ >=  5) return L"\x30D0\x30C3\x30BB\x30F3\x521D\x6BB5";       // バッセン初段
    if (totalHits_ >=  1) return L"\x30D0\x30C3\x30BB\x30F3\x898B\x7FD2\x3044"; // バッセン見習い

    return L"\x30D0\x30C3\x30BB\x30F3\x521D\x4F53\x9A13";                       // バッセン初体験
}

void Collection::DrawHud() const
{
    // 右上 HUD：HR 数と現在の称号
    SetFontSize(14);
    wchar_t buf[64];
    swprintf_s(buf, L"HR: %d", hrCount_);
    DrawString(OVERLAY_X, OVERLAY_Y,
               buf, GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));

    DrawString(OVERLAY_X, OVERLAY_Y + LINE_HEIGHT,
               GetCurrentTitle(),
               GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));

    swprintf_s(buf, L"\x5B89\x6253 %d/%d", totalHits_, totalSwings_); // 安打 N/M
    DrawString(OVERLAY_X, OVERLAY_Y + LINE_HEIGHT * 2,
               buf, GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));

    SetFontSize(16);
}

void Collection::DrawCollectionScreen() const
{
    if (bank_ == nullptr) return;

    // 半透明黒背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ======== 称号 ========
    SetFontSize(36);
    const wchar_t* title = GetCurrentTitle();
    const int titleWidth = GetDrawStringWidth(title, static_cast<int>(wcslen(title)), FALSE);
    DrawString((1280 - titleWidth) / 2, 60,
               title, GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));

    // ======== 成績 ========
    SetFontSize(20);
    wchar_t buf[128];
    const int statsX = 200;
    int statsY = 140;

    // ======== トータル飛距離（中央・大）========
    {
        wchar_t distBuf[64];
        swprintf_s(distBuf, L"%.1f m", totalDistanceM_);

        // 数字（大）
        SetFontSize(72);
        const int distW = GetDrawStringWidth(distBuf, static_cast<int>(wcslen(distBuf)), FALSE);
        const int distX = (1280 - distW) / 2;
        const int distY = 430;   // 画面中央あたり。好みで調整
        DrawString(distX + 3, distY + 3, distBuf, GetColor(0, 0, 0));            // 影
        DrawString(distX, distY, distBuf, GetColor(255, 220, 60));       // 本体（黄）

        // 見出し「トータル飛距離」（数字の上に小さく）
        SetFontSize(24);
        const wchar_t* label = L"トータル飛距離";
        const int labelW = GetDrawStringWidth(label, static_cast<int>(wcslen(label)), FALSE);
        DrawString((1280 - labelW) / 2, distY - 34, label,
            GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));

        SetFontSize(16);
    }

    swprintf_s(buf, L"トータル%d球", totalSwings_); 
    DrawString(statsX, statsY, buf, GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));

    swprintf_s(buf, L"安打   : %d本 (%d%%)", // 安打   : N本 (N%)
               totalHits_,
               totalSwings_ > 0 ? (totalHits_ * 100 / totalSwings_) : 0);
    DrawString(statsX, statsY += 28, buf,
               GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));;

    // HR は 1 本以上のときだけ表示（隠し要素のため 0 のときは見せない）
    if (hrCount_ >= 1)
    {
        swprintf_s(buf, L"HR      : %d本", hrCount_);
        DrawString(statsX, statsY += 28, buf,
                   GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));
    }

    swprintf_s(buf, L"ピッタリ : %d回", perfectCount_); // ピッタリ : N回
    DrawString(statsX, statsY += 28, buf,
               GetColor(255, 255, 0));

    swprintf_s(buf, L"真芯   : %d回", sweetCount_); // 真芯   : N回
    DrawString(statsX, statsY += 28, buf,
               GetColor(255, 160, 0));

    // ======== 今回ゲットした景品（このプレイ入手分のみ・画像なし）========
    SetFontSize(18);
    const int prizeX = 650;
    int       prizeY = 140;

    DrawString(prizeX, prizeY,
        L"今回ゲットした景品",
        GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));
    prizeY += 32;

    SetFontSize(15);
    if (sessionPrizes_.empty())
    {
        DrawString(prizeX, prizeY, L"なし",
            GetColor(COLOR_STATS.r, COLOR_STATS.g, COLOR_STATS.b));
    }
    else
    {
        for (int idx : sessionPrizes_)
        {
            swprintf_s(buf, L"★ %s", bank_->GetItem(idx).name.c_str());
            DrawString(prizeX, prizeY, buf,
                GetColor(COLOR_PRIZE_NEW.r, COLOR_PRIZE_NEW.g, COLOR_PRIZE_NEW.b));
            prizeY += 24;
        }
    }
}

void Collection::DebugUnlockAll()
{
    for (size_t i = 0; i < obtained_.size(); ++i)
        obtained_[i] = true;
}
const wchar_t* Collection::GetItemName(int index) const
{
    if (bank_ == nullptr || index < 0 || index >= bank_->GetItemCount()) return nullptr;
    return bank_->GetItem(index).name.c_str();
}

void Collection::ResetStats()
{
    hrCount_ = 0;
    perfectCount_ = 0;
    sweetCount_ = 0;
    normalCount_ = 0;
    grazeCount_ = 0;
    totalHits_ = 0;
    totalSwings_ = 0;
    totalDistanceM_ = 0.0f;
    sessionPrizes_.clear();
}

void Collection::Load(HRItemBank& bank)
{
    bank_ = &bank;
    obtained_ = SaveData::Load(bank_->GetItemCount());
}

void Collection::Save() const
{
    if (bank_ == nullptr) return;
    SaveData::Save(obtained_);
}


void Collection::DrawCollectionBrowse() const
{
    if (bank_ == nullptr) return;

    // 背景
    DrawBox(0, 0, 1280, 720, GetColor(137, 207, 240), TRUE);

    // タイトル
    SetFontSize(32);
    DrawString(80, 40, L"コレクション",
        GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));

    const int count = bank_->GetItemCount();

    // ===== アイテムグリッド =====
    for (int i = 0; i < count; ++i)
    {
        const int col = i % BROWSE_COLS;
        const int row = i / BROWSE_COLS;
        const int cx = BROWSE_START_X + col * BROWSE_CELL_W;
        const int cy = BROWSE_START_Y + row * BROWSE_CELL_H;

        const HRItem& item = bank_->GetItem(i);
        const bool    gotIt = obtained_[i];
        const bool    selected = (i == browseSel_);

        // 選択ハイライト枠
        if (selected)
            DrawBox(cx - 6, cy - 6, cx + BROWSE_ICON + 6, cy + BROWSE_ICON + 6,
                GetColor(255, 220, 60), FALSE);

        // アイコン（未取得は暗箱）
        if (gotIt && item.imageHandle != -1)
            DrawExtendGraph(cx, cy, cx + BROWSE_ICON, cy + BROWSE_ICON, item.imageHandle, TRUE);
        else
            DrawBox(cx, cy, cx + BROWSE_ICON, cy + BROWSE_ICON, GetColor(40, 40, 40), TRUE);

        // 名前
        SetFontSize(14);
        const unsigned int nameCol = gotIt
            ? GetColor(COLOR_PRIZE_NEW.r, COLOR_PRIZE_NEW.g, COLOR_PRIZE_NEW.b)
            : GetColor(90, 90, 90);
        DrawString(cx, cy + BROWSE_ICON + 4, gotIt ? item.name.c_str() : L"？？？", nameCol);
    }

    // ===== 下部コメントパネル =====
    DrawBox(60, 580, 1220, 690, GetColor(20, 20, 30), TRUE);
    DrawBox(60, 580, 1220, 690, GetColor(120, 120, 140), FALSE);

    if (browseSel_ >= 0 && browseSel_ < count)
    {
        const HRItem& sel = bank_->GetItem(browseSel_);
        const bool    gotIt = obtained_[browseSel_];

        SetFontSize(24);
        DrawString(80, 595, gotIt ? sel.name.c_str() : L"？？？",
            GetColor(COLOR_TITLE.r, COLOR_TITLE.g, COLOR_TITLE.b));

        // 未取得時のヒントを出し分ける
        const wchar_t* lockedHint =
            (browseSel_ == count - 1)   // 最後 = Rider'sキー
            ? L"他のすべてのアイテムをゲットしてホームランを打ち続けると…？"
            : L"まだ手に入れていない…ホームランで集めよう！";

        SetFontSize(18);
        DrawString(80, 632,
            gotIt ? sel.comment.c_str() : lockedHint,
            GetColor(210, 210, 210));
    }

    // 操作ガイド
    SetFontSize(16);
    DrawString(80, 700, L"← → ↑ ↓ で選択　Esc でもどる",
        GetColor(255, 255,255));
    SetFontSize(16);
}

void Collection::ResetBrowse()
{
    browseSel_ = 0;
    browsePrevL_ = browsePrevR_ = browsePrevU_ = browsePrevD_ = browsePrevBack_ = false;
}

bool Collection::UpdateBrowse()
{
    if (bank_ == nullptr) return false;
    const int count = bank_->GetItemCount();
    if (count <= 0) return false;

    const bool l = CheckHitKey(KEY_INPUT_LEFT) != 0;
    const bool r = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    const bool u = CheckHitKey(KEY_INPUT_UP) != 0;
    const bool d = CheckHitKey(KEY_INPUT_DOWN) != 0;
    const bool back = CheckHitKey(KEY_INPUT_ESCAPE) != 0; // ← BACK を ESCAPE に

    // 押した瞬間だけ動かす（押しっぱなしで暴走しないように）
    if (l && !browsePrevL_) browseSel_ = std::max(browseSel_ - 1, 0);
    if (r && !browsePrevR_) browseSel_ = std::min(browseSel_ + 1, count - 1);
    if (u && !browsePrevU_) browseSel_ = std::max(browseSel_ - BROWSE_COLS, 0);
    if (d && !browsePrevD_) browseSel_ = std::min(browseSel_ + BROWSE_COLS, count - 1);

    const bool backEdge = back && !browsePrevBack_;

    browsePrevL_ = l; browsePrevR_ = r; browsePrevU_ = u; browsePrevD_ = d;
    browsePrevBack_ = back;

    return backEdge; // true で呼び出し側がスタート画面へ戻す
}