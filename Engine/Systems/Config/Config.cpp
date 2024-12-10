#include "Config.h"

#include <sstream>

Config::Config(const std::string& _name, const std::string& _folderpath) :
    groupName_(_name),
    folderPath_(_folderpath) {
    if (folderPath_.empty())
        folderPath_ = "resources/Data/Parameter/";

    if (folderPath_.back() != '/')
    {
        folderPath_ += "/";
    }

}

void Config::Save()
{

    for ( auto& [name, val] : mPtr_)
    {
        if (valueAddress_.contains(name))
        {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;  // 変数の型を推測
                if constexpr (std::is_pointer_v<T>) {
                    using U = std::remove_pointer_t<T>;  // ポインタの中身の型
                    T ptr = std::get<T>(val.address);  // ポインタを取得
                    if (ptr) {
                        // ポインタが有効であればvalueAddress_の対応する場所にコピー
                        // すでにvalueAddress_にポインタが格納されている
                        *std::get<T>(valueAddress_[name].address) = *ptr;
                    }
                }
                       }, val.address);
        }
        else
        {
            valueAddress_.erase(name);
        }
    }

    ConfigManager::GetInstance()->SetDirectoryPath(folderPath_);
    ConfigManager::GetInstance()->SaveData(groupName_);
}
