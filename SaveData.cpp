#include "SaveData.h"
#include <cstdio>

void SaveData::Save(const std::vector<bool>& obtained)
{
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, SAVE_PATH, L"w") != 0 || fp == nullptr) return;
    for (bool b : obtained)
        fwprintf(fp, L"%d\n", b ? 1 : 0);
    fclose(fp);
}

std::vector<bool> SaveData::Load(int itemCount)
{
    std::vector<bool> result(itemCount, false);
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, SAVE_PATH, L"r") != 0 || fp == nullptr) return result;
    for (int i = 0; i < itemCount; ++i)
    {
        int val = 0;
        if (fwscanf_s(fp, L"%d", &val) != 1) break;
        result[i] = (val == 1);
    }
    fclose(fp);
    return result;
}