#include "AudioSpectrum.h"

#include <numbers>
#include <Debug/Debug.h>
#include <numeric>

AudioSpectrum::AudioSpectrum(size_t windowSize, float _overlapRatio)
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

// x(n) = (1/N) * Σ[k=0 to N-1] X(k) * e^(+i*2π*k*n/N)
std::vector<float> AudioSpectrum::IDFT(const std::vector<std::complex<float>>& _input)
{

    size_t N = _input.size();
    float fN = static_cast<float>(N);

    if (N == 0)
        return {};

    // 出力配列の確保
    std::vector<float> output(N);

    for (size_t n = 0; n < N; ++n)// 時間のインデックス(出力)
    {
        std::complex<float> sum(0.0f, 0.0f);

        for (size_t k = 0; k < N; ++k)// 周波数のインデックス(入力)
        {
            // e^(+i*2π*k*n/N) = cos(2π*k*n/N) + i*sin(2π*k*n/N)
            float angle = 2 * std::numbers::pi_v<float> *static_cast<float>(k) * static_cast<float>(n) / fN; // 2π*k*n/N
            std::complex<float> complexValue(std::cos(angle), std::sin(angle)); // e^(+i*2π*k*n/N)

            sum += _input[k] * complexValue; // X(k) * e^(+i*2π*k*n/N)
        }

        output[n] = sum.real() / fN; // x(n) = 1/N * sum; // 実数部を取り出して正規化
    }

    return output;
}

void AudioSpectrum::Butterfly2(std::complex<float>& _x0, std::complex<float>& _x1)
{
    std::complex<float> temp = _x0;
    _x0 = temp + _x1;
    _x1 = temp - _x1;
}

void AudioSpectrum::Butterfly4(std::complex<float>& _x0, std::complex<float>& _x1, std::complex<float>& _x2, std::complex<float>& _x3)
{
    std::complex<float> E0 = _x0;    // E[0]コピー
    std::complex<float> E1 = _x2;    // E[1]コピー

    std::complex<float> O0 = _x1;    // O[0]コピー
    std::complex<float> O1 = _x3;    // O[1]コピー


    /// 分割
    Butterfly2(E0, E1); // even E[0],E[1]
    Butterfly2(O0, O1); //odd  O[0],O[1]


    // 回転因子
    std::complex<float> w0(1.0f, 0.0f);     // e^(-i*2π*0/4) = 1
    std::complex<float> w1(0.0f, -1.0f);    // e^(-i*2π*1/4) = -i


    /// 結合
    _x0 = E0 + w0 * O0; // X[0] = E[0] + w^0 * O[0]
    _x2 = E0 - w0 * O0; // X[2] = E[0] - w^0 * O[0]
    _x1 = E1 + w1 * O1; // X[1] = E[1] + w^1 * O[1]
    _x3 = E1 - w1 * O1; // X[3] = E[1] - w^1 * O[1]
}

void AudioSpectrum::Butterfly8(std::array<std::complex<float>, 8>& _x)
{

    std::complex<float> E0 = _x[0]; // E[0]コピー
    std::complex<float> E1 = _x[2]; // E[1]コピー
    std::complex<float> E2 = _x[4]; // E[2]コピー
    std::complex<float> E3 = _x[6]; // E[3]コピー

    std::complex<float> O0 = _x[1]; // O[0]コピー
    std::complex<float> O1 = _x[3]; // O[1]コピー
    std::complex<float> O2 = _x[5]; // O[2]コピー
    std::complex<float> O3 = _x[7]; // O[3]コピー

    // 分割 - 偶数と奇数に分ける
    Butterfly4(E0, E1, E2, E3); // even
    Butterfly4(O0, O1, O2, O3); // odd

    // 回転因子 w^k
    std::complex<float> w0(1.0f, 0.0f);                         // e^(-i*2π*0/8) = 1
    std::complex<float> w1(0.707f, -0.707f);                    // e^(-i*2π*1/8) = √2/2 - i√2/2
    std::complex<float> w2(0.0f, -1.0f);                        // e^(-i*2π*2/8) = -i
    std::complex<float> w3(-0.707f, -0.707f);                   // e^(-i*2π*3/8) = -√2/2 - i√2/2


    // 結合
    _x[0] = E0 + w0 * O0; // X[0] = E[0] + w^0 * O[0]
    _x[1] = E1 + w1 * O1; // X[1] = E[1] + w^1 * O[1]
    _x[2] = E2 + w2 * O2; // X[2] = E[2] + w^2 * O[2]
    _x[3] = E3 + w3 * O3; // X[3] = E[3] + w^3 * O[3]

    _x[4] = E0 - w0 * O0; // X[4] = E[0] - w^0 * O[0]
    _x[5] = E1 - w1 * O1; // X[5] = E[1] - w^1 * O[1]
    _x[6] = E2 - w2 * O2; // X[6] = E[2] - w^2 * O[2]
    _x[7] = E3 - w3 * O3; // X[7] = E[3] - w^3 * O[3]

}

void AudioSpectrum::FFT(const std::vector<float>& _input, std::vector<std::complex<float>>& _output)
{
    size_t N = _input.size();
    if (N == 0)
    {
        _output.clear();
        return;
    }
    // 入力サイズが2のべき乗でない場合は確保する
    size_t paddedSize = GetNextPowerOf2(N);
    if (paddedSize == 0)
    {
        // サイズが大きすぎる場合のエラーハンドリング
        throw std::runtime_error("Input size too large for FFT");
    }

    _output.resize(paddedSize);

    // 入力データを複素数配列にコピー
    for (size_t i = 0; i < N; ++i)
    {
        float windowValue = HanningWindowValue(N, i);
        _output[i] = std::complex<float>(_input[i] * windowValue, 0.0f); // 窓関数を適用
    }

    // 残りの要素をゼロで埋める
    for (size_t i = N; i < _output.size(); ++i)
    {
        _output[i] = std::complex<float>(0.0f, 0.0f);
    }

    // 再帰的にFFTを実行
    RecursiveFFT(_output);

}

std::vector<float> AudioSpectrum::GetSpectrumAtTime(float _time)
{
    if (std::abs(_time - cashedTime_) < 0.01f)
        return cashedSpectrum_;

    size_t centerIndex = static_cast<size_t>(sampleRate_ * _time);
    const size_t windowHalfSize = 1024 / 2;
    size_t s = centerIndex - windowHalfSize;
    size_t e = centerIndex + windowHalfSize;
    if (centerIndex < windowHalfSize)        s = 0;
    if (e >= audioData_.size()) e = audioData_.size() - 1;

    auto spectrum = ComputeSpectrum(_time, s, e);

    // キャッシュ更新
    cashedTime_ = _time;
    cashedSpectrum_ = spectrum;

    return spectrum;
}

void AudioSpectrum::RecursiveFFT(std::vector<std::complex<float>>& _x)
{
    size_t N = _x.size();
    float fN = static_cast<float>(N);
    size_t halfN = _x.size() / 2;

    if (N <= 1)
        return;

    std::vector<std::complex<float>> even(halfN);
    std::vector<std::complex<float>> odd(halfN);

    // 偶数要素と基数要素に分割
    for (size_t i = 0; i < halfN; ++i)
    {
        even[i] = _x[i * 2];
        odd[i] = _x[i * 2 + 1];
    }

    // 再帰 さらに分割していく
    RecursiveFFT(even);
    RecursiveFFT(odd);

    float angleStep = 2.0f * std::numbers::pi_v<float> / fN;
    float angle = 0.0f; // 回転因子
    for (size_t i = 0; i < halfN; ++i)
    {
        std::complex<float> w(std::cos(angle), -std::sin(angle));
        angle += angleStep;

        _x[i] = even[i] + w * odd[i];
        _x[i + halfN] = even[i] - w * odd[i];
    }
}

void AudioSpectrum::IterativeFFT(std::vector<std::complex<float>>& _x)
{
    size_t N = _x.size();
    float fN = static_cast<float>(N);
    size_t halfN = _x.size() / 2;

    std::vector<std::complex<float>> temp(N);
    for (size_t i = 0; i < N; ++i)
    {
        //bit反転

    }

}

std::vector<float> AudioSpectrum::ComputeSpectrum(float _time, size_t _startIndex, size_t _endIndex)
{
    std::vector<float> segment(1024, 0.0f);  // ゼロで初期化

    size_t centerIndex = static_cast<size_t>(sampleRate_ * _time);
    size_t windowHalfSize = 512;

    // 音声データから取得可能な範囲を計算
    size_t audioStart = (centerIndex >= windowHalfSize) ? centerIndex - windowHalfSize : 0;
    size_t audioEnd = std::min(centerIndex + windowHalfSize, audioData_.size());

    // セグメントバッファ内での配置位置を計算
    size_t bufferOffset = (centerIndex < windowHalfSize) ? windowHalfSize - centerIndex : 0;

    // 実際にコピーするサンプル数
    size_t copyLength = audioEnd - audioStart;

    // データをコピー
    if (copyLength > 0 && bufferOffset + copyLength <= 1024)
    {
        std::copy(audioData_.begin() + audioStart,
                  audioData_.begin() + audioEnd,
                  segment.begin() + bufferOffset);
    }

    // FFT実行
    std::vector<std::complex<float>> fftResult;
    FFT(segment, fftResult);

    // マグニチュード計算
    std::vector<float> magnitude(fftResult.size() / 2);

    // ハニング窓のゲイン補正係数 (窓関数の平均値の逆数)
    const float windowGain = 2.0f;
    // FFTの正規化係数
    const float fftNorm = 1.0f / static_cast<float>(fftResult.size());
    for (size_t i = 0; i < magnitude.size(); ++i)
    {
        magnitude[i] = std::abs(fftResult[i]) * windowGain * fftNorm;
    }

    return magnitude;

}

float AudioSpectrum::HanningWindowValue(size_t _N, size_t _n)
{
    if (_N == 0)
        return 0.0f;

    return 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> *static_cast<float>(_n) / static_cast<float>(_N - 1)));
}

size_t AudioSpectrum::GetNextPowerOf2(size_t _n)
{
    if (_n == 0)
        return 1;
    if (_n >= (SIZE_MAX >> 1)) return 0; // オーバーフロー防止

    if ((_n & (_n - 1)) == 0)
        return _n; // すでに2のべき乗

    size_t power = 1;
    while (power < _n)
    {
        power <<= 1;
    }
    return power;
}

void AudioSpectrum::RoundTripTest(const std::vector<float>& _input)
{
    Debug::Log("==========================\n");
    auto dft = DFT(_input);
    // 結果の表示
    for (size_t i = 0; i < _input.size(); ++i)
    {
        Debug::Log(std::format("X[{}]: {} + {}i\n", i, dft[i].real(), dft[i].imag()));
    }

    auto idft = IDFT(dft);
    for (size_t i = 0; i < idft.size(); ++i)
    {
        Debug::Log(std::format("x[{}]: {}\n", i, idft[i]));
    }

}
