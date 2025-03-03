#pragma once

#include <Features/Model/Animation/SkinCluster/SkinCluster.h>
#include <Features/Model/Mesh/Mesh.h>

#include <d3d12.h>
#include <wrl.h>
#include <span>

#include <cstdint>
#include <limits>

class DXCommon;
class SRVManager;

class SkinningCS
{
public:
    SkinningCS();
    ~SkinningCS();

    void Initialize();
    void Execute();

    uint32_t CreateSRVForInputVertexResource(ID3D12Resource* _resource, uint32_t _vertexNum);
    uint32_t CreateSRVForOutputVertexResource(ID3D12Resource* _resource, uint32_t _vertexNum);
    uint32_t CreateSRVForInfluenceResource(ID3D12Resource* _resource, uint32_t _vertexNum);
    uint32_t CreateSRVForMatrixPaletteResource(ID3D12Resource* _resource, uint32_t _jointNum);

    uint32_t GetInputVertexSrvIndex() const { return inputVertexSrvIndex_; }
    uint32_t GetOutputVertexUavIndex() const { return outputVertexUavIndex_; }
    uint32_t GetInfluenceSrvIndex() const { return influenceSrvIndex_; }
    uint32_t GetMatrixPaletteSrvIndex() const { return matrixPaletteSrvIndex_; }


    static Microsoft::WRL::ComPtr<ID3D12Resource> CreateOutputVertexResource(size_t _vertexSize);



private:
    void CreateInfoResource(uint32_t _vertexNum);

    void CreateComputePipeline();

    static bool isCreated_;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipeline_;
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    DXCommon* dxCommon_;
    ID3D12GraphicsCommandList* commandList_;
    SRVManager* srvManager_;

    Microsoft::WRL::ComPtr<ID3D12Resource> inputVertexResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> matrixPaletteResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> skinnedInformationResources_ = nullptr;

    uint32_t inputVertexSrvIndex_ = (std::numeric_limits<uint32_t>::max)();
    uint32_t outputVertexUavIndex_ = (std::numeric_limits<uint32_t>::max)();
    uint32_t influenceSrvIndex_ = (std::numeric_limits<uint32_t>::max)();
    uint32_t matrixPaletteSrvIndex_ = (std::numeric_limits<uint32_t>::max)();

    uint32_t* vertexNum_ = 0;
    uint32_t influenceNum_ = 0;


};