#pragma once
#include <vector>

/// <summary>キャラクターごとのスプライト設定。</summary>
struct PlayerSpriteConfig
{
    const wchar_t* idleDir;
    const wchar_t* idlePrefix;
    int            idleFrameCount;
    bool           idleIsSpriteSheet;
    int            idleFrameW;
    int            idleFrameH;

    const wchar_t* attackDir;
    const wchar_t* attackPrefix;
    int            attackFrameCount;
    bool           attackIsSpriteSheet;
    int            attackFrameW;
    int            attackFrameH;

    float          drawScale;
    int            chargeLoopStart;  // 溜め中ループの始端フレーム
    int            chargeLoopEnd;    // 溜め中ループの終端フレーム（start==end でピン）
};

/// <summary>
/// プレイヤーのスプライトアニメーションを管理するクラス。
/// Idle（待機ループ）と Attack（スイング 1 回再生）の 2 状態を持つ。
/// Space キーが押された瞬間に StartAttack() を呼ぶと Attack が再生され、
/// 最終フレームに達すると自動で Idle に戻る。
/// </summary>
class Player
{
public:
    // -------------------------------------------------------
    // アニメーション状態
    // -------------------------------------------------------
    enum class AnimState
    {
        IDLE,   ///< 待機（ループ）
        ATTACK  ///< スイング（1 回再生して IDLE へ）
    };

    // -------------------------------------------------------
    // スプライトファイル定数
    // -------------------------------------------------------
    static constexpr wchar_t SPRITE_DIR_IDLE[] = L"Assets\\Player\\Idle\\";
    static constexpr wchar_t SPRITE_DIR_ATTACK[] = L"Assets\\Player\\Attack\\";

    static constexpr wchar_t IDLE_PREFIX[]   = L"ninja_10KDStudios_idle_";
    static constexpr wchar_t ATTACK_PREFIX[] = L"ninja_10KDStudios_attack_";

    // -------------------------------------------------------
    // フレーム数・FPS 定数
    // -------------------------------------------------------
    static constexpr int   IDLE_FRAME_COUNT      = 16;   ///< Idle フレーム総数
    static constexpr int   ATTACK_FRAME_COUNT    = 67;   ///< Attack フレーム総数
    static constexpr int   ATTACK_WINDUP_END_FRAME = 12; ///< このフレームで一時停止（0始まり）
    static constexpr float IDLE_FPS              = 10.0f; ///< Idle 再生速度
    static constexpr float ATTACK_FPS            = 90.0f; ///< Attack 再生速度（30fps の 3 倍）
    static constexpr float CHARGE_LOOP_FPS = 20.0f; ///< 溜め中ループの再生速度（遅め）

    // -------------------------------------------------------
    // 描画定数
    // -------------------------------------------------------
    static constexpr int   SPRITE_SIZE     = 165;   ///< 元画像の一辺ピクセル数
    static constexpr int   DRAW_CENTER_X   = 200;   ///< 描画中心 X（HIT_ZONE_X に合わせる）
    static constexpr int   DRAW_CENTER_Y   = 430;   ///< 描画中心 Y（調整用）
    static constexpr float DRAW_SCALE      = 1.5f;  ///< 描画倍率
    static constexpr bool  FLIP_HORIZONTAL = false; ///< 左右反転フラグ（向きが逆なら true）

    // -------------------------------------------------------
    // パワーティント定数
    // -------------------------------------------------------
    static constexpr int TINT_COLOR_R    = 255; ///< オレンジティントの R 成分
    static constexpr int TINT_COLOR_G    = 140; ///< オレンジティントの G 成分
    static constexpr int TINT_COLOR_B    =   0; ///< オレンジティントの B 成分
    static constexpr int TINT_MAX_ALPHA  = 200; ///< chargeRatio=1.0 のときの最大重ね強度（0-255）

    /// オフスクリーンバッファのサイズ（スプライト表示サイズ＋余白）
    static constexpr int ORANGE_SCREEN_SIZE = 400;

    Player();
    ~Player();

    void Load(const PlayerSpriteConfig& config);
    void Unload();

    /// <summary>
    /// アニメーションを 1 フレーム分更新する。
    /// Attack 中、isChargingSpace が true のとき ATTACK_WINDUP_END_FRAME で一時停止する。
    /// Space が離された（false になった）瞬間に残りのフレームを再生再開する。
    /// </summary>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    /// <param name="isChargingSpace">Space キーを押し続けているか</param>
    void Update(float deltaTime, bool isChargingSpace);

    /// <summary>
    /// 現在のフレームを描画する。
    /// chargeRatio > 0 のとき、パワー量に応じてオレンジ色のティントを重ねる。
    /// </summary>
    /// <param name="chargeRatio">パワー充填割合（0.0〜1.0）。0 ならティントなし。</param>
    void Draw(float chargeRatio) const;

    /// <summary>
    /// Attack アニメーションを先頭から再生開始する。
    /// Space が押された瞬間（SwingJustPressed）に呼ぶ。
    /// </summary>
    void StartAttack();

    AnimState GetState() const { return state_; }

private:
    AnimState state_;
    std::vector<int> idleHandles_;
    std::vector<int> attackHandles_;
    int   currentFrame_;
    float frameTimer_;
    int   idleFrameCount_;
    int   attackFrameCount_;
    float drawScale_;
    float auraTimer_;
    int chargeLoopStart_ = 0;
    int chargeLoopEnd_ = 0;

    /// <summary>現在の状態・フレームに対応するハンドルを返す。</summary>
    int GetCurrentHandle() const;

    /// <summary>
    /// 連番 PNG を 1 枚ロードしてハンドルを返す。
    /// パス = dir + prefix + 5桁ゼロ埋め番号 + ".png"
    /// </summary>
    static int LoadFrame(const wchar_t* dir, const wchar_t* prefix, int index);
};
