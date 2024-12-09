#include "Config.h"

#include <sstream>


Config::Config(const std::string& _name) : groupName_(_name)
{
    std::ostringstream oss;
    oss << this;
    ptrString_ = oss.str();
}

void Config::Save() const
{
    for (auto [groupName, variable] : ptr_)
    {
        ConfigManager::GetInstance()->SaveData(groupName);
    }
}
