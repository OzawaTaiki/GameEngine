#include "VST3Effect.h"

#include "VST3Plugin.h"
#include <cstring>

namespace
{
const XAPO_REGISTRATION_PROPERTIES kRegProps = {
    __uuidof(VST3EffectTag),
    L"VST3Effect",
    L"Engine",
    1,0,
    XAPO_FLAG_INPLACE_SUPPORTED | XAPO_FLAG_INPLACE_REQUIRED,
    1,1,1,1
};
}

HRESULT Engine::VST3Effect::Create(VST3Plugin* plugin, IUnknown** effect)
{
    if (!plugin || !effect)
        return E_INVALIDARG;

    auto* newEffect = new VST3Effect(plugin);
    *effect = static_cast<IXAPO*>(newEffect);

    return S_OK;
}

void STDMETHODCALLTYPE Engine::VST3Effect::Process(UINT32 , const XAPO_PROCESS_BUFFER_PARAMETERS* input, UINT32 , XAPO_PROCESS_BUFFER_PARAMETERS* output, BOOL isEnabled)
{
    if (!isEnabled || !plugin_ || !plugin_->IsActive())
    {
        PassThrough(input, output);
        return;
    }

    if (!input || !output)
        return;

    const float*    src     = static_cast<const float*>(input[0].pBuffer);
    float*          dst     = static_cast<float*>(output[0].pBuffer);
    uint32_t        frames  = input[0].ValidFrameCount;

    // バッファサイズを確保
    inputBuf_ .resize(frames * channels_);
    outputBuf_.resize(frames * channels_);

    // interleaved -> planar変換
    for (uint32_t ch = 0; ch < channels_; ++ch)
    {
        for (uint32_t i = 0; i < frames; ++i)
        {
            inputBuf_[ch * frames + i] = src[i * channels_ + ch];
        }
    }

    // VST3 processDataの構築
    std::vector<float*> inputPtrs(channels_);
    std::vector<float*> outputPtrs(channels_);
    for (uint32_t ch = 0; ch < channels_; ++ch)
    {
        inputPtrs[ch] = &inputBuf_[ch * frames];
        outputPtrs[ch] = &outputBuf_[ch * frames];
    }

    inputBusBuffers_.numChannels        = channels_;
    outputBusBuffers_.numChannels       = channels_;
    inputBusBuffers_.channelBuffers32   = inputPtrs.data();
    outputBusBuffers_.channelBuffers32  = outputPtrs.data();

    processData_.numSamples       = static_cast<Steinberg::int32>(frames);
    processData_.numInputs       = 1;
    processData_.numOutputs      = 1;
    processData_.inputs          = &inputBusBuffers_;
    processData_.outputs         = &outputBusBuffers_;
    processData_.symbolicSampleSize = Steinberg::Vst::kSample32;

    plugin_->Process(processData_);

    // planar -> interleved変換
    for (uint32_t ch = 0; ch < channels_; ++ch)
    {
        for (uint32_t i = 0; i < frames; ++i)
        {
            dst[i * channels_ + ch] = outputBuf_[ch * frames + i];
        }
    }
}
