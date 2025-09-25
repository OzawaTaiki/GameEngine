#pragma once

#include <vector>
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

    std::vector<std::complex<float>> DFT(const std::vector<float>& _input);

private:



private:


    std::vector<std::complex<float>> fftBuffer_;

    size_t windowSize_;
    float overlapRatio_;
    size_t hopSize_;
    size_t writePos_;


};