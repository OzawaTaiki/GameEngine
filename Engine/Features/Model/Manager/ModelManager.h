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
    void PreDrawForAlphaObjectModel() const;

    ID3D12PipelineState* GetComputePipeline() const { return computePipeline_.Get(); }
    ID3D12RootSignature* GetComputeRootSignature() const { return rootSignatureForCompute_.Get(); }

    Model* FindSameModel(const std::string& _name);
    Model* Create(const std::string& _name);
public:

    void ImGui(bool* _open);

private:

    void CreateComputePipeline();

    std::unordered_map < std::string, std::unique_ptr<Model>> models_ = {};

    ID3D12RootSignature* rootSignature_ = {};
    ID3D12PipelineState* graphicsPipelineState_ = {};

    ID3D12PipelineState* graphicsPipelineStateForAlpha_ = {};

    Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipeline_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatureForCompute_ = nullptr;

    PSOFlags psoFlags_ = {};
    PSOFlags psoFlagsForAlpha_{};



private:
    ModelManager();
    ~ModelManager();
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

};
