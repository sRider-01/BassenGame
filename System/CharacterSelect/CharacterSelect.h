#pragma once
#include "../../Game/Player/Player.h"
#include <vector>
#include <string>

struct CharaData
{
    std::wstring     name;
    std::wstring     comment;
    const wchar_t* seDir;

    std::vector<int> idleHandles;
    int              frameCount;
    float            animTimer;
    int              currentFrame;
    float            drawSize;
    int              drawOffsetY;
};

class CharacterSelect
{
public:
    static constexpr wchar_t CSV_PATH[] = L"Assets\\CSV\\CharacterSelect.csv";

    CharacterSelect();
    ~CharacterSelect();

    bool Load();

    bool Update(float deltaTime);

    void Draw() const;

    const wchar_t* GetSelectedSeDir() const;
    PlayerSpriteConfig GetSelectedPlayerConfig() const;
    int GetSelectedIndex() const { return selectedIndex_; }

private:
    std::vector<CharaData> characters_;
    int  selectedIndex_;
    bool prevLeftHeld_;
    bool prevRightHeld_;
    bool prevSpaceHeld_;

    void LoadSprites(CharaData& data, const wchar_t* idlePath,
        const wchar_t* prefix, int frameCount,
        bool isSpriteSheet, int frameW, int frameH);
};