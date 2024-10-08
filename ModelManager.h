#pragma once

#include <wrl.h>
#include <d3d12.h>

class ModelManager
{
public:

    static ModelManager* GetInstance();

    void Initialize();


private:

    void CreatePipeLine();

    ID3D12RootSignature* rootSignature = nullptr;
    ID3D12PipelineState* graphicsPipelineState = nullptr;


    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

};