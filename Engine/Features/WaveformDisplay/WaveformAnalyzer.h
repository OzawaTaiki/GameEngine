#pragma once

#include <System/Audio/SoundInstance.h>

#include <vector>
#include <cstdint>


struct WaveformDara
{
    std::vector<float> rmsValues; // RMS値の配列
};

namespace Waveform
{
// 圧縮レベル
enum class CompressionLevel
{
    None,       // 圧縮なし（生データそのまま）100%
    Low,        // 軽い圧縮（細かい変化も見える）80%
    Medium,     // 中程度圧縮（標準的な概観） 60%
    High,       // 高圧縮（大まかな流れのみ) 40%
    Ultra       // 超高圧縮（全体の輪郭のみ) 20%
};
}// namespace Waveform

class WaveformAnalyzer
{
public:
    WaveformAnalyzer() = default;
    ~WaveformAnalyzer() = default;

    /// <summary>
    /// 生波形データのPeakの抽出
    /// </summary>
    /// <param name="_soundInstance">解析する音声インスタンス</param>
    /// <param name="_displayWidth">表示幅(ピクセル)</param>
    /// <param name="_displayDuration">表示時間(秒)</param>
    /// <returns>生波形データのPeakの配列</returns>
    static std::vector<float> ExtractRawWaveformMaxMin(const SoundInstance* _soundInstance, float displayWidth, float displayDuration);

    //*=====================
    // RMS : Root Mean Square
    //      一定時間内の音声信号のエネルギーを計算し、その平均的な大きさを表す
    //*=====================

    /// <summary>
    /// RMS解析
    /// </summary>
    /// <param name="_soundInstance">解析する音声インスタンス</param>
    /// <param name="_compressionLevel">圧縮レベル</param>
    /// <returns>RMS値の配列</returns>
    static std::vector<float> AnalyzeRMS(const SoundInstance* _soundInstance, Waveform::CompressionLevel _compressionLevel = Waveform::CompressionLevel::Medium);

    /// <summary>
    /// RMS解析
    /// </summary>
    /// <param name="_soundInstance">解析する音声インスタンス</param>
    /// <param name="_displayWidth">表示幅(ピクセル)</param>
    /// <param name="_displayDuration">表示時間(秒)</param>
    /// <returns>RMS値の配列</returns>
    static std::vector<float> AnalyzeRMS(const SoundInstance* _soundInstance, float _displayWidth, float _displayDuration);




private:
    //*=====================
    // メンバ関数
    //*=====================


    /// <summary>
    /// 圧縮レベルから圧縮率を取得
    /// </summary>
    /// <param name="_compressionLevel">圧縮レベル</param>
    /// <returns>圧縮率(0.0~1.0)</returns>
    static float GetCompressionRatio(Waveform::CompressionLevel _compressionLevel);

    /// <summary>
    /// 圧縮率からウィンドウサイズを計算
    /// </summary>
    /// <param name="_compressionRate">0.0~1.0</param>
    /// <param name="totalDuration">音声の総時間(秒)</param>
    /// <returns>ウィンドウサイズ(ミリ秒)</returns>
    static float CompressionRateToWindowSize(float _compressionRate, float totalDuration);


private:
    //*=====================
    // メンバ変数
    //*=====================

    // 圧縮時に使用するウィンドウサイズ(ミリ秒)
    // 50ms分のデータを1つのRMS値に圧縮する


};