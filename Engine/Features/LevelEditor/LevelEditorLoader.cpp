#include "LevelEditorLoader.h"

#include <numbers>

void LevelEditorLoader::Load(const std::string& _filePath)
{
    // jsonファイルを読み込み jsonオブジェクトを取得
    json j = JsonFileIO::Load(_filePath, "");

    // "name:scene"をふくんでいなければこれは不正なデータ
    if (j.contains("name"))
    {
        if (j["name"] != "scene")
            return;
    }
    else
        return;

    // "objects"
    // 読み込むものはないので返す
    if (!j.contains("objects"))
        return;

    // オブジェクトのパラメータを取得してマップに追加
    for (auto obj : j["objects"])
    {
        // オブジェクトのパラメータを取得
        ObjectParameters param = GetObjectParameterFromJson(obj); // オブジェクトのパラメータを取得

        // 名前を持っているときはに追加する
        if (!param.name.empty())
            objectParameters_[param.name] = param; // パラメータをマップに追加
    }
}

ObjectParameters LevelEditorLoader::GetObjectParameter(const std::string& _name) const
{
    auto it = objectParameters_.find(_name);
    if (it != objectParameters_.end())
    {
        return it->second; // 見つかった場合はパラメータを返す
    }
    else
    {
        return ObjectParameters(); // 見つからない場合はデフォルトのパラメータを返す
    }
}

ObjectParameters LevelEditorLoader::GetObjectParameterFromJson(const json& obj) const
{
    if (!obj.contains("name"))
        return {}; // 必要な情報がない場合はスキップ

    std::string name = obj["name"];
    // 要素を作成
    ObjectParameters param;
    param.name = name; // 名前を設定

    if (obj.contains("transform"))
    {
        auto transform = obj["transform"];
        if (transform.contains("transform"))
        {
            param.position = Vector3(transform["transform"][0], transform["transform"][1], transform["transform"][2]);
        }
        if (transform.contains("rotation"))
        {
            param.rotation = Vector3(transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]);
            // かめらのとき デフォルトの向きが違うので合わせる
            if (obj.contains("type") && obj["type"] == "CAMERA")
            {
                // blenderでは (0,0,0)のとき 下向き
                // こちらでは (0,0,0)のとき 正面なので調整する

                param.rotation.x += std::numbers::pi_v<float> / 2.0f; // カメラの向きを調整
            }
        }
        if (transform.contains("scale"))
        {
            param.scale = Vector3(transform["scale"][0], transform["scale"][1], transform["scale"][2]);
        }
    }
    if (obj.contains("collider"))
    {
        auto collisers = obj["collider"];

        for (auto collider : collisers)
        {
            ColliderPrams colliderParam;
            if (collider.contains("type"))
            {
                colliderParam.type = collider["type"];
            }
            if (collider.contains("center"))
            {
                colliderParam.center = Vector3(collider["center"][0], collider["center"][1], collider["center"][2]);
            }
            if (collider.contains("size"))
            {
                colliderParam.size = Vector3(collider["size"][0], collider["size"][1], collider["size"][2]);
            }
            param.colliders.push_back(colliderParam);
        }
    }

    if (obj.contains("file_name"))
    {
        param.modelPath = obj["file_name"].get<std::string>(); // ファイル名を設定
    }
    else
    {
        param.modelPath = "cube/cube.obj"; // デフォルトのファイル名を設定
    }

    // 子を持っているときは再帰的に取得する
    if (obj.contains("children"))
    {
        param.childParameters.push_back(GetObjectParameterFromJson(obj["children"]));
    }

    return param;
}
