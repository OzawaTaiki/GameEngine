#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <d3d12.h>
#include <wrl.h>

class Batch2DRenderer
{
public:
    struct VertexData
    {
        Vector4 position;
        Vector2 texCoord;
        Vector4 color;
    };

    struct InstanceData
    {
        Matrix4x4 transform;
        Matrix4x4 uvTransform;
        Vector4 color;

        uint32_t textureIndex;
        uint32_t useTextureAlpha; // 0: テキスト 1: スプライト
        uint32_t instanceBaseIndex;

        int32_t padding;
    };

    struct DrawCommand
    {
        int32_t layer;
        uint32_t startInstance;
        uint32_t instanceCount;
    };


public:

    static Batch2DRenderer* GetInstance();

    void Initialize();
    void Render();

    void AddInstace(const InstanceData& _instance, const std::vector<VertexData>& _v);

private:

    Batch2DRenderer() = default;
    ~Batch2DRenderer() = default;

    void CreateVertexBuffer();
    void CreateInstanceDataSRV();
    void Reset();

    void CreatePipelineStateObject();
    void CreateRootSignature();

    void SortData();
    void UploadData();
    void BuildDrawCommands();

private:

    // 描画用データ ソート用
    struct DrawData
    {
        uint32_t layer;
        uint32_t order;

        std::vector<VertexData> vertices; // 2D四角形を描画するための6頂点分のデータ
        InstanceData instance;
    };

private:

    const uint32_t kMaxInstanceCount_ = 1 << 10;// 1024

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;


    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexMap_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
    InstanceData* instanceMap_ = nullptr;
    uint32_t instanceDataSRVIndex_ = 0;

    std::vector<DrawData> drawDataList_;
    std::vector<uint32_t> sortIndices_;
    std::vector<DrawCommand> drawCommands_;
};