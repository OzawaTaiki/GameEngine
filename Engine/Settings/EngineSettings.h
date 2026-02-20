#pragma once

#include <cstdint>
#include <string>

/// <summary>
/// エンジン設定データ構造体。
/// </summary>

namespace Engine {

struct EngineConfig
{
    std::wstring windowTitle = L"GameEngine"; // ウィンドウタイトル
    uint32_t windowWidth = 1280;              // ウィンドウ幅
    uint32_t windowHeight = 720;              // ウィンドウ高さ
    bool isFullscreen = false;                // フルスクリーンモード
    bool enableVSync = true;                  // VSync有効化
    uint32_t targetFPS = 60;                  // 目標FPS
};

/// <summary>
/// エンジン設定管理クラス。
/// </summary>
class EngineSettings
{
public:
    static EngineConfig current_;   // 現在の設定
    static EngineConfig default_;   // デフォルト設定

    /// <summary>
    /// 設定をファイルから読み込む。
    /// </summary>
    /// <param name="filePath">読み込むファイルパス（デフォルト値あり）</param>
    static void Load(const std::string& filePath = "Resources/Engine/engine_config.json");

    /// <summary>
    /// 設定をファイルへ保存する。
    /// </summary>
    /// <param name="filePath">保存先ファイルパス（デフォルト値あり）</param>
    static void Save(const std::string& filePath = "Resources/Engine/engine_config.json");
};

} // namespace Engine
