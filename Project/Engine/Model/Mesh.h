#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>
#include <functional>

struct VertexInfluenceData;
class DXCommon;
class Mesh
{
public:
    void Initialize();
    //void LoadFile(const std::string& _filepath,  const std::string& _directoryPath="Resources/models/");

    void TransferData();
    void QueueCommand(ID3D12GraphicsCommandList* _commandList)const;
    void QueueCommand(ID3D12GraphicsCommandList* _commandList, const D3D12_VERTEX_BUFFER_VIEW& _vbv)const;

    uint32_t GetIndexNum() const { return static_cast<uint32_t>(indices_.size()); }
    uint32_t GetVertexNum() const { return static_cast<uint32_t>(vertices_.size()); }

    D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &vertexBufferView_; }
    D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &indexBufferView_; }

    uint32_t GetUseMaterialIndex() const { return useMaterialIndex_; }
    void SetUseMaterialIndex(uint32_t _index) { useMaterialIndex_ = _index; }

    struct VertexData
    {
        Vector4 position;
        Vector2 texcoord;
        Vector3 normal;

        bool operator==(const VertexData& _obj) const {
            return position == _obj.position && texcoord == _obj.texcoord && normal == _obj.normal;
        }
    };
    std::vector<VertexData>                     vertices_ = {};                   // データ格納用
    std::vector<uint32_t>                       indices_ = {};                   // データ格納用
private:

    DXCommon* dxCommon = nullptr;

    std::string                                 name_                   = {};

    VertexData*                                 vConstMap_              = nullptr;              // map用
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResource_        = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferView_        = {};

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