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
    for (auto& [name, val] : mPtr_)
    {
        if (valueAddress_.contains(name))
        {
            // mPtr_に存在しないキーをvalueAddress_から削除
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, RefVector<uint32_t>> ||
                              std::is_same_v<T, RefVector<float>> ||
                              std::is_same_v<T, RefVector<Vector2>> ||
                              std::is_same_v<T, RefVector<Vector3>> ||
                              std::is_same_v<T, RefVector<Vector4>> ||
                              std::is_same_v<T, RefVector<std::string>>)
                {
                    // valueAddress_[name].addressがstd::vectorのとき
                    // mPtr_に存在しない値を削除
                    T v = std::get<T>(valueAddress_[name].address);
                    T ptr = std::get<T>(val.address);
                    // vectorの中身を取得
                    for (auto it = v.get().begin(); it != v.get().end();)
                    {
                        if (std::find(ptr.get().begin(), ptr.get().end(), *it) == ptr.get().end())
                        {
                            it = v.get().erase(it);
                        }
                        else
                        {
                            ++it;
                        }

                    }
                }
                       }, valueAddress_[name].address);
        }

        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_pointer_v<T>) {
                using U = std::remove_pointer_t<T>;
                T ptr = std::get<T>(val.address);
                if (ptr) {
                    *std::get<T>(valueAddress_[name].address) = *ptr;
                }
            }
            else if constexpr (std::is_same_v<T, RefVector<uint32_t>> ||
                               std::is_same_v<T, RefVector<float>> ||
                               std::is_same_v<T, RefVector<Vector2>> ||
                               std::is_same_v<T, RefVector<Vector3>> ||
                               std::is_same_v<T, RefVector<Vector4>> ||
                               std::is_same_v<T, RefVector<std::string>>)
            {
                T& v = std::get<T>(valueAddress_[name].address);
                T& ptr = std::get<T>(val.address);
                v.get() = ptr.get();
            }}, val.address);
    }

    ConfigManager::GetInstance()->SetDirectoryPath(folderPath_);
    ConfigManager::GetInstance()->SaveData(groupName_);
}
