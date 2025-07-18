#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <string>
#include <vector>
#include <variant>

#include "json.hpp"

using json = nlohmann::json;

class JsonFileIO {

public:

    JsonFileIO();
    ~JsonFileIO();

    /// <summary>
    /// Jsonファイルを読み込む
    /// </summary>
    /// <param name="_filepath">Jsonファイルのパス</param>
    /// <param name="_directory">Jsonファイルのディレクトリ</param>
    static json Load(const std::string& _filepath, const std::string& _directory);

    /// <summary>
    /// Jsonファイルを保存する
    /// </summary>
    /// <param name="_filepath">Jsonファイルのパス</param>
    /// <param name="_directory">Jsonファイルのディレクトリ</param>
    static void Save(const std::string& _filepath, const std::string& _directory, const json& _data);

private:
    static std::string defaultDirectory;
};
