#pragma once

/// <summary>キャラごとの特性をまとめた構造体。</summary>
struct CharaTrait
{
    float chargeSpeedMul = 1.0f;   ///< チャージ速度倍率（小さいほど遅い）
    float perfectWidthMul = 1.0f;   ///< ピッタリ判定窓の倍率（大きいほど広い）
    int   hrBonusPercent = 0;      ///< HR確率ボーナス(%)
    bool  hrOnOrange = false;  ///< オレンジ(SWEET)でもHR抽選するか
    bool  hidePowerGauge = false;  ///< パワーゲージを隠すか
    float chargeHoldSec = 0.0f;   ///< パワー最大(1.0)で停滞する時間（秒）



    /// <summary>キャラ番号（0=シノビ,1=ウィザード,2=老師）から特性を返す。</summary>
    static CharaTrait FromIndex(int idx)
    {
        CharaTrait t;
        switch (idx)
        {
        case 1: // ウィザードマン：速度は普通、最大パワーを少しキープできる
            t.chargeSpeedMul = 1.0f;     // 速度は普通に戻す
            t.chargeHoldSec = 0.4f;     // 1.0 に達したら0.4秒キープ
            break;
        case 2: // 老師：ゲージ非表示・HR出やすい
            t.hrBonusPercent = 20;
            t.hrOnOrange = true;
            t.hidePowerGauge = true;
            break;
        default: // シノビ（0）：基本のまま
            break;
        }
        return t;
    }
};