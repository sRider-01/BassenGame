#include "DxLib.h"
#include "AudioBank.h"
#include <cstdlib>  // rand()
#include <cstring>  // wcscpy_s, wcscat_s
#include <algorithm>

AudioBank::AudioBank()
    : hitHandle_(-1)
    , missHandle_(-1)
    , homeRunHandle_(-1)
    , foulHandles_{ -1, -1 }
    , bgmHandles_{ -1, -1 }
    , resultBgmHandle_(-1)
    , chargeSeHandle_(-1)
{
    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
    {
        foulHandles_[i] = -1;
    }
}

AudioBank::~AudioBank()
{
    if (hitHandle_     != -1) DeleteSoundMem(hitHandle_);
    if (missHandle_    != -1) DeleteSoundMem(missHandle_);
    if (homeRunHandle_ != -1) DeleteSoundMem(homeRunHandle_);

    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
    {
        if (foulHandles_[i] != -1) DeleteSoundMem(foulHandles_[i]);
    }

    for (int i = 0; i < BGM_COUNT; ++i)
        if (bgmHandles_[i] != -1) DeleteSoundMem(bgmHandles_[i]);

    if (resultBgmHandle_ != -1) DeleteSoundMem(resultBgmHandle_);

    if (chargeSeHandle_ != -1) DeleteSoundMem(chargeSeHandle_); 

}

int AudioBank::LoadSoundFile(const wchar_t* filePath)
{
    return LoadSoundMem(filePath);
}

void AudioBank::Load()
{
    static constexpr size_t PATH_BUF_SIZE = 512;
    wchar_t pathBuf[PATH_BUF_SIZE];

    auto MakePath = [](const wchar_t* dir, const wchar_t* filename,
                       wchar_t* outBuf, size_t bufSize)
    {
        wcscpy_s(outBuf, bufSize, dir);
        wcscat_s(outBuf, bufSize, filename);
    };

    // Hit.mp3
    MakePath(SOUND_DIR, L"Hit.mp3", pathBuf, PATH_BUF_SIZE);
    hitHandle_ = LoadSoundFile(pathBuf);

    // Miss.mp3
    MakePath(SOUND_DIR, L"Miss.mp3", pathBuf, PATH_BUF_SIZE);
    missHandle_ = LoadSoundFile(pathBuf);

    // HomeRun.mp3
    MakePath(SOUND_DIR, L"HomeRun.mp3", pathBuf, PATH_BUF_SIZE);
    homeRunHandle_ = LoadSoundFile(pathBuf);

    // Foul1.mp3・Foul2.mp3
    static const wchar_t* FOUL_FILE_NAMES[FOUL_SOUND_COUNT] =
    {
        L"Foul1.mp3",
        L"Foul2.mp3",
    };

    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
    {
        MakePath(SOUND_DIR, FOUL_FILE_NAMES[i], pathBuf, PATH_BUF_SIZE);
        foulHandles_[i] = LoadSoundFile(pathBuf);
    }

    // BGM をロード
    wchar_t bgmPath[256];
    for (int i = 0; i < BGM_COUNT; ++i)
    {
        swprintf_s(bgmPath, L"%sbgm_%d.mp3", BGM_DIR, i);
        bgmHandles_[i] = LoadSoundMem(bgmPath);
    }

    MakePath(SOUND_DIR, L"Charge.mp3", pathBuf, PATH_BUF_SIZE);
    chargeSeHandle_ = LoadSoundFile(pathBuf);


    resultBgmHandle_ = LoadSoundMem(RESULT_BGM_PATH);
}

void AudioBank::LoadSeForCharacter(const wchar_t* seDir)
{
    // 既存のSEハンドルを解放
    if (hitHandle_ != -1) { DeleteSoundMem(hitHandle_);     hitHandle_ = -1; }
    if (missHandle_ != -1) { DeleteSoundMem(missHandle_);    missHandle_ = -1; }
    if (homeRunHandle_ != -1) { DeleteSoundMem(homeRunHandle_); homeRunHandle_ = -1; }
    if (chargeSeHandle_ != -1) { DeleteSoundMem(chargeSeHandle_); chargeSeHandle_ = -1; }
    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
        if (foulHandles_[i] != -1) { DeleteSoundMem(foulHandles_[i]); foulHandles_[i] = -1; }

    // キャラクターに応じたディレクトリを選択
    const wchar_t* dir = seDir;

    // SEを再ロード
    static constexpr size_t BUF = 512;
    wchar_t path[BUF];
    auto Make = [&](const wchar_t* file) {
        wcscpy_s(path, BUF, dir);
        wcscat_s(path, BUF, file);
        };

    Make(L"Hit.mp3");     hitHandle_ = LoadSoundFile(path);
    Make(L"Miss.mp3");    missHandle_ = LoadSoundFile(path);
    Make(L"HomeRun.mp3"); homeRunHandle_ = LoadSoundFile(path);
    Make(L"Charge.mp3");  chargeSeHandle_ = LoadSoundFile(path);
    static const wchar_t* fouls[] = { L"Foul1.mp3", L"Foul2.mp3" };
    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
    {
        Make(fouls[i]);
        foulHandles_[i] = LoadSoundFile(path);
    }
}


void AudioBank::PlayChargeSe()
{
    if (chargeSeHandle_ == -1) return;
    // 既に再生中なら重複再生しない
    if (CheckSoundMem(chargeSeHandle_)) return;
    ChangeVolumeSoundMem(seVolume_, chargeSeHandle_);
    PlaySoundMem(chargeSeHandle_, DX_PLAYTYPE_LOOP, TRUE);
}

void AudioBank::StopChargeSe()
{
    if (chargeSeHandle_ != -1) StopSoundMem(chargeSeHandle_);
}


void AudioBank::PlayBgm(int index)
{
    StopBgm();

    if (index < 0 || index >= BGM_COUNT) return;   // 範囲外は無視
    if (bgmHandles_[index] != -1)
    {
        ChangeVolumeSoundMem(bgmVolume_, bgmHandles_[index]);
        PlaySoundMem(bgmHandles_[index], DX_PLAYTYPE_LOOP, TRUE);
    }
}

void AudioBank::PlayResultBgm()
{
    StopBgm(); // ゲームBGMを止めてから
    if (resultBgmHandle_ != -1)
    {
        ChangeVolumeSoundMem(bgmVolume_, resultBgmHandle_);
        PlaySoundMem(resultBgmHandle_, DX_PLAYTYPE_LOOP, TRUE);
    }
}


void AudioBank::StopBgm()
{
    for (int i = 0; i < BGM_COUNT; ++i)
        if (bgmHandles_[i] != -1) StopSoundMem(bgmHandles_[i]);
    if (resultBgmHandle_ != -1) StopSoundMem(resultBgmHandle_);
}

int AudioBank::GetRandomFoulHandle() const
{
    int validHandles[FOUL_SOUND_COUNT];
    int validCount = 0;

    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
    {
        if (foulHandles_[i] != -1)
        {
            validHandles[validCount++] = foulHandles_[i];
        }
    }

    if (validCount == 0) return -1;
    return validHandles[rand() % validCount];
}

void AudioBank::Play(SwingResult result, LatencyOverlay& overlay)
{
    int handleToPlay = -1;

    switch (result)
    {
    case SwingResult::MISS:
        handleToPlay = missHandle_;
        break;

    case SwingResult::GRAZE:
    case SwingResult::NORMAL:
    case SwingResult::WEAK_POWER:   // パワー不足もファール音
        handleToPlay = GetRandomFoulHandle();
        break;

    case SwingResult::SWEET:
    case SwingResult::PERFECT:
        handleToPlay = hitHandle_;
        break;

    default:
        return;
    }

    if (handleToPlay == -1) return;

    overlay.RecordSound();
    ChangeVolumeSoundMem(seVolume_, handleToPlay);
    PlaySoundMem(handleToPlay, DX_PLAYTYPE_BACK, TRUE);
}

void AudioBank::PlayHomeRun()
{
    if (homeRunHandle_ == -1) return;
    ChangeVolumeSoundMem(seVolume_, homeRunHandle_);
    PlaySoundMem(homeRunHandle_, DX_PLAYTYPE_BACK, TRUE);
}

void AudioBank::SetBgmVolume(int vol)
{
    bgmVolume_ = std::clamp(vol, 0, 255);
    // 再生中のBGMにも即反映
    for (int i = 0; i < BGM_COUNT; ++i)
        if (bgmHandles_[i] != -1) ChangeVolumeSoundMem(bgmVolume_, bgmHandles_[i]);
    if (resultBgmHandle_ != -1) ChangeVolumeSoundMem(bgmVolume_, resultBgmHandle_);
}

void AudioBank::SetSeVolume(int vol)
{
    seVolume_ = std::clamp(vol, 0, 255);
    // SEは再生のたびに音量指定するので、ここでは値を更新するだけ
}

void AudioBank::PlayRandomSe()
{
    // 鳴らせるSEハンドルを集める
    int candidates[8];
    int n = 0;
    if (hitHandle_ != -1) candidates[n++] = hitHandle_;
    if (missHandle_ != -1) candidates[n++] = missHandle_;
    if (homeRunHandle_ != -1) candidates[n++] = homeRunHandle_;
    for (int i = 0; i < FOUL_SOUND_COUNT; ++i)
        if (foulHandles_[i] != -1) candidates[n++] = foulHandles_[i];

    if (n == 0) return;

    const int h = candidates[rand() % n];
    ChangeVolumeSoundMem(seVolume_, h);
    PlaySoundMem(h, DX_PLAYTYPE_BACK, TRUE);
}