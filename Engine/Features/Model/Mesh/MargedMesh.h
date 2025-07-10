#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>

#include <Features/Model/Mesh/Mesh.h>

#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>
#include <functional>


class DXCommon;
class Mesh;
// 1modelの全メッシュをまとめて持つクラス
class MargedMesh
{
public:
    MargedMesh() = default;
    ~MargedMesh() = default;

    void Initialize(const std::vector<std::unique_ptr<Mesh>>& _meshes);


    void QueueCommand(ID3D12GraphicsCommandList* _commandList) const;

    size_t GetMeshCount() const { return meshCount_; }
    size_t GetVertexCount() const { return vertexCount_; }
    size_t GetIndexCount() const { return indexCount_; }

    const std::vector<size_t>& GetVertexBufferViewOffset() const { return vbvOffset_; }

    UINT GetVertexOffset(size_t _meshIndex) const;
    UINT GetIndexOffset(size_t _meshIndex) const;
    UINT GetIndexCount(size_t _meshIndex) const;

    UINT GetVertexOffset(const std::string& _meshName) const;
    UINT GetIndexOffset(const std::string& _meshName) const;
    UINT GetIndexCount(const std::string& _meshName) const;


    /// <summary>
    /// 頂点リソースを設定する
    /// </summary>
    /// <param name="_resource">SkiningCSからresoutceをも</param>
    void SetSkinnedVertexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource> _resource);

    ID3D12Resource* GetVertexResource() { return vertexResource_.Get(); }
    ID3D12Resource* GetSkinnedVertexResource() { return skinnedVertexResource_.Get(); }

    ID3D12Resource* GetIndexResource() { return indexResource_.Get(); }

    D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &vertexBufferView_; }
    D3D12_VERTEX_BUFFER_VIEW* GetSkinnedVertexBufferView() { return &skinnedVertexBufferView_; }

    D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &indexBufferView_; }

private:

    void CreateResources();
    void Map();


private:

    DXCommon* dxCommon_ = nullptr;

    // 持っているメッシュの数
    size_t meshCount_ = 0;
    size_t vertexCount_ = 0; // 全メッシュの頂点数
    size_t indexCount_ = 0; // 全メッシュのインデックス数

    std::vector<size_t> indexCountPerMesh_; // 各メッシュのインデックス数

    // メッシュの頂点データ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};// <-いらんかも？
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr; // 頂点データのリソース
    VertexData* vConstMap_ = nullptr; // 頂点データの定数バッファマップ

    // メッシュのインデックスデータ
    D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_ = nullptr; // インデックスデータのリソース
    uint32_t* iConstMap_ = nullptr; // インデックスデータの定数バッファマップ


    D3D12_VERTEX_BUFFER_VIEW skinnedVertexBufferView_ = {}; // スキンニング後の頂点データのバッファビュー
    Microsoft::WRL::ComPtr<ID3D12Resource> skinnedVertexResource_ = nullptr; // スキンニング後の頂点データのリソース

    // 各メッシュの頂点バッファとインデックスバッファのオフセット
    std::vector<size_t> vbvOffset_;
    std::vector<size_t> ibvOffset_;

    std::map<std::string, size_t> nameToIndexMap_; // メッシュ名からインデックスへのマップ
    std::map<size_t, std::string> indexToNameMap_; // インデックスからメッシュ名へのマップ
};