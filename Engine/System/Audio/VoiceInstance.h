#pragma once

#include <xaudio2.h>

class VoiceInstance
{
public:
    VoiceInstance(IXAudio2SourceVoice* _sourceVoice, float _volume = 1.0f, float _sampleRate = 44100.0f);
    ~VoiceInstance() = default;

    /// <summary>
    /// 始めから再生
    /// </summary>
    void Play();

    /// <summary>
    /// 再生を停止
    /// </summary>
    void Stop();

    /// <summary>
    /// 一時停止
    /// </summary>
    void Pause();

    /// <summary>
    /// 一時停止から再開
    /// あるいは始めから再生
    /// </summary>
    void Resume();

    /// <summary>
    /// フェードイン(未実装)
    /// </summary>
    /// <param name="_fadeTime">フェードインにかかる時間(秒)</param>
    void FadeIn(float _fadeTime);

    /// <summary>
    /// フェードアウト(未実装)
    /// </summary>
    /// <param name="_fadeTime">フェードアウトにかかる時間(秒)</param>
    void FadeOut(float _fadeTime);

    /// <summary>
    /// 音量を設定
    /// </summary>
    /// <param name="_volume">0.0f から 1.0f の範囲で設定</param>
    void SetVolume(float _volume);

    /// <summary>
    /// 音量を取得
    /// </summary>
    /// <returns>音量</returns>
    float GetVolume() const { return volume_; }

    /// <summary>
    /// 再生中かどうかを確認
    /// </summary>
    /// <returns>再生中なら true</returns>
    bool IsPlaying() const;

    /// <summary>
    /// 一時停止中かどうかを確認
    /// </summary>
    /// <returns>一時停止中なら true</returns>
    bool IsPaused() const { return isPaused_; }

    /// <summary>
    /// 経過時間を取得
    /// </summary>
    /// <returns>経過時間(秒)</returns>
    float GetElapsedTime() const;


private:
    /// <summary>
    /// HRESULTをチェックし、エラーがあれば例外を投げる
    /// </summary>
    void CheckHRESULT() const ;

    // 一時停止
    bool isPaused_ = false;

    // フェードイン中かどうか
    bool isFadingIn_ = false;

    // 音量
    float volume_ = 1.0f;

    // サンプルレート
    float sampleRate_ = 44100.0f;

    // 音声ソースボイス
    IXAudio2SourceVoice* sourceVoice_ = nullptr;


    HRESULT hr_ = S_OK;

};