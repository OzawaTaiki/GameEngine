#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>

static std::string NormalizeKey(const std::string& s) {
    if (s.empty()) return s;
    std::string result = s;
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    for (size_t i = 1; i < result.size(); ++i) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}



class JsonBinder;
class CollisionLayerManager
{
public:
    // インスタンスを取得する
    static CollisionLayerManager* GetInstance();
    // Layerを取得する
    uint32_t GetLayer(const std::string& _layer);
    const std::string& GetLayerStr(uint32_t _layer) const;

    void InitJsonBinder();

private:

    // Layerのマップ
    std::map<std::string, uint32_t> layerMap_;
    std::vector<std::string> layerNames_;

    std::unique_ptr<JsonBinder> jsonBinder_;


private:
    CollisionLayerManager();
    ~CollisionLayerManager();
    CollisionLayerManager(const CollisionLayerManager&) = delete;
    CollisionLayerManager& operator=(const CollisionLayerManager&) = delete;

};