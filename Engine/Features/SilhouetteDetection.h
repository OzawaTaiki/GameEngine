#pragma once

#include <Math/Vector/Vector3.h>

#include <d3d12.h>
#include <wrl.h>

class DXCommon;
class SilhouetteDetection
{
public:
    SilhouetteDetection() = default;
    ~SilhouetteDetection();

    void Initialize(uint32_t _inputVertexSrvIndex, uint32_t _inputIndexSrvIndex, uint32_t _vertexCount, uint32_t _indexCount, const Vector3& _lightDirection = { 0,-1,0 });
    void Initialize(uint32_t _inputVertexSrvIndex, uint32_t _inputIndexSrvIndex, uint32_t _vertexCount, uint32_t _indexCount, Vector3* _lightDirection = nullptr);

    static uint32_t CreateSRVForInputIndexResource(ID3D12Resource* _resource, uint32_t _indexCount);

    void Execute();

private:

    void CreatePipeline();
    void CreateConstantBuffer();
    void CreateResources();

    DXCommon* dxCommon_;

    static bool isCreated_;

    static Microsoft::WRL::ComPtr<ID3D12PipelineState> edgeDetectionPSO_;
    static Microsoft::WRL::ComPtr<ID3D12PipelineState> silhouetteEdgePSO_;

    // 共通のルートシグネチャ
    static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    static const uint32_t kDefaultEdgeInfoCount_;
    static const uint32_t kEdgeInfoCountOffset_;
    static const uint32_t kDedaultPairEdgeCount_;
    static const uint32_t kDefaultSilhouetteEdgeCount_;


    // リソースたち
    uint32_t inputVertexSrvIndex_;
    uint32_t inputIndexSrvIndex_;

    Microsoft::WRL::ComPtr<ID3D12Resource> edgeInfoResource_= nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> counterResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> silhouetteEdgeResource_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> pairEdgeResource_ = nullptr;

    uint32_t edgeInfoSrvIndex_;
    uint32_t counterSrvIndex_;
    uint32_t silhouetteEdgeUavIndex_;
    uint32_t pairEdgeSrvIndex_;

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;


    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_ = nullptr;

    struct SilhouetteEdgeData
    {
        uint32_t vertex1;
        uint32_t vertex2;
        uint32_t isSilhouette;
        float pad;
    };

    struct EdgeInfoData
    {
        uint32_t triangleA;
        uint32_t triangleB;
        uint32_t isBoundary;
        uint32_t isProcessed;
    };

    struct PairEdgeData
    {
        uint32_t edge1;
        uint32_t edge2;
        uint32_t padding[2];
    };

    struct ConstantBufferData
    {
        Vector3 lightDirection;
        uint32_t indexCount;

        uint32_t vertexCount;
        uint32_t debug_frameCount;
        float pad[2];
    };


    ConstantBufferData* constantBufferData_;

    Vector3* lightDirection_;

};