#pragma once
#include <string>


namespace Engine
{

enum class AudioEffectType
{
    Native, // コードに記述したエフェクト
    VST3 // VST3プラグインエフェクト
};

struct AudioEffectDef
{
    std::string name; // エフェクトの名前
    std::string path; // エフェクトのファイルパス(vst)
    std::string className; // エフェクトのクラス名(vst)
    AudioEffectType type; // エフェクトの種類
};

}// namespace Engine