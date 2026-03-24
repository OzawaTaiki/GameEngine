#include "AudioEffectManager.h"

#include <Debug/Debug.h>
#include <Features/Json/Loader/JsonFileIO.h>
#include <Features/Json/JsonSerializers.h>
#include <System/Audio/AudioEffectDef.h>

#include <System/Audio/VST3/VST3Host.h>
#include <System/Audio/VST3/VST3Module.h>
#include <System/Audio/VST3/VST3Plugin.h>
#include <System/Audio/VST3/VST3Effect.h>
#include <System/Audio/VST3/VST3ParameterManager.h>
#include "AudioEffect.h"

namespace Engine
{

AudioEffectManager* AudioEffectManager::GetInstance()
{
    static AudioEffectManager instance;
    return &instance;
}

void AudioEffectManager::Initialize()
{
    effectDefs_.clear();
    loadedModules_.clear();
    nativeFactories_.clear();
}

void AudioEffectManager::Finalize()
{
}

void AudioEffectManager::LoadEffectData(const std::string& jsonPath)
{
    json jsonData = JsonFileIO::Load(jsonPath, "");
    if (jsonData.empty())
    {
        Debug::Log("Failed to load effect data from: " + jsonPath + "\n");
        return;
    }

    if (!jsonData.contains("effects"))
        return;

    auto host = VST3Host::GetInstance();

    for (const auto& entry : jsonData["effects"])
    {
        AudioEffectDef def = entry.get<AudioEffectDef>(); // from_json(AudioEffectDef) を使用
        effectDefs_[def.name] = def;
        auto module = host->LoadModule(def.path);
        if (!module)
        {
            Debug::Log("Failed to load VST3 module for effect: " + def.name + "\n");
            continue;
        }
        auto classes = module->GetAudioEffectClasses();
        if (!classes.empty())
        {
            if (def.className.empty())
            {
                def.className = classes[0].name(); // クラス名が指定されていない場合は最初のクラスを使用
            }
            for (const auto& cls : classes)
            {
                if (cls.name() == def.className)
                {
                    VST3PluginEntry pluginEntry;
                    pluginEntry.plugin = module->CreatePlugin(cls);
                    // TODO : プラグインの初期化パラメーターは要検討。とりあえず固定値で入れてみる
                    pluginEntry.plugin->Initialize(module->GetFactory(),
                                                   host->GetHostApp(),
                                                   48000.0f,
                                                   4096,
                                                   2,
                                                   2);
                    pluginEntry.paramMgr.Initialize(pluginEntry.plugin->GetController());

                    VST3ModuleEntry& moduleEntry = loadedModules_[def.path];
                    moduleEntry.module = module;
                    moduleEntry.plugins[def.name] = std::make_unique<VST3PluginEntry>(std::move(pluginEntry));

                    break;
                }
            }
        }

    }
}

void AudioEffectManager::RegisterNativeEffect(const std::string& name, std::function<IUnknown* ()> creator)
{
    if (!creator)
    {
        Debug::Log("Create Function is null for effect: " + name + "\n");
        return;
    }

#ifdef _DEBUG
    // 同名のエフェクトがすでに登録されている場合は警告を出す（上書きはする）
    auto it = nativeFactories_.find(name);
    if (it != nativeFactories_.end())
    {
        Debug::Log("Warning: Native effect already registered with name: " + name + "\n");
    }

#endif // _DEBUG

    nativeFactories_[name] = creator;
}

Engine::AudioEffectChain AudioEffectManager::BuildEffectChain(const std::vector<std::string>& effectNames)
{
    Engine::AudioEffectChain effectChain = {};

    for (const std::string& effectName : effectNames)
    {
        auto defIt = effectDefs_.find(effectName);
        if (defIt == effectDefs_.end())
            continue;

        IUnknown* xapo = nullptr;
        if(defIt->second.type==AudioEffectType::VST3)
        {

            auto pluginEntry = GetVST3PluginEntry(effectName);
            if (!pluginEntry)
                continue;

            if (SUCCEEDED(VST3Effect::Create(pluginEntry->plugin.get(), &xapo)) && xapo)
            {
                pluginEntry->paramMgr.SetEffect(static_cast<VST3Effect*>(static_cast<IXAPO*>(xapo)));
            }
        }
        else if (defIt->second.type == AudioEffectType::Native)
        {
            auto factoryIt = nativeFactories_.find(effectName);
            if (factoryIt == nativeFactories_.end())
                continue;

            xapo = factoryIt->second();
        }

        if (xapo)
        {
            effectChain.AddEffect(AudioEffect(xapo, 2, false));
            xapo->Release();
        }
    }

    return effectChain;
}

VST3ParameterManager* AudioEffectManager::GetParameterManager(const std::string& effectName)
{
    for (auto& def : effectDefs_)
    {
        if (def.first == effectName && def.second.type == AudioEffectType::VST3)
        {
            return GetVST3PluginEntry(effectName) ? &GetVST3PluginEntry(effectName)->paramMgr : nullptr;
        }
    }/*
    for (auto& modulePair : loadedModules_)
    {
        auto& plugins = modulePair.second.plugins;
        auto it = plugins.find(effectName);
        if (it != plugins.end())
        {
            return &it->second->paramMgr;
        }
    }*/
    return nullptr; // エフェクトが見つからない場合はnullptrを返す
}

AudioEffectManager::VST3PluginEntry* AudioEffectManager::GetVST3PluginEntry(const std::string& effectName)
{
    for (auto& modulePair : loadedModules_)
    {
        auto& plugins = modulePair.second.plugins;
        auto it = plugins.find(effectName);
        if (it != plugins.end())
        {
            return it->second.get();
        }
    }
    return nullptr; // エフェクトが見つからない場合はnullptrを返す
}


}// namespace Engine
