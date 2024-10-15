#pragma once
#include "PSOManager.h"
#include "Model.h"

#include <string>
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <optional>
#include <unordered_map>

class ModelManager
{
public:

    static ModelManager* GetInstance();

    void Initialize();
    void PreDraw();

    void SetBlendMode(PSOManager::BlendMode _mode) { blendMode_ = _mode; };

    Model* FindSameModel(const std::string& _name);
private:

    std::unordered_map < std::string, std::unique_ptr<Model>> models_ = {};

    ID3D12RootSignature* rootSignature_ = nullptr;
    ID3D12PipelineState* graphicsPipelineState_ = nullptr;

    PSOManager::BlendMode blendMode_ = {};


    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

};