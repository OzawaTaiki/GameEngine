#pragma once
#include "Sprite.h"

#include <Render/PSOManager.h>
#include <Math/Vector2.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <optional>

#include <wrl.h>
#include <d3d12.h>

class SpriteManager
{
public:
    static SpriteManager* GetInstance();

    void Initialize();
    void PreDraw();
    Sprite* Create();

private:

    /// mesh 四角形用


    std::list<std::unique_ptr<Sprite>> sprites_ = {};

    ID3D12RootSignature* rootSignature_ = nullptr;
    ID3D12PipelineState* graphicsPipelineState_ = nullptr;

    PSOManager::BlendMode blendMode_ = {};


    SpriteManager() = default;
    ~SpriteManager() = default;
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;


};