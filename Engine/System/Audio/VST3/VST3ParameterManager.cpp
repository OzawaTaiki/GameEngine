#include "VST3ParameterManager.h"
#include "pluginterfaces/base/ustring.h"

void Engine::VST3ParameterManager::Initialize(Steinberg::Vst::IEditController* controller)
{
    controller_ = controller;
}

int32_t Engine::VST3ParameterManager::GetParameterCount() const
{
    if (!controller_)
        return 0;

    return controller_->getParameterCount();
}

std::string Engine::VST3ParameterManager::GetParameterName(int32_t index) const
{
    if (!controller_)
        return "";

    Steinberg::Vst::ParameterInfo info;
    if (controller_->getParameterInfo(index, info) != Steinberg::kResultOk)
        return "";

    // Steinberg::UString128はVST3 SDKで文字列を扱うためのクラス
    // Steinberg::UString128 はwchar_t の文字列を扱うクラスなので、ASCII文字列に変換してからstd::stringにする
    Steinberg::UString128 ustr(info.title);
    char buffer[128];
    ustr.toAscii(buffer, sizeof(buffer));
    return std::string(buffer);
}

double Engine::VST3ParameterManager::GetParameter(Steinberg::Vst::ParamID id) const
{
    if (!controller_)
        return 0.0;

    return controller_->getParamNormalized(id);
}

void Engine::VST3ParameterManager::SetParameter(Steinberg::Vst::ParamID id, double normalizedValue)
{
    if (!controller_)
        return;

    controller_->setParamNormalized(id, normalizedValue);
}
