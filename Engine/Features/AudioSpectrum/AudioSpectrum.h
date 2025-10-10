#pragma once

#include <vector>
#include <array>
#include <complex>

class AudioSpectrum
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="windowSize">FFTの窓サイズ (2のべき乗)</param>
    /// <param name="_overlapRatio">オーバーラップ率 (0.0f ~ 1.0f) 解像度と速度のトレードオフ</param>
    AudioSpectrum(size_t windowSize = 1024, float _overlapRatio = 0.5f);

    ~AudioSpectrum()= default;



    // 離散フーリエ変換
    std::vector<std::complex<float>> DFT(const std::vector<float>& _input);
    // 逆離散フーリエ変換
    std::vector<float> IDFT(const std::vector<std::complex<float>>& _input);


    static void Butterfly2(std::complex<float>& _x0, std::complex<float>& _x1);
    static void Butterfly4(std::complex<float>& _x0, std::complex<float>& _x1, std::complex<float>& _x2, std::complex<float>& _x3);
    static void Butterfly8(std::array<std::complex<float>, 8>& _x);

    // 高速フーリエ変換
    static void FFT(const std::vector<float>& _input, std::vector<std::complex<float>>& _output);

    std::vector<float> GetSpectrumAtTime(float _time);

    // 入出力のラウンドトリップテスト
    void RoundTripTest(const std::vector<float>& _input);


    void SetAudioData(const std::vector<float>& _audioData) { audioData_ = _audioData; }
    void SetSampleRate(float _sampleRate) { sampleRate_ = _sampleRate; }


    // 再帰的FFT
    static void RecursiveFFT(std::vector<std::complex<float>>& _x);

    // 反復的FFT
    static void IterativeFFT(std::vector<std::complex<float>>& _x);

private:

    std::vector<float> ComputeSpectrum(float _time, size_t _startIndex, size_t _endIndex);

    static float HanningWindowValue(size_t _N, size_t _n);

    // 次の2のべき乗を取得
    static size_t GetNextPowerOf2(size_t _n);



private:


    float cashedTime_ = 0.0f; // 保持している時間
    std::vector<float> cashedSpectrum_; // 保持しているスペクトラム

    std::vector<float> audioData_; // 入力音声データ
    float sampleRate_ = 44100.0f; // サンプリングレート

};