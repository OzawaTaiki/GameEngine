#include "AudioSystem.h"

#include <System/Audio/SoundInstance.h>
#include <Debug/Debug.h>

AudioSystem* AudioSystem::GetInstance()
{
    static AudioSystem instance;
    return &instance;
}

void AudioSystem::Initialize()
{
    HRESULT hresult = S_FALSE;

    // エンジンのインスタンスを生成
    hresult = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);

    // マスターボイスを生成
    hresult = xAudio2_->CreateMasteringVoice(&masterVoice_);
}

void AudioSystem::Finalize()
{
    masterVoice_->DestroyVoice();
    masterVoice_ = nullptr;
    xAudio2_.Reset();

    for (auto sound : sounds_)
        SoundUnLoad(&sound);
}

std::shared_ptr<SoundInstance> AudioSystem::Load(const std::string& _filename)
{
    Debug::Log("Loading sound file: " + _filename);

    std::ifstream file;
    file.open(_filename, std::ios_base::binary);
    if (!file.is_open())
    {
        Debug::Log("Error: File not found - " + _filename + "\n");
        throw std::runtime_error("Error: File not found - " + _filename);
        return nullptr;
    }

    // RIFFヘッダーの読み込み
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
    {
        Debug::Log("Error: Not a RIFF - " + _filename + "\n");
        throw std::runtime_error("Error: Not a RIFF");
        return nullptr;
    }
    if (strncmp(riff.type, "WAVE", 4) != 0)
    {
        Debug::Log("Error: Not a WAVE - " + _filename + "\n");
        throw std::runtime_error("Error: Not a WAVE");
        return nullptr;
    }

    // フォーマットチャンクとデータチャンクを見つけるまで繰り返し
    FormatChunk format = {};
    ChunkHeader data = {};
    bool foundFormat = false;
    bool foundData = false;

    while (!foundFormat || !foundData)
    {
        ChunkHeader header;
        file.read((char*)&header, sizeof(ChunkHeader));

        // ファイル終端チェック
        if (file.eof())
        {
            Debug::Log("Error: Unexpected end of file - " + _filename + "\n");
            throw std::runtime_error("Error: Unexpected end of file - " + _filename);
        }

        // フォーマットチャンク
        if (strncmp(header.id, "fmt ", 4) == 0)
        {
            format.chunk = header;
            // フォーマットデータのサイズチェック
            if (header.size > sizeof(format.fmt))
            {
                // 追加データがある場合は一時バッファへ読み込む
                std::vector<char> tempBuffer(header.size);
                file.read(tempBuffer.data(), header.size);
                memcpy(&format.fmt, tempBuffer.data(), sizeof(format.fmt));
            }
            else
            {
                file.read((char*)&format.fmt, header.size);
            }
            foundFormat = true;
        }
        // データチャンク
        else if (strncmp(header.id, "data", 4) == 0)
        {
            data = header;
            foundData = true;
            break; // データチャンクが見つかったら終了
        }
        // その他のチャンク（スキップ）
        else
        {
            file.seekg(header.size, std::ios_base::cur);
        }
    }

    if (!foundFormat)
    {
        Debug::Log("Error: No format chunk found - " + _filename + "\n");
        throw std::runtime_error("Error: No format chunk found - " + _filename);
        return nullptr;
    }

    if (!foundData)
    {
        Debug::Log("Error: No data chunk found - " + _filename + "\n");
        throw std::runtime_error("Error: No data chunk found - " + _filename);
        return nullptr;
    }

    // データの読み込み
    char* pbuffer = new char[data.size];
    file.read(pbuffer, data.size);
    file.close();

    SoundData soundData = {};
    soundData.bufferSize = data.size;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pbuffer);
    soundData.wfex = format.fmt;
    soundData.path = _filename;

    Debug::Log("Sound file loaded successfully: " + _filename);

    sounds_.push_back(soundData);

    // 音声データのIDを取得
    uint32_t soundId = static_cast<uint32_t>(sounds_.size() - 1);
    pathToid_[_filename] = soundId;

    Debug::Log("\tID : " + std::to_string(soundId) + "\n");
    float sampleRate = static_cast<float>(soundData.wfex.nSamplesPerSec);

    auto soundindtance = std::make_shared<SoundInstance>(soundId, this, sampleRate);
    soundInstances_[soundId] = soundindtance;

    Debug::Log("=== Detailed Audio Analysis ===");

    // 変換処理
    std::vector<float> samples;
    int bytesPerSample = soundData.wfex.wBitsPerSample / 8;
    int totalSamples = soundData.bufferSize / bytesPerSample;
    int channels = soundData.wfex.nChannels;

    if (soundData.wfex.wBitsPerSample == 16)
    {
        const short* shortData = reinterpret_cast<const short*>(soundData.pBuffer);

        // ステレオの場合はモノラル化
        for (int i = 0; i < totalSamples; i += channels)
        {
            float mixedSample = 0.0f;
            for (int ch = 0; ch < channels; ch++)
            {
                mixedSample += shortData[i + ch] / 32768.0f;
            }
            samples.push_back(mixedSample / channels);
        }
    }

    // 統計計算
    if (!samples.empty())
    {
        float minVal = *std::min_element(samples.begin(), samples.end());
        float maxVal = *std::max_element(samples.begin(), samples.end());
        float rms = 0.0f;

        for (float sample : samples)
        {
            rms += sample * sample;
        }
        rms = sqrt(rms / samples.size());

        Debug::Log("Converted Float Stats:");
        Debug::Log("  Min: " + std::to_string(minVal));
        Debug::Log("  Max: " + std::to_string(maxVal));
        Debug::Log("  RMS: " + std::to_string(rms));
        Debug::Log("  Peak: " + std::to_string(std::max(abs(minVal), abs(maxVal))));
    }

    return soundindtance;
}

void AudioSystem::SetMasterVolume(float _volume)
{
    if (masterVoice_)
    {
        masterVoice_->SetVolume(_volume);
        masterVolume_ = _volume;
    }
}

void AudioSystem::SoundUnLoad(SoundData* _soundData)
{
    delete[] _soundData->pBuffer;

    _soundData->pBuffer = 0;
    _soundData->bufferSize = 0;
    _soundData->wfex = {};
    _soundData->path = "";
}

AudioSystem::~AudioSystem()
{
    Finalize();
}


