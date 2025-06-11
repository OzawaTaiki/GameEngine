#pragma once

#include <Features/Json/Loader/JsonLoader.h>

#include <Math/Vector/Vector3.h>

#include <string>
#include <vector>
#include <map>


struct ColliderPrams
{
    std::string type = "box"; // デフォルトはボックスコライダー
    Vector3 center = Vector3(0, 0, 0); // デフォルトの位置
    Vector3 size = Vector3(1, 1, 1); // デフォルトのサイズ
};

struct ObjectParameters
{
    std::string name = ""; // オブジェクトの名前
    std::string modelPath = "";
    Vector3 position = Vector3(0, 0, 0);
    Vector3 rotation = Vector3(0, 0, 0);
    Vector3 scale = Vector3(1, 1, 1);

    bool hasChild = false; // 子オブジェクトを持つかどうか

    std::vector<ObjectParameters> childParameters; // 子オブジェクトのパラメータ

    std::vector<ColliderPrams> colliders; // コライダーのパラメータ
};

class LevelEditorLoader
{

public:

    LevelEditorLoader() = default;
    ~LevelEditorLoader() = default;

    void Load(const std::string& _filePath);

    ObjectParameters GetObjectParameter(const std::string& _name) const;
    std::map<std::string, ObjectParameters> GetAllObjectParameters() const { return objectParameters_; }


private:

    ObjectParameters GetObjectParameterFromJson(const json& obj) const;


    std::map<std::string, ObjectParameters> objectParameters_; // オブジェクトのパラメータを格納するベクター


};