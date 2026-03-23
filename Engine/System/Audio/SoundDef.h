#pragma once

#include <string>

namespace Engine {

/// <summary>
/// SoundEngine に登録するサウンド定義。
/// SoundData.json で id / path / type を記述し、
/// SoundEngine::LoadSoundData() で読み込む。
/// </summary>
struct SoundDef
{
    std::string id;                // サウンドID（一意なキー）
    std::string filePath;          // 音声ファイルパス
    std::string type;              // "BGM" or "SE"（submix の振り分けに使用）
    bool        enableOverlap = true; // 重複再生を許可するか（BGM は false 推奨）
};

} // namespace Engine
