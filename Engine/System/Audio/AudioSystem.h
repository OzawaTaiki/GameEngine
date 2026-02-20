#pragma once


#include <fstream>
#include <array>
#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <wrl.h>

#include <xaudio2.h>

#pragma comment (lib,"xaudio2.lib")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")



namespace Engine {

class SoundInstance;

class AudioSystem
{
public:

    static AudioSystem* GetInstance();

    void Initialize();
    void Finalize();

    std::shared_ptr<SoundInstance> Load(const std::string& _filename);

    Microsoft::WRL::ComPtr<IXAudio2> GetXAudio2() { return xAudio2_; }

    void SetMasterVolume(float _volume);
    float GetMasterVolume() const { return masterVolume_; }

    const WAVEFORMATEX& GetSoundFormat(uint32_t _soundID) const { return sounds_[_soundID].wfex; }
    const BYTE* GetBuffer(uint32_t _soundID) const { return sounds_[_soundID].mediaData.data(); }
    size_t GetBufferSize(uint32_t _soundID) const { return sounds_[_soundID].mediaData.size(); }


private:

    struct SoundData
    {
        WAVEFORMATEX wfex;
        std::vector<BYTE> mediaData;
        std::string path;
    };

private:

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masterVoice_;

    //std::map <uint32_t, uint32_t> map_;

    //std::vector<SoundData> sounds_;
    //std::map<uint32_t, IXAudio2SourceVoice*> sourceVoice_;
    std::vector<SoundData> sounds_;
    std::map<std::string, uint32_t> pathToid_;

    std::map<uint32_t, std::weak_ptr<SoundInstance>> soundInstances_;

    float masterVolume_ = 1.0f;


    std::shared_ptr<SoundInstance> CreateSoundInstance(const WAVEFORMATEX& _wfex, std::vector<BYTE> _mediaData, const std::string& _path);


private:
    // 構造体たち
    struct ChunkHeader
    {
        char id[4];             // チャンク毎のID
        int32_t size;           // チャンクサイズ
    };

    struct RiffHeader
    {
        ChunkHeader chunk;      // "RIFF"
        char type[4];             // "WAVE"
    };

    struct FormatChunk
    {
        ChunkHeader chunk;      // ”fmt"
        WAVEFORMATEX fmt;       // 波形フォーマット
    };

private:

    AudioSystem() = default;
    ~AudioSystem();
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
};

} // namespace Engine
