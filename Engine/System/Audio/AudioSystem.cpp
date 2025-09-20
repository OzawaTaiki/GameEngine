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
    CHECK_HR_VOID(hresult, "Failed to create XAudio2 instance");

    // マスターボイスを生成
    hresult = xAudio2_->CreateMasteringVoice(&masterVoice_);
    CHECK_HR_VOID(hresult, "Failed to create mastering voice");

    hresult = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    CHECK_HR_VOID(hresult, "Failed to startup Media Foundation");
}

void AudioSystem::Finalize()
{
    masterVoice_->DestroyVoice();
    masterVoice_ = nullptr;
    xAudio2_.Reset();

    sounds_.clear();        // ← 自動的にすべてのvectorが解放される
    pathToid_.clear();
    soundInstances_.clear();

    HRESULT hr = MFShutdown();
    CHECK_HR_VOID(hr, "Failed to shutdown Media Foundation");
}

std::shared_ptr<SoundInstance> AudioSystem::Load(const std::string& _filename)
{
    HRESULT hr = S_OK;

    Debug::Log("Loading sound file: " + _filename);

    // ワイド文字列に変換
    std::wstring wFilename(_filename.begin(), _filename.end());

    // ソースリーダーの生成
    Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };
    hr = MFCreateSourceReaderFromURL(wFilename.c_str(), NULL, &pMFSourceReader);
    CHECK_HR(hr, "Failed to create source reader from URL");

    // 出力メディアタイプの設定
    Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType{ nullptr };
    hr = MFCreateMediaType(&pMFMediaType);
    CHECK_HR(hr, "Failed to create media type");
    hr = pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio); // オーディオストリーム
    CHECK_HR(hr, "Failed to set major type");
    hr = pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);    // PCM形式
    CHECK_HR(hr, "Failed to set subtype");
    hr = pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType.Get()); // 出力メディアタイプの設定
    CHECK_HR(hr, "Failed to set current media type");

    //pMFMediaType->Release();

    // 現在のメディアタイプを取得
    hr = pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMFMediaType);
    CHECK_HR(hr, "Failed to get current media type");

    // WAVEFORMATEXの取得
    WAVEFORMATEX* waveFormat{ nullptr };
    hr = MFCreateWaveFormatExFromMFMediaType(pMFMediaType.Get(), &waveFormat, nullptr);
    CHECK_HR(hr, "Failed to create WAVEFORMATEX from media type");

    // メディアデータの読み込み
    std::vector<BYTE> mediaData;
    while (true)
    {
        Microsoft::WRL::ComPtr<IMFSample> pMFSample;    // サンプル
        DWORD dwStreamFlags{ 0 };           // ストリームの状態を格納するフラグ

        // サンプルの読み込み
        hr = pMFSourceReader->ReadSample(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            nullptr,
            &dwStreamFlags,
            nullptr,
            &pMFSample);
        CHECK_HR(hr, "Failed to read sample");

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            break; // 最後まで読み込んだら終了
        }

        // サンプルからメディアバッファを取得
        Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer;
        hr = pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer); // 連続したバッファに変換
        CHECK_HR(hr, "Failed to convert sample to contiguous buffer");

        // メディアバッファからデータを取得
        BYTE* pBuffer{ nullptr };
        DWORD cbCurrentLength{ 0 };
        hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength); // バッファのロック (ポインタを取得)
        CHECK_HR(hr, "Failed to lock media buffer");

        mediaData.resize(mediaData.size() + cbCurrentLength);
        memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);// データのコピー

        pMFMediaBuffer->Unlock(); // バッファのアンロック

    }

    auto soundInstance = CreateSoundInstance(*waveFormat, std::move(mediaData), _filename);

    CoTaskMemFree(waveFormat);

    return soundInstance;

}

void AudioSystem::SetMasterVolume(float _volume)
{
    if (masterVoice_)
    {
        masterVoice_->SetVolume(_volume);
        masterVolume_ = _volume;
    }
}

std::shared_ptr<SoundInstance> AudioSystem::CreateSoundInstance(const WAVEFORMATEX& _wfex, std::vector<BYTE> _mediaData, const std::string& _path)
{
    SoundData soundData{};
    soundData.wfex = _wfex;
    soundData.mediaData = std::move(_mediaData);
    soundData.path = _path;

    sounds_.push_back(std::move(soundData));

    uint32_t soundID = static_cast<uint32_t>(sounds_.size() - 1);
    pathToid_.emplace(_path, soundID);

    // サウンドインスタンスの生成
    auto soundInstance = std::make_shared<SoundInstance>(soundID, this, static_cast<float>(_wfex.nSamplesPerSec));
    // サウンドインスタンスの登録
    soundInstances_.emplace(soundID, soundInstance);

    Debug::Log("Sound Loaded : " + _path + " (ID " + std::to_string(soundID) + ")\n");

    // サウンドインスタンスを返す
    return soundInstance;
}

AudioSystem::~AudioSystem()
{
    Finalize();
}


