#include "LevelEditorLoader.h"

void LevelEditorLoader::Load(const std::string& _filePath)
{

    json j = JsonLoader::LoadFile(_filePath);

    // "name:scene"をふくんでいなければこれは不正なデータ
    if (j.contains("name"))
    {
        if (j["name"] != "scene")
            return;
    }
    else
        return;

    // "objects"を読み込むものはないので返す
    if (!j.contains("objects"))
        return;

    for (auto obj : j["objects"])
    {
        if (!obj.contains("name"))
            continue; // 必要な情報がない場合はスキップ

        std::string name = obj["name"];
        // 要素を作成
        ObjectParameters& param = objectParameters_[name];
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

        if (obj.contains("children"))
        {
            auto children = obj["children"];
            param.hasChild = true; // 子オブジェクトを持つことを示す

            for (auto child : children)
            {
                if (!child.contains("name"))
                    continue; // 必要な情報がない場合はスキップ

                ObjectParameters childParam;

                auto transform = child["transform"];
                childParam.position = Vector3(transform["transform"][0], transform["transform"][1], transform["transform"][2]);
                childParam.rotation = Vector3(transform["rotation"][0], transform["rotation"][1], transform["rotation"][2]);
                childParam.scale = Vector3(transform["scale"][0], transform["scale"][1], transform["scale"][2]);


                if (child.contains("collider"))
                {
                    auto collider = child["collider"];

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

                param.childParameters.push_back(childParam);
            }
        }

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
