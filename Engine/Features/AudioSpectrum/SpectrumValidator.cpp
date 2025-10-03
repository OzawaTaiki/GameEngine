#include "SpectrumValidator.h"
#include <Debug/Debug.h>
#include <chrono>

const float M_PI = 3.14159265358979323846f;

bool SpectrumValidator::HasPeakAt(const std::vector<float>& _spectrum, float _targetFreq, float _tolerance)
{
    size_t targetBin = static_cast<size_t>(_targetFreq * _spectrum.size() * 2 / sampleRate_);
    size_t toleranceBins = std::max<size_t>(1, static_cast<size_t>(_tolerance * _spectrum.size() * 2 / sampleRate_));

    size_t startBin = (targetBin > toleranceBins) ? targetBin - toleranceBins : 0;
    size_t endBin = std::min(targetBin + toleranceBins, _spectrum.size());

    float maxMagnitude = 0.0f;
    for (size_t i = startBin; i < endBin; ++i)
    {
        maxMagnitude = std::max(maxMagnitude, _spectrum[i]);
    }

    // 他の部分と比較してピークかどうか判定
    float averageMagnitude = 0.0f;
    for (float mag : _spectrum)
    {
        averageMagnitude += mag;
    }
    averageMagnitude /= _spectrum.size();

    return maxMagnitude > averageMagnitude * 3.0f; // 平均の3倍以上をピークとする
}

std::vector<bool> SpectrumValidator::HasPeaksAt(const std::vector<float>& _spectrum, const std::vector<float>& _targetFreqs)
{
    std::vector<bool> results;
    for (float freq : _targetFreqs)
    {
        results.push_back(HasPeakAt(_spectrum, freq));
    }
    return results;
}

void SpectrumValidator::AnalyzeSpectrum(float _time, const std::vector<float>& _expectedFreqs, const std::string& _testName)
{
    auto spectrumResult = spectrum_->GetSpectrumAtTime(_time);

    Debug::Log(L"\n=== " + std::wstring(_testName.begin(), _testName.end()) + L" (時刻: " + std::to_wstring(_time) + L"s) ===\n");

    for (float expectedFreq : _expectedFreqs)
    {
        bool hasPeak = HasPeakAt(spectrumResult, expectedFreq);
        Debug::Log(std::format(L"周波数 {}Hz: {}\n", expectedFreq, hasPeak ? L"✓ ピーク検出" : L"✗ ピーク未検出"));
    }

    // トップ5の周波数成分を表示
    Debug::Log(L"トップ5周波数成分:\n");
    std::vector<std::pair<float, float>> freqMag;

    for (size_t i = 0; i < spectrumResult.size(); ++i)
    {
        float freq = static_cast<float>(i) * sampleRate_ / (spectrumResult.size() * 2);
        freqMag.push_back({ freq, spectrumResult[i] });
    }

    std::sort(freqMag.begin(), freqMag.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (int i = 0; i < 5 && i < freqMag.size(); ++i)
    {
        Debug::Log(std::format(L"  {}. {}Hz (強度: {})\n", i + 1, freqMag[i].first, freqMag[i].second));
    }
}

std::vector<float> SegmentedAudioGenerator::GenerateSegmentedTones(int _sampleRate, float _totalDuration)
{
    size_t totalSamples = static_cast<size_t>(_totalDuration * _sampleRate);
    std::vector<float> audioData(totalSamples, 0.0f);

    // 各1秒区間で異なる周波数
    struct TimeSegment
    {
        float startTime;
        float endTime;
        float frequency;
        const char* note;
    };

    std::vector<TimeSegment> segments = {
        {0.0f, 10.0f, 261.63f, "C4"},  // ド
        //{1.0f, 2.0f, 293.66f, "D4"},  // レ
        //{2.0f, 3.0f, 329.63f, "E4"},  // ミ
        //{3.0f, 4.0f, 349.23f, "F4"},  // ファ
        //{4.0f, 5.0f, 392.00f, "G4"}   // ソ
    };

    for (const auto& segment : segments)
    {
        size_t startSample = static_cast<size_t>(segment.startTime * _sampleRate);
        size_t endSample = static_cast<size_t>(segment.endTime * _sampleRate);

        for (size_t i = startSample; i < endSample && i < totalSamples; ++i)
        {
            float time = static_cast<float>(i) / _sampleRate;
            audioData[i] = std::sin(2.0f * M_PI * segment.frequency * time);
        }
    }

    return audioData;
}

std::vector<float> SegmentedAudioGenerator::GenerateChordProgression(int _sampleRate, float _totalDuration)
{
    size_t totalSamples = static_cast<size_t>(_totalDuration * _sampleRate);
    std::vector<float> audioData(totalSamples, 0.0f);

    struct ChordSegment
    {
        float startTime;
        float endTime;
        std::vector<float> frequencies;
        const char* name;
    };

    std::vector<ChordSegment> chords = {
        {0.0f, 1.0f, {261.63f, 329.63f, 392.00f}, "C Major"},      // C-E-G
        {1.0f, 2.0f, {293.66f, 369.99f, 440.00f}, "D Minor"},      // D-F#-A
        {2.0f, 3.0f, {329.63f, 415.30f, 493.88f}, "E Minor"},      // E-G#-B
        {3.0f, 4.0f, {349.23f, 440.00f, 523.25f}, "F Major"},      // F-A-C
        {4.0f, 5.0f, {392.00f, 493.88f, 587.33f}, "G Major"}       // G-B-D
    };

    for (const auto& chord : chords)
    {
        size_t startSample = static_cast<size_t>(chord.startTime * _sampleRate);
        size_t endSample = static_cast<size_t>(chord.endTime * _sampleRate);

        for (size_t i = startSample; i < endSample && i < totalSamples; ++i)
        {
            float time = static_cast<float>(i) / _sampleRate;
            float sample = 0.0f;

            for (float freq : chord.frequencies)
            {
                sample += std::sin(2.0f * M_PI * freq * time) / chord.frequencies.size();
            }

            audioData[i] = sample;
        }
    }

    return audioData;
}

void SpectrumTest::Test()
{
    AudioSpectrum audioSpectrum(1024, 0.5f);
    const int sampleRate = 44100;
    const float duration = 5.0f;

    AudioSpectrum spectrum;
    SpectrumValidator validator(&spectrum, sampleRate);

    // テスト1: 単音の時間変化
    Debug::Log(L"\n############ 単音時間変化テスト ############\n");
    {
        auto audioData = SegmentedAudioGenerator::GenerateSegmentedTones(sampleRate, duration);
        spectrum.SetAudioData(audioData);
        spectrum.SetSampleRate(sampleRate);

        struct TestPoint
        {
            float time;
            float expectedFreq;
            const char* note;
        };

        std::vector<TestPoint> testPoints = {
            {0.5f, 261.63f, "C4"},
            {1.5f, 293.66f, "D4"},
            {2.5f, 329.63f, "E4"},
            {3.5f, 349.23f, "F4"},
            {4.5f, 392.00f, "G4"}
        };

        for (const auto& point : testPoints)
        {
            validator.AnalyzeSpectrum(point.time, { point.expectedFreq }, point.note);
        }
    }

    // テスト2: 和音の時間変化
    Debug::Log(L"\n############ 和音時間変化テスト ############\n");
    {
        auto audioData = SegmentedAudioGenerator::GenerateChordProgression(sampleRate, duration);
        spectrum.SetAudioData(audioData);
        spectrum.SetSampleRate(sampleRate);


        struct ChordTestPoint
        {
            float time;
            std::vector<float> expectedFreqs;
            const char* chordName;
        };

        std::vector<ChordTestPoint> chordTests = {
            {0.5f, {261.63f, 329.63f, 392.00f}, "C Major"},
            {1.5f, {293.66f, 369.99f, 440.00f}, "D Minor"},
            {2.5f, {329.63f, 415.30f, 493.88f}, "E Minor"},
            {3.5f, {349.23f, 440.00f, 523.25f}, "F Major"},
            {4.5f, {392.00f, 493.88f, 587.33f}, "G Major"}
        };

        for (const auto& chord : chordTests)
        {
            validator.AnalyzeSpectrum(chord.time, chord.expectedFreqs, chord.chordName);
        }
    }

    // テスト3: キャッシュの動作確認
    Debug::Log(L"\n############ キャッシュ動作テスト ############\n");
    {
        auto audioData = SegmentedAudioGenerator::GenerateSegmentedTones(sampleRate, duration);
        spectrum.SetAudioData(audioData);
        spectrum.SetSampleRate(sampleRate);


        // 同じ時刻を複数回呼び出してキャッシュが働くかテスト
        float testTime = 2.5f;

        auto start = std::chrono::high_resolution_clock::now();
        auto result1 = spectrum.GetSpectrumAtTime(testTime);
        auto end1 = std::chrono::high_resolution_clock::now();

        auto start2 = std::chrono::high_resolution_clock::now();
        auto result2 = spectrum.GetSpectrumAtTime(testTime); // キャッシュされるはず
        auto end2 = std::chrono::high_resolution_clock::now();

        auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start).count();
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

        Debug::Log(L"キャッシュ前後の実行時間比較:\n");
        Debug::Log(std::format(L"1回目: {} μs, 2回目: {} μs\n", duration1, duration2));
        Debug::Log(std::format(L"結果が同一: {}\n", (result1 == result2) ? L"✓" : L"✗"));
        Debug::Log("==========================\n");
        Debug::Log("\n");
    }


}
