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
    AudioSpectrum(size_t windowSize = 1024);

    ~AudioSpectrum()= default;



    // 離散フーリエ変換
    std::vector<std::complex<float>> DFT(const std::vector<float>& _input);
    // 逆離散フーリエ変換
    std::vector<float> IDFT(const std::vector<std::complex<float>>& _input);


    static void Butterfly2(std::complex<float>& _x0, std::complex<float>& _x1);
    static void Butterfly4(std::complex<float>& _x0, std::complex<float>& _x1,
                           std::complex<float>& _x2, std::complex<float>& _x3);
    static void Butterfly8(std::array<std::complex<float>, 8>& _x);

    // 高速フーリエ変換
    static void FFT(const std::vector<float>& in, std::vector<std::complex<float>>& out);

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

    std::vector<float> ComputeSpectrum(float _time);

    static float HanningWindowValue(size_t _N, size_t _n);

    // 次の2のべき乗を取得
    static size_t GetNextPowerOf2(size_t _n);

    // ビット反転
    static uint32_t BitReversal(uint32_t _n, uint32_t _bite);

private:
    size_t windowSize_ = 1024; // 窓サイズ
    float cashedTime_ = 0.0f; // 保持している時間
    std::vector<float> cashedSpectrum_; // 保持しているスペクトラム

    std::vector<float> audioData_; // 入力音声データ
    float sampleRate_ = 44100.0f; // サンプリングレート

};