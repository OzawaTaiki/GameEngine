#include "Config.h"

#include "ConfigManager.h"

Config* Config::Create(const std::string& _sceneName, const std::string& _directoryPath)
{
    Config* instance = ConfigManager::GetInstance()->Create(_sceneName);
    instance->sceneName_ = _sceneName;
    return instance;
}

void Config::LoadData()
{
    ConfigManager::GetInstance()->LoadData();
}

void Config::SaveData()
{
    ConfigManager::GetInstance()->SaveData();
}
