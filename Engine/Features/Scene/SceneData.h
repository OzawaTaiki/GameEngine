#pragma once

#include <string>

// シーン間でやりとりするデータ構造体
// これを継承した構造体を作成し引数で渡す
struct SceneData
{
    std::string beforeScene;

    virtual ~SceneData() = default;
};