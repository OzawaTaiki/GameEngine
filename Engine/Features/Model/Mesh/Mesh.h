#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>

#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>
#include <functional>


struct VertexData
{
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;

    bool operator==(const VertexData& _obj) const {
        return position == _obj.position && texcoord == _obj.texcoord && normal == _obj.normal;
    }
};

struct VertexInfluenceData;
class DXCommon;
class Mesh
{
public:


    void Initialize(const std::vector<VertexData>& _v, const std::vector<uint32_t>& _i);
    void SetOutputVertexResource(Microsoft::WRL::ComPtr<ID3D12Resource> _resource);
    //void LoadFile(const std::string& _filepath,  const std::string& _directoryPath="Resources/models/");

    void TransferData();
    void QueueCommand(ID3D12GraphicsCommandList* _commandList) const;
    void QueueCommand(ID3D12GraphicsCommandList* _commandList, const D3D12_VERTEX_BUFFER_VIEW& _vbv)const;

    uint32_t GetIndexNum() const { return static_cast<uint32_t>(indices_.size()); }
    uint32_t GetVertexNum() const { return static_cast<uint32_t>(vertices_.size()); }

    D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &vertexBufferView_; }
    D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &indexBufferView_; }

    VertexData* GetVertexData() { return vConstMap_; }

    ID3D12Resource* GetVertexResource() { return vertexResource_.Get(); }
    ID3D12Resource* GetOutputVertexResource() { return outPutVertexResource_.Get(); }
    ID3D12Resource* GetIndexResource() { return indexResource_.Get(); }


    uint32_t GetUseMaterialIndex() const { return useMaterialIndex_; }
    void SetUseMaterialIndex(uint32_t _index) { useMaterialIndex_ = _index; }

    Vector3 GetMin() const { return min; }
    Vector3 GetMax() const { return max; }

    void SetMin(const Vector3& _min) { min = _min; }
    void SetMax(const Vector3& _max) { max = _max; }

    std::vector<VertexData>                     vertices_ = {};                   // データ格納用
    std::vector<uint32_t>                       indices_ = {};                   // データ格納用
private:

    DXCommon* dxCommon = nullptr;

    std::string                                 name_                   = {};

    VertexData*                                 vConstMap_              = nullptr;              // map用
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResource_         = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      outPutVertexResource_   = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferView_       = {};


    uint32_t*                                   iConstMap_              = nullptr;              // map用
    Microsoft::WRL::ComPtr<ID3D12Resource>      indexResource_         = nullptr;
    D3D12_INDEX_BUFFER_VIEW                     indexBufferView_         = {};

    Vector3 min;
    Vector3 max;

    uint32_t useMaterialIndex_ = 0;

    void InitializeReources();
    void CreateResources();
    void Map();

};
