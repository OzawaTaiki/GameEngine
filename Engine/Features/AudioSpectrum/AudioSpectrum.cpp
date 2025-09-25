#include "AudioSpectrum.h"

#include <numbers>

AudioSpectrum::AudioSpectrum(size_t windowSize, float _overlapRatio):
    windowSize_(windowSize),
    overlapRatio_(_overlapRatio),
    writePos_(0)
{
}

// X(k) = Σ[n=0 to N-1] x(n) * e^(-i*2π*k*n/N)
std::vector<std::complex<float>> AudioSpectrum::DFT(const std::vector<float>& _input)
{
    size_t N = _input.size();
    float fN = static_cast<float>(N);

    if (N == 0)
        return {};

    // 出力配列の確保
    std::vector<std::complex<float>> output(N);//複素数配列

    for (size_t k = 0; k < N; ++k)// 周波数のインデックス(出力)
    {
        std::complex<float> sum(0.0f, 0.0f);

        for (size_t n = 0; n < N; ++n)// 時間のインデックス(入力)
        {
            // e^(-i*2π*k*n/N) = cos(2π*k*n/N) - i*sin(2π*k*n/N)
            float angle = 2 * std::numbers::pi_v<float> *static_cast<float>(k) * static_cast<float>(n) / fN; // 2π*k*n/N
            std::complex<float> complexValue(std::cos(angle), -std::sin(angle)); // e^(-i*2π*k*n/N)

            sum += _input[n] * complexValue; // x(n) * e^(-i*2π*k*n/N)
        }

        output[k] = sum; // X(k) = ;
    }

    return output;
}
