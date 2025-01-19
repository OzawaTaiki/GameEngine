#pragma once

#include <functional>

// 前方宣言
class UVTransform;

// UVアニメーションのインターフェース
class IUVAnimation {
public:
    virtual ~IUVAnimation() = default;


    /// <summary>
    /// 制御対象のUVTransformを追加
    /// </summary>
    /// <param name="_transform"></param>
    virtual void AddTransform(UVTransform* _transform) = 0;

    /// <summary>
    /// 制御対象のUVTransformを削除
    /// </summary>
    /// <param name="_transform"></param>
    virtual void RemoveTransform(UVTransform* _transform) = 0;

    /// <summary>
    /// アニメ―ションの状態を更新
    /// </summary>
    /// <param name="_deltaTime"></param>
    virtual void Update(float _deltaTime) = 0;

    /// <summary>
    /// アニメーションを再生する
    /// </summary>
    virtual void Play() = 0;

    /// <summary>
    /// アニメーションを停止する
    /// </summary>
    virtual void Stop() = 0;

    /// <summary>
    /// アニメ―ションを初期状態にする
    /// </summary>
    virtual void Reset() = 0;

    /// <summary>
    /// 再生時間を設定する
    /// </summary>
    /// <param name="duration"></param>
    virtual void SetDuration(float duration) = 0;

    /// <summary>
    /// ループ再生するかどうかを設定する
    /// </summary>
    /// <param name="looping"></param>
    virtual void SetLooping(bool looping) = 0;

    /// <summary>
    /// アニメーション再生完了時のコールバックを設定する
    /// </summary>
    /// <param name="_callback"></param>
    virtual void SetOnComplete(std::function<void()> _callback) = 0;

    /// <summary>
    /// アニメーションが再生中かどうかを取得する
    /// </summary>
    /// <returns></returns>
    virtual bool IsPlaying() const = 0;

    /// <summary>
    /// アニメーションの進行度を取得する
    /// </summary>
    /// <returns></returns>
    virtual float GetProgress() const = 0;
};
