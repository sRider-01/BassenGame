#include "HitCommentBank.h"
#include <cstdio>   // _wfopen_s, fgetws, fclose
#include <cstdlib>  // rand
#include <cstring>  // wcslen

// 1 行あたりの最大文字数（コメントが極端に長い場合への保険）
static constexpr int LINE_BUF_SIZE = 512;

HitCommentBank::HitCommentBank()
    : isLoaded_(false)
{
}

SwingResult HitCommentBank::ParseResultString(const std::wstring& str)
{
    if (str == L"PERFECT")    return SwingResult::PERFECT;
    if (str == L"SWEET")      return SwingResult::SWEET;
    if (str == L"NORMAL")     return SwingResult::NORMAL;
    if (str == L"GRAZE")      return SwingResult::GRAZE;
    if (str == L"MISS")       return SwingResult::MISS;
    if (str == L"WEAK_POWER") return SwingResult::WEAK_POWER;
    return SwingResult::NONE;
}

bool HitCommentBank::Load()
{
    // ccs=UTF-8 で開くことで Windows がファイルの UTF-8 バイト列を
    // wchar_t（UTF-16LE）に自動変換してくれる
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, CSV_PATH, L"r, ccs=UTF-8") != 0 || fp == nullptr)
    {
        return false;
    }

    wchar_t lineBuf[LINE_BUF_SIZE];
    bool isFirstLine = true;

    while (fgetws(lineBuf, LINE_BUF_SIZE, fp) != nullptr)
    {
        std::wstring line(lineBuf);

        // 行末の改行・CR を除去
        while (!line.empty() && (line.back() == L'\n' || line.back() == L'\r'))
        {
            line.pop_back();
        }

        // Excel が付ける行全体を囲む二重引用符を除去
        if (line.size() >= 2 && line.front() == L'"' && line.back() == L'"')
        {
            line = line.substr(1, line.size() - 2);
        }

        if (line.empty()) continue;

        // ヘッダ行をスキップ
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        // 最初のカンマで result と comment に分割
        const auto commaPos = line.find(L',');
        if (commaPos == std::wstring::npos) continue;

        const std::wstring resultStr = line.substr(0, commaPos);
        const std::wstring comment   = line.substr(commaPos + 1);

        if (comment.empty()) continue;

        const SwingResult result = ParseResultString(resultStr);
        if (result == SwingResult::NONE) continue;

        commentsByResult_[static_cast<int>(result)].push_back(comment);
    }

    fclose(fp);
    isLoaded_ = true;
    return true;
}

const wchar_t* HitCommentBank::GetRandomComment(SwingResult result) const
{
    const int index = static_cast<int>(result);
    if (index < 0 || index >= RESULT_COUNT) return L"";

    const auto& list = commentsByResult_[index];
    if (list.empty()) return L"";

    return list[static_cast<size_t>(rand()) % list.size()].c_str();
}
