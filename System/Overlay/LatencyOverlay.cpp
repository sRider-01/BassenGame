#include "DxLib.h"
#include "LatencyOverlay.h"
#include <cwchar>

LatencyOverlay::LatencyOverlay()
    : inputTick_(0)
    , soundTick_(0)
{
    QueryPerformanceFrequency(&freq_);
}

void LatencyOverlay::RecordInput()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    inputTick_ = now.QuadPart;
}

void LatencyOverlay::RecordSound()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    soundTick_ = now.QuadPart;

    const bool validMeasurement = (inputTick_ > 0) && (soundTick_ > inputTick_);
    if (validMeasurement)
    {
        const double latencyMs =
            static_cast<double>(soundTick_ - inputTick_)
            / static_cast<double>(freq_.QuadPart)
            * 1000.0;

        latencyHistory_.push_back(latencyMs);
        if (static_cast<int>(latencyHistory_.size()) > HISTORY_COUNT)
        {
            latencyHistory_.pop_front();
        }
    }
}

void LatencyOverlay::Draw() const
{
    const int drawX = OVERLAY_X;
    const int drawY = OVERLAY_Y;
    wchar_t textBuf[STR_BUF_SIZE];

    // ヘッダ「[ 入力→発音レイテンシ ]」
    DrawString(
        drawX, drawY,
        L"[ \x5165\x529B\x2192\x767A\x97F3\x30EC\x30A4\x30C6\x30F3\x30B7 ]",
        GetColor(COLOR_HEADER.r, COLOR_HEADER.g, COLOR_HEADER.b));

    if (!latencyHistory_.empty())
    {
        // 最新値「最新 : X.XXX ms」
        swprintf_s(textBuf, L"\x6700\x65B0 : %.3f ms", latencyHistory_.back());
        DrawString(drawX, drawY + LINE_HEIGHT,
                   textBuf, GetColor(COLOR_LATEST.r, COLOR_LATEST.g, COLOR_LATEST.b));

        // 平均値「平均 : X.XXX ms  (n=N)」
        double total = 0.0;
        for (const double latencyMs : latencyHistory_) total += latencyMs;
        const double averageMs = total / static_cast<double>(latencyHistory_.size());

        swprintf_s(textBuf, L"\x5E73\x5747 : %.3f ms  (n=%d)",
                   averageMs, static_cast<int>(latencyHistory_.size()));
        DrawString(drawX, drawY + LINE_HEIGHT * 2,
                   textBuf, GetColor(COLOR_AVERAGE.r, COLOR_AVERAGE.g, COLOR_AVERAGE.b));
    }
    else
    {
        // 計測待ち「計測待ち」
        DrawString(drawX, drawY + LINE_HEIGHT,
                   L"\x8A08\x6E2C\x5F85\x3061",
                   GetColor(COLOR_WAITING.r, COLOR_WAITING.g, COLOR_WAITING.b));
    }

    // ミニグラフ（水平バー）
    const int graphX = drawX;
    const int graphY = drawY + GRAPH_Y_OFFSET;

    for (int i = 0; i < static_cast<int>(latencyHistory_.size()); ++i)
    {
        const double latencyMs = latencyHistory_[i];

        int barWidth = static_cast<int>(latencyMs / GRAPH_MAX_MS * GRAPH_WIDTH);
        if (barWidth > GRAPH_WIDTH) barWidth = GRAPH_WIDTH;

        const int barTop = graphY + i * (GRAPH_BAR_HEIGHT + GRAPH_BAR_SPACING);

        // 良好 / 警告 / 危険 で色を変える
        const Rgb& barColor = (latencyMs < LATENCY_GOOD_MS) ? COLOR_BAR_GOOD
                            : (latencyMs < LATENCY_WARN_MS) ? COLOR_BAR_WARN
                                                            : COLOR_BAR_BAD;

        DrawBox(graphX, barTop, graphX + barWidth,   barTop + GRAPH_BAR_HEIGHT,
                GetColor(barColor.r,       barColor.g,       barColor.b),       TRUE);
        DrawBox(graphX, barTop, graphX + GRAPH_WIDTH, barTop + GRAPH_BAR_HEIGHT,
                GetColor(COLOR_BAR_BG.r, COLOR_BAR_BG.g, COLOR_BAR_BG.b), FALSE);
    }
}
