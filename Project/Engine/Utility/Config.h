#pragma once

#include <string>

class Config
{
public:

    Config() = default;
    ~Config() = default;

    static Config* Create(const std::string& _sceneName, const std::string& _directoryPath = "resources/Data/Parameter");

    void LoadData();
    void SaveData();

    std::string GetSceneName() const { return sceneName_; }
    std::string GetDirectoryPath() const { return directoryPath_; }
    void SetDirectoryPath(const std::string& _directoryPath) { directoryPath_ = _directoryPath; }
private:
    std::string sceneName_;
    std::string directoryPath_ = "resources/Data/Parameter";


};