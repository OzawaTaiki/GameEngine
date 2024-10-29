#include "Audio.h"
#include <cassert>

Audio::~Audio()
{
    xAudio2_.Reset();

    for (auto sound : sounds_)
        SoundUnLoad(&sound);
    sounds_.clear();
}

void Audio::Initialize()
{
    HRESULT hresult = S_FALSE;

    // エンジンのインスタンスを生成
    hresult = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);

    // マスターボイスを生成
    hresult = xAudio2_->CreateMasteringVoice(&masterVoice_);
}

uint32_t Audio::SoundLoadWave(const std::string& _filename)
{
    HRESULT hresult = S_FALSE;

    std::ifstream file;
    //waveをバイナリ形式で開く
    file.open(_filename, std::ios_base::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Error: File not found - " + _filename);
    }

    /// riffヘッダーの読み込み
    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));

    // riffチェック
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
        assert(0);
    // waveチェック
    if (strncmp(riff.type, "WAVE", 4) != 0)
        assert(0);

    /// formatチャンクの読み込み
    FormatChunk format = {};

    file.read((char*)&format, sizeof(ChunkHeader));
    if (strncmp(format.chunk.id, "fmt ", 4) != 0)
        assert(0);

    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    ///DATAチャンクの読み込み
    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
    // Junkファイルを見つけた時
    if (strncmp(data.id, "JUNK", 4) == 0)
    {
        // 読み取り位置をjunkの最後まですすめる
        file.seekg(data.size, std::ios_base::cur);

        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0)
        assert(0);

    char* pbuffer = new char[data.size];
    file.read(pbuffer, data.size);

    file.close();

    SoundData soundData = {};

    soundData.bufferSize = data.size;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pbuffer);
    soundData.wfex = format.fmt;

    sounds_.push_back(soundData);

    return static_cast<uint32_t>(sounds_.size() - 1);
}

void Audio::SoundUnLoad(SoundData* _soundData)
{
    //TODO: サウンド再生チェック
    delete[] _soundData->pBuffer;

    _soundData->pBuffer = 0;
    _soundData->bufferSize = 0;
    _soundData->wfex = {};
}

uint32_t Audio::SoundPlay(uint32_t _soundHandle, float _volume, bool _loop, bool _enableOverlap)
{
    // 重複再生が無効なら
    if (!_enableOverlap)
    {
        // 再生フラグを取得
        uint32_t voicehandle = map_[_soundHandle];
        if (IsPlaying(voicehandle))
        {// 再生されていたらリターン
            return voicehandle;
        }
    }

    HRESULT hresult = S_FALSE;

    SoundData data = sounds_[_soundHandle];

    IXAudio2SourceVoice* pSourceVoice = nullptr;
    hresult = xAudio2_->CreateSourceVoice(&pSourceVoice, &data.wfex);
    assert(SUCCEEDED(hresult));

    sourceVoice_.push_back(pSourceVoice);

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = data.pBuffer;
    buf.AudioBytes = data.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    if (_loop)
        buf.LoopCount = XAUDIO2_LOOP_INFINITE;

    hresult = pSourceVoice->SubmitSourceBuffer(&buf);
    assert(SUCCEEDED(hresult));

    hresult = pSourceVoice->Start();
    assert(SUCCEEDED(hresult));

    pSourceVoice->SetVolume(_volume);

    uint32_t index = static_cast<uint32_t>(sourceVoice_.size() - 1);
    map_[_soundHandle] = index;
    return index;
}

bool Audio::IsPlaying(uint32_t _voiceHandle) const
{
    if (sourceVoice_.size()==0||
        sourceVoice_.size() < _voiceHandle)
        return false;

    XAUDIO2_VOICE_STATE state;
    sourceVoice_[_voiceHandle]->GetState(&state);

    return state.BuffersQueued > 0;
}

void Audio::SetVolume(uint32_t _voiceHandle, float _volume)
{
    sourceVoice_[_voiceHandle]->SetVolume(_volume);
}

void Audio::SoundStop(uint32_t _voiceHandle)
{
    sourceVoice_[_voiceHandle]->Stop();
}
