#pragma once

#include "AudioSpectrum.h"

#include <vector>
#include <complex>
#include <algorithm>
#include <iostream>


namespace Engine {

namespace SpectrumTest
{
void Test();
}

class SpectrumValidator
{
private:
    AudioSpectrum* spectrum_;
    int sampleRate_;

public:


    SpectrumValidator(AudioSpectrum* _spectrum, int _sampleRate)
        : spectrum_(_spectrum), sampleRate_(_sampleRate){}

    // 指定周波数にピークがあるかチェック
    bool HasPeakAt(const std::vector<float>& _spectrum, float _targetFreq, float _tolerance = 10.0f);

    // 複数周波数のピークチェック
    std::vector<bool> HasPeaksAt(const std::vector<float>& _spectrum, const std::vector<float>& _targetFreqs);

    // 詳細解析結果を出力
    void AnalyzeSpectrum(float _time, const std::vector<float>& _expectedFreqs, const std::string& _testName);
};

class SegmentedAudioGenerator
{
public:
    // 時間区間ごとに異なる周波数を生成
    static std::vector<float> GenerateSegmentedTones(int _sampleRate, float _totalDuration);

    // 和音の時間変化
    static std::vector<float> GenerateChordProgression(int _sampleRate, float _totalDuration);
};

} // namespace Engine
