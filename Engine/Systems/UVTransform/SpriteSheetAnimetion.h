#pragma once

#include <Systems/UVTransform/IUVAnimation.h>


/// <summary>
/// 連番画像を使用したアニメーションを制御するクラス
/// </summary>
class SpriteSheetAnimation : public IUVAnimation
{
public:
    SpriteSheetAnimation();
    SpriteSheetAnimation(uint32_t _sheetNumX, uint32_t _sheetNumY, uint32_t _sheetNum, float _switchSpeed);
    ~SpriteSheetAnimation() = default;

    /// <summary>
    /// 制御対象のUVTransformを追加
    /// </summary>
    /// <param name="_transform"></param>
    void AddTransform(UVTransform* _transform) override;

    /// <summary>
    /// 制御対象のUVTransformを削除
    /// </summary>
    /// <param name="_transform"></param>
    void RemoveTransform(UVTransform* _transform) override;

    /// <summary>
    /// アニメ―ションの状態を更新
    /// </summary>
    /// <param name="_deltaTime"></param>
    void Update(float _deltaTime) override;

    /// <summary>
    /// アニメーションを再生する
    /// </summary>
    void Play() override;;

    /// <summary>
    /// アニメーションを停止する
    /// </summary>
    void Stop() override;

    /// <summary>
    /// アニメ―ションを初期状態にする
    /// </summary>
    void Reset() override;

    /// <summary>
    /// 再生時間を設定する
    /// </summary>
    /// <param name="duration"></param>
    void SetDuration(float _duration) override;

    /// <summary>
    /// ループ再生するかどうかを設定する
    /// </summary>
    /// <param name="looping"></param>
    void SetLooping(bool _looping) override;

    /// <summary>
    /// アニメーション再生完了時のコールバックを設定する
    /// </summary>
    /// <param name="_callback"></param>
    void SetOnComplete(std::function<void()> _callback) override;

    /// <summary>
    /// アニメーションが再生中かどうかを取得する
    /// </summary>
    /// <returns></returns>
    bool IsPlaying() const override;

    /// <summary>
    /// アニメーションの進行度を取得する
    /// </summary>
    /// <returns></returns>
    float GetProgress() const override;

    /// <summary>
    /// 連番画像の枚数を設定する
    /// </summary>
    /// <param name="_sheetNumX"></param>
    void SetSheetNumX(uint32_t _sheetNumX);

    /// <summary>
    /// 連番画像の枚数を設定する
    /// </summary>
    /// <param name="_sheetNumY"></param>
    void SetSheetNumY(uint32_t _sheetNumY);

    /// <summary>
    /// 連番画像の枚数を設定する
    /// </summary>
    /// <param name="_sheetNum"></param>
    void SetSheetNum(uint32_t _sheetNum);

    /// <summary>
    /// 画像の切り替え速度を設定する
    /// </summary>
    /// <param name="_switchSpeed"></param>
    void SetSwitchSpeed(float _switchSpeed);

private:
    // 制御対象のUVTransform
    std::vector<UVTransform*> transforms_;
    // アニメーションの再生時間
    float duration_ = 1.0f;
    // 現在の再生時間
    float currentTime_ = 0.0f;
    // アニメーションが再生中かどうか
    bool isPlaying_ = false;
    // ループ再生するかどうか
    bool isLooping_ = false;
    // アニメーション再生完了時のコールバック
    std::function<void()> onComplete_ = nullptr;

    // 画像の切り替え速度(秒)
    float switchSpeed_ = 0.1f;

    // 連番画像の枚数 横
    uint32_t sheetNumX_ = 1;
    // 連番画像の枚数 縦
    uint32_t sheetNumY_ = 1;
    // 連番画像の枚数 合計
    uint32_t sheetNum_ = 1;

    // 現在の連番画像のインデックス
    uint32_t currentSheetIndex_ = 0;

    // 一枚当たりのUV値
    Vector2 sheetSize_ = {};

};
