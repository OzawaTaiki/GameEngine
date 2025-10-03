#include "WaveformAnalyzer.h"

#include <System/Audio/AudioSystem.h>

#include <algorithm>
#include <Debug/Debug.h>


std::vector<float> WaveformAnalyzer::ExtractRawWaveformMaxMin(const SoundInstance* _soundInstance, float _displayWidth, float _displayDuration)
{
    AudioSystem* audioSystem = AudioSystem::GetInstance();
    std::vector<float> peakValues;

    // サンプルレート
    float sampleRate = _soundInstance->GetSampleRate();

    float duration = _soundInstance->GetDuration();

    // 1ピクセルあたりの時間
    float timePerPixel = _displayDuration / _displayWidth;

    // 音声データのいろいろ
    uint32_t soundID = _soundInstance->GetSoundID();
    const WAVEFORMATEX& format = audioSystem->GetSoundFormat(soundID);
    const BYTE* buffer = audioSystem->GetBuffer(soundID);
    size_t bufferSize = audioSystem->GetBufferSize(soundID);
    const float* waveformF = reinterpret_cast<const float*>(buffer); // float配列として扱う

    size_t sampleCount = static_cast<size_t>(duration * sampleRate * format.nChannels);

    size_t dataPerSec = static_cast<size_t>(sampleRate * format.nChannels);
    size_t dataPerPixel = static_cast<size_t>((dataPerSec * _displayDuration) / _displayWidth);

    for (size_t step = 0; step < sampleCount; step +=dataPerPixel)
    {
        float maxVal = -1.0f;
        float minVal = 1.0f;
        for (size_t i = 0; i < dataPerPixel; ++i)
        {
            size_t index = step + i;
            if (index >= sampleCount)
            {
                break;
            }
            float sample = waveformF[index];
            sample = std::clamp(sample, -1.0f, 1.0f);
            maxVal = std::max(maxVal, sample);
            minVal = std::min(minVal, sample);
        }
        peakValues.push_back(maxVal);
        peakValues.push_back(minVal);

    }
    return peakValues;

}

float WaveformAnalyzer::GetRMSAtTime(const SoundInstance* _soundInstance, float _time, float _windowSizeMs)
{
    AudioSystem* audioSystem = AudioSystem::GetInstance();
    // サンプルレート
    float sampleRate = _soundInstance->GetSampleRate();
    size_t windowSizeSamples = static_cast<size_t>(_windowSizeMs * 0.001f * sampleRate);
    uint32_t soundID = _soundInstance->GetSoundID();
    const WAVEFORMATEX& format = audioSystem->GetSoundFormat(soundID);
    const BYTE* buffer = audioSystem->GetBuffer(soundID);
    size_t bufferSize = audioSystem->GetBufferSize(soundID);
    const float* waveformF = reinterpret_cast<const float*>(buffer);
    // フォーマット情報
    size_t channels = static_cast<size_t>(format.nChannels);
    size_t totalFloats = bufferSize / sizeof(float);     // float要素の総数
    size_t totalFrames = totalFloats / channels;         // フレーム数
    size_t centerFrame = static_cast<size_t>(_time * sampleRate);
    size_t startFrame = (centerFrame >= windowSizeSamples / 2) ? (centerFrame - windowSizeSamples / 2) : 0;
    size_t endFrame = std::min(centerFrame + windowSizeSamples / 2, totalFrames);
    long double sumSquares = 0.0L;
    size_t framesInWindow = endFrame - startFrame;

    for (size_t frame = 0; frame < framesInWindow; ++frame)
    {
        // チャンネルごとの処理
        for (size_t ch = 0; ch < channels; ++ch)
        {
            size_t sampleIndex = (startFrame + frame) * channels + ch;
            if (sampleIndex >= totalFloats) // 範囲確認
                continue;
            float sample = waveformF[sampleIndex];
            sample = std::clamp(sample, -1.0f, 1.0f);
            sumSquares += static_cast<long double>(sample * sample);
        }
    }

    // 全チャンネル・全フレームでの平均二乗
    if (framesInWindow > 0 && channels > 0)
    {
        float meanSquare = static_cast<float>(sumSquares / (framesInWindow * channels));
        return std::sqrt(meanSquare);
    }

    return 0.0f;
}

std::vector<float> WaveformAnalyzer::AnalyzeRMS(const SoundInstance* _soundInstance, Waveform::CompressionLevel _compressionLevel)
{
    AudioSystem* audioSystem = AudioSystem::GetInstance();
    std::vector<float> rmsValues;

    // 圧縮レベルから圧縮率を取得
    float compressionRate = GetCompressionRatio(_compressionLevel);

    // 圧縮率からウィンドウサイズを計算
    float windowSizeMs = CompressionRateToWindowSize(compressionRate, _soundInstance->GetDuration());

    // サンプルレート
    float sampleRate = _soundInstance->GetSampleRate();
    size_t windowSizeSamples = static_cast<size_t>(windowSizeMs * 0.001f * sampleRate);

    uint32_t soundID = _soundInstance->GetSoundID();
    const WAVEFORMATEX& format = audioSystem->GetSoundFormat(soundID);
    const BYTE* buffer = audioSystem->GetBuffer(soundID);
    size_t bufferSize = audioSystem->GetBufferSize(soundID);
    const float* waveformF = reinterpret_cast<const float*>(buffer);

    // フォーマット情報
    size_t channels = static_cast<size_t>(format.nChannels);
    size_t totalFloats = bufferSize / sizeof(float);     // float要素の総数
    size_t totalFrames = totalFloats / channels;         // フレーム数


    // デバッグ：最初の数値をチェック
    Debug::Log("Buffer size: " + std::to_string(bufferSize));
    Debug::Log("Total floats: " + std::to_string(totalFloats));
    Debug::Log("Channels: " + std::to_string(channels));
    Debug::Log("Total frames: " + std::to_string(totalFrames));

    // 最初の10個のfloat値をチェック
    for (size_t i = 0; i < std::min(size_t(10), totalFloats); ++i)
    {
        float val = waveformF[i];
        Debug::Log("waveformF[" + std::to_string(i) + "] = " + std::to_string(val) +
                   (std::isnan(val) ? " (NaN!)" : "") +
                   (std::isinf(val) ? " (Inf!)" : ""));
    }

    for (size_t framePos = 0; framePos < totalFrames; framePos += windowSizeSamples)
    {
        long double sumSquares = 0.0L;
        size_t framesInWindow = std::min(windowSizeSamples, totalFrames - framePos);

        for (size_t frame = 0; frame < framesInWindow; ++frame)
        {
            // チャンネルごとの処理
            for (size_t ch = 0; ch < channels; ++ch)
            {
                size_t sampleIndex = (framePos + frame) * channels + ch;
                if (sampleIndex >= totalFloats) // 範囲確認
                    continue;

                float sample = waveformF[sampleIndex];
                sample = std::clamp(sample, -1.0f, 1.0f);
                sumSquares += static_cast<long double>(sample * sample);
                if (std::isnan(sumSquares))
                {
                    __debugbreak();
                }
            }
        }

        // 全チャンネル・全フレームでの平均二乗
        if (framesInWindow > 0 && channels > 0)
        {
            float meanSquare = static_cast<float>(sumSquares / (framesInWindow * channels));
            rmsValues.push_back(std::sqrt(meanSquare));
        }
    }

    return rmsValues;
}

std::vector<float> WaveformAnalyzer::AnalyzeRMS(const SoundInstance* _soundInstance, float _displayWidth, float _displayDuration)
{
    AudioSystem* audioSystem = AudioSystem::GetInstance();
    std::vector<float> rmsValues;

    // サンプルレート
    float sampleRate = _soundInstance->GetSampleRate();

    // 1ピクセルあたりの時間
    float timePerPixel = _displayDuration / _displayWidth;

    // ウィンドウサイズ（サンプル数）
    size_t windowSizeSamples = static_cast<size_t>(timePerPixel * sampleRate);
    windowSizeSamples = std::max(size_t(1), windowSizeSamples); // 最低1サンプル

    uint32_t soundID = _soundInstance->GetSoundID();
    const WAVEFORMATEX& format = audioSystem->GetSoundFormat(soundID);
    const BYTE* buffer = audioSystem->GetBuffer(soundID);
    size_t bufferSize = audioSystem->GetBufferSize(soundID);
    const float* waveformF = reinterpret_cast<const float*>(buffer);

    // フォーマット情報
    size_t channels = static_cast<size_t>(format.nChannels);
    size_t totalFloats = bufferSize / sizeof(float);     // float要素の総数
    size_t totalFrames = totalFloats / channels;         // フレーム数

    for (size_t framePos = 0; framePos < totalFrames; framePos += windowSizeSamples)
    {
        long double sumSquares = 0.0L;
        size_t framesInWindow = std::min(windowSizeSamples, totalFrames - framePos);
        for (size_t frame = 0; frame < framesInWindow; ++frame)
        {
            // チャンネルごとの処理
            for (size_t ch = 0; ch < channels; ++ch)
            {
                size_t sampleIndex = (framePos + frame) * channels + ch;
                if (sampleIndex >= totalFloats) // 範囲確認
                    continue;
                float sample = waveformF[sampleIndex];
                sample = std::clamp(sample, -1.0f, 1.0f);
                sumSquares += static_cast<long double>(sample * sample);
            }
        }
        // 全チャンネル・全フレームでの平均二乗
        if (framesInWindow > 0 && channels > 0)
        {
            float meanSquare = static_cast<float>(sumSquares / (framesInWindow * channels));
            rmsValues.push_back(std::sqrt(meanSquare));
        }
    }

    return rmsValues;
}


float WaveformAnalyzer::GetCompressionRatio(Waveform::CompressionLevel _compressionLevel)
{
    switch (_compressionLevel)
    {
        case Waveform::CompressionLevel::None:
            return 1.0f;
        case Waveform::CompressionLevel::Low:
            return 0.8f;
        case Waveform::CompressionLevel::Medium:
            return 0.6f;
        case Waveform::CompressionLevel::High:
            return 0.4f;
        case Waveform::CompressionLevel::Ultra:
            return 0.2f;
        default:
            return 0.6f; // デフォルトは中程度
    }
    return 0.0f;
}

float WaveformAnalyzer::CompressionRateToWindowSize(float _compressionRate, float totalDuration)
{
    float minWindowMs = 2.0f;   // 最高品質時（100%）
    float maxWindowMs = 50.0f; // 最低品質時（20%）

    // 圧縮率が低いほど大きなウィンドウ（粗い解析）
    float quality = _compressionRate / 100.0f;  // 0.2 ~ 1.0
    float windowMs = maxWindowMs - (maxWindowMs - minWindowMs) * quality;

    return windowMs;
}
