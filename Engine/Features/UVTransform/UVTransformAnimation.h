#pragma once

#include <Features/UVTransform/IUVAnimation.h>

#include <Math/Vector/Vector2.h>

class UVTransformAnimation : public IUVAnimation
{
public:
    UVTransformAnimation();
    ~UVTransformAnimation() = default;

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
    void SetDuration(float duration) override;

    /// <summary>
    /// ループ再生するかどうかを設定する
    /// </summary>
    /// <param name="looping"></param>
    void SetLooping(bool looping) override;

    /// <summary>
    /// アニメーション再生完了時のコールバックを設定する
    /// </summary>
    /// <param name="_callback"></param>

    /// <summary>
    /// アニメーションが再生中かどうかを取得する
    /// </summary>
    /// <returns></returns>
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
    /// スクロール速度を設定
    /// </summary>
    /// <param name="_speed"></param>
    void SetScrollSpeed(const Vector2& _speed) { scrollSpeed_ = _speed; }

    /// <summary>
    /// スケール速度を設定
    /// </summary>
    /// <param name="_speed"></param>
    void SetScaleSpeed(const Vector2& _speed) { scaleSpeed_ = _speed; }

    /// <summary>
    /// 回転速度を設定
    /// </summary>
    /// <param name="_speed"></param>
    void SetRotationSpeed(float _speed) { rotationSpeed_ = _speed; }

private:

    std::vector<UVTransform*> transforms_;
    float duration_ = 1.0f;
    float currentTime_ = 0.0f;
    bool isPlaying_ = false;
    bool isLooping_ = false;
    std::function<void()> onComplete_ = nullptr;

    Vector2 scrollSpeed_ = {};
    Vector2 scaleSpeed_ = {};
    float  rotationSpeed_ = 0.0f;
};

