#pragma once
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Model/Model.h>

#include <string>
#include <memory>
#include <wrl.h>
#include <d3d12.h>
#include <optional>
#include <unordered_map>
#include <array>

class ModelManager
{
public:

    static ModelManager* GetInstance();

    void Initialize();
    void PreDrawForObjectModel() const;
    void PreDrawForAnimationModel() const;


    Model* FindSameModel(const std::string& _name);
private:

    std::unordered_map < std::string, std::unique_ptr<Model>> models_ = {};

    std::array <ID3D12RootSignature*, 2> rootSignature_ = {};
    std::array <ID3D12PipelineState*, 2> graphicsPipelineState_ = {};

    std::array<PSOFlags, 2> psoFlags_ = {};


    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

};
