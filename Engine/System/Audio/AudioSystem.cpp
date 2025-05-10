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
    }

    // RIFFヘッダーの読み込み
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
    {
        Debug::Log("Error: Not a RIFF - " + _filename + "\n");
        throw std::runtime_error("Error: Not a RIFF");
    }
    if (strncmp(riff.type, "WAVE", 4) != 0)
    {
        Debug::Log("Error: Not a WAVE - " + _filename + "\n");
        throw std::runtime_error("Error: Not a WAVE");
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
    }

    if (!foundData)
    {
        Debug::Log("Error: No data chunk found - " + _filename + "\n");
        throw std::runtime_error("Error: No data chunk found - " + _filename);
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

    auto soundindtance = std::make_shared<SoundInstance>(soundId,this);
    soundInstances_[soundId] = soundindtance;

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


