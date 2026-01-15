#pragma once

#include <Features/Model/Model.h>
#include <Features/Json/Loader/JsonFileIO.h>
#include <string>
#include <vector>

/// <summary>
/// ビルド時にプリミティブファイルを読み込んでModelManagerに登録するクラス
/// </summary>

namespace Engine {

class PrimitiveBuilder
{
public:
    PrimitiveBuilder() = default;
    ~PrimitiveBuilder() = default;

    /// <summary>
    /// 保存されたプリミティブファイルを全て読み込んでModelManagerに登録
    /// </summary>
    /// <param name="directory">プリミティブファイルのディレクトリ</param>
    static void BuildAndRegisterAll(const std::string& directory = "Resources/Primitives/");

    /// <summary>
    /// 単一のプリミティブファイルを読み込んでモデルを生成
    /// </summary>
    /// <param name="fileName">ファイル名（拡張子なし）</param>
    /// <param name="directory">ディレクトリパス</param>
    /// <returns>生成されたModel（失敗時はnullptr）</returns>
    static Model* BuildFromFile(const std::string& fileName, const std::string& directory = "Resources/Primitives/");

private:
    /// <summary>
    /// JSONから設定を読み込んでModelを生成
    /// </summary>
    static Model* CreateModelFromJson(const json& j);
};

} // namespace Engine
