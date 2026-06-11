#pragma once
#include "../../Game/Swing/SwingResult.h"
#include "../Overlay/LatencyOverlay.h"
#include "../CharacterSelect/CharacterSelect.h"

/// <summary>
/// 打撃音を管理するクラス。
/// 起動時に MP3 ファイルをプリロードし、状況に応じた音を即座に再生する。
///
/// 判定とサウンドのマッピング:
///   MISS             → Miss.mp3
///   GRAZE / NORMAL   → Foul1・Foul2 をランダム選択
///   SWEET / PERFECT  → Hit.mp3
///   HR 確定時        → HomeRun.mp3（PlayHomeRun() で別途呼ぶ）
/// </summary>
class AudioBank
{
public:
    // -------------------------------------------------------
    // サウンドファイルパス定数
    // -------------------------------------------------------
    static constexpr wchar_t SOUND_DIR[] = L"Assets\\SE\\Shinobi\\";
    static constexpr wchar_t BGM_DIR[] = L"Assets\\BGM\\";
    static constexpr int     BGM_COUNT = 2; ///< BGMの曲数
    static constexpr wchar_t RESULT_BGM_PATH[] = L"Assets\\BGM\\result_0.mp3";

    // -------------------------------------------------------
    // ハンドル数定数
    // -------------------------------------------------------
    /// Foul サウンドの種類数（Foul1・Foul2）
    static constexpr int FOUL_SOUND_COUNT = 2;

    AudioBank();
    ~AudioBank();

    /// <summary>
    /// 全打撃音を MP3 ファイルからプリロードする。
    /// DxLib_Init() の後、ゲームループ開始前に 1 回だけ呼ぶこと。
    /// </summary>
    void Load();

    /// <summary>
    /// スイング判定に対応する打撃音を非同期再生する（DX_PLAYTYPE_BACK）。
    /// 発音直前に LatencyOverlay へ時刻を記録する。
    /// MISS → Miss、GRAZE/NORMAL → Foul ランダム、SWEET/PERFECT → Hit。
    /// </summary>
    /// <param name="result">再生する判定結果</param>
    /// <param name="overlay">レイテンシ計測用オーバーレイ</param>
    void Play(SwingResult result, LatencyOverlay& overlay);

    /// <summary>
    /// ホームラン確定時に呼ぶ。HomeRun.mp3 を非同期再生する。
    /// スイング音（Hit.mp3）とは独立して再生される。
    /// </summary>
    void PlayHomeRun();

    /// <summary>
    /// BGMをランダムに1曲選んでループ再生する。
    ///</summary>
    void PlayBgm(int index);

    /// <summary>
    ///再生中のBGMを停止する。
    /// </summary>
    void StopBgm();

    /// <summary>
    ///チャージSEをループ再生する。
    /// </summary>
    void PlayChargeSe();

    /// <summary>
    ///チャージSEを停止する。
    /// </summary>
    void StopChargeSe();

    /// <summary>
    ///結果画面のBGMを再生する。
    /// </summary>
    void PlayResultBgm();

    /// <summary>
    /// キャラクターに応じたSEを再ロードする。
    ///</summary>
    void LoadSeForCharacter(const wchar_t* seDir);

    void  SetBgmVolume(int vol);   // 0〜255
    void  SetSeVolume(int vol);    // 0〜255
    int   GetBgmVolume() const { return bgmVolume_; }
    int   GetSeVolume()  const { return seVolume_; }
    void PlayRandomSe();

private:
    int hitHandle_;                     ///< Hit.mp3 のサウンドハンドル
    int missHandle_;                    ///< Miss.mp3 のサウンドハンドル
    int homeRunHandle_;                 ///< HomeRun.mp3 のサウンドハンドル
    int foulHandles_[FOUL_SOUND_COUNT]; ///< Foul1・Foul2 のサウンドハンドル
    int bgmHandles_[BGM_COUNT]; ///< BGMのサウンドハンドル
    int resultBgmHandle_;
    int chargeSeHandle_;

    /// <summary>指定パスのサウンドファイルをロードする。失敗時は -1 を返す。</summary>
    static int LoadSoundFile(const wchar_t* filePath);

    /// <summary>Foul1・Foul2 からランダムにハンドルを返す。</summary>
    int GetRandomFoulHandle() const;

    int bgmVolume_ = 80;   // 既定音量（今のコードと同じ）
    int seVolume_ = 80;
};
