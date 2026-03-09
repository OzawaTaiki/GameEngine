#pragma once

#include <vector>

#include <System/Audio/AudioEffectBase.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>

struct __declspec(uuid("12345678-1234-1234-1234-123456789ABC")) VST3EffectTag {};

namespace Engine
{

class VST3Plugin;

class VST3Effect : public AudioEffectBase
{
public:

    static HRESULT Create(VST3Plugin* plugin, IUnknown** effect);

    STDMETHOD_(void, Process)(
        UINT32 inputCount,
        const XAPO_PROCESS_BUFFER_PARAMETERS* input,
        UINT32 outputCount,
        XAPO_PROCESS_BUFFER_PARAMETERS* output,
        BOOL isEnabled) override;

private:

    VST3Effect(VST3Plugin* plugin);

private:

    VST3Plugin* plugin_ = nullptr;

    std::vector<float> inputBuf_;
    std::vector<float> outputBuf_;

    Steinberg::Vst::ProcessData processData_ = {};
    Steinberg::Vst::AudioBusBuffers inputBusBuffers_ = {};
    Steinberg::Vst::AudioBusBuffers outputBusBuffers_ = {};

};

}// namespace Engine