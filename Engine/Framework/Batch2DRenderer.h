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
        uint32_t useTextureAlpha; // 1: テキスト 0: スプライト

        int32_t padding[2];
    };





public:

    static Batch2DRenderer* GetInstance();

    void Initialize();
    void Render();

    void AddInstace(const InstanceData& _instance, const std::vector<VertexData>& _v, uint32_t _order);
    void AddInstace(const InstanceData& _instance, const std::vector<VertexData>& _v);
    // LT RT RB LB

private:

    Batch2DRenderer() = default;
    ~Batch2DRenderer() = default;

    void CreateVertexBuffer();
    void CreateInstanceDataSRV();
    void CreateViewProjectionResource();
    void Reset();

    void CreatePipelineStateObject();
    void CreateRootSignature();

    void SortData();
    void UploadData();
    void BuildDrawCommands();

private:
    struct DrawCommand
    {
        int32_t layer;
        uint32_t startInstance;
        uint32_t instanceCount;
    };

    struct SortKey
    {
        uint8_t layer; // レイヤー
        uint16_t userOrder; // ユーザー指定の順序
        uint8_t internalOrder; // 内部的な順序 (追加された順番)

        uint32_t GetSortKey() const
        {
            uint32_t key = 0;
            key |= static_cast<uint32_t>(userOrder) << 24;
            key |= static_cast<uint32_t>(internalOrder) << 8;
            key |= static_cast<uint32_t>(layer);
            return key;
        }

        bool operator<(const SortKey& other) const
        {
            return GetSortKey() < other.GetSortKey();
        }
    };

    // 描画用データ ソート用
    struct DrawData
    {
        SortKey sortKey;

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

    Microsoft::WRL::ComPtr<ID3D12Resource> viewProjectionResource_;
    Matrix4x4* viewProjectionMap_ = nullptr;


    std::vector<DrawData> drawDataList_;
    std::vector<uint32_t> sortIndices_;
    std::vector<DrawCommand> drawCommands_;
};