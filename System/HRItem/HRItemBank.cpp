#include "DxLib.h"
#include "HRItemBank.h"
#include <cstdio>
#include <cwchar>

HRItemBank::HRItemBank() {}   // items_ は vector なので自動初期化

HRItemBank::~HRItemBank()
{
    for (auto& item : items_)
        if (item.imageHandle != -1) DeleteGraph(item.imageHandle);
}

bool HRItemBank::Load()
{
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, CSV_PATH, L"r, ccs=UTF-8") != 0 || fp == nullptr)
        return false;

    wchar_t buf[512];
    bool isFirst = true;
    int  rowIndex = 0;

    while (fgetws(buf, 512, fp) != nullptr)
    {
        std::wstring line(buf);
        while (!line.empty() && (line.back() == L'\n' || line.back() == L'\r'))
            line.pop_back();
        if (line.empty()) continue;

        // ヘッダ行をスキップ
        if (isFirst) { isFirst = false; continue; }

        if (line.size() >= 2 && line.front() == L'"' && line.back() == L'"')
            line = line.substr(1, line.size() - 2);

        const auto comma = line.find(L',');
        if (comma == std::wstring::npos) continue;

        HRItem item;

        auto stripQuotes = [](std::wstring& s)
            {
                if (s.size() >= 2 && s.front() == L'"' && s.back() == L'"')
                    s = s.substr(1, s.size() - 2);
            };


        item.name = line.substr(0, comma);
        item.comment = line.substr(comma + 1);
        stripQuotes(item.name);   
        stripQuotes(item.comment);
        item.hrRequired = rowIndex + 1; // 1行目=1HR, 2行目=2HR…
        item.imageHandle = -1;

        // 画像をロード: Assets\Items\item_00.png
        wchar_t imgPath[256];
        swprintf_s(imgPath, L"%s%s%02d.png", IMAGE_DIR, IMAGE_PREFIX, rowIndex);
        item.imageHandle = LoadGraph(imgPath); // 失敗時は -1 のまま

        items_.push_back(item);
        ++rowIndex;
    }

    fclose(fp);
    return !items_.empty();
}