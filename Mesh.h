#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>

class DXCommon;

class Mesh
{
public:

    void Initialize();
    void LoadFile(const std::string& _filepath,  const std::string& _directoryPath="Resources/Obj/");

    void TransferData();

    std::string GetTexturePath() { return textureHandlePath_; }

private:
    DXCommon* dxCommon = nullptr;

    std::string                                 name_                   = {};
    std::string                                 textureHandlePath_      = {};

    struct VertexData
    {
        Vector4 position;
        Vector3 normal;
        Vector2 texcoord;

        bool operator==(const VertexData& _obj) const {
            return position == _obj.position && texcoord == _obj.texcoord && normal == _obj.normal;
        }
    };

    std::vector<VertexData>                     vertices_               = {};                   // データ格納用
    VertexData*                                 vConstMap_              = nullptr;              // map用
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResource_        = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferView        = {};

    std::vector<uint32_t>                       indices_                = {};                   // データ格納用
    uint32_t*                                   iConstMap_              = nullptr;              // map用
    Microsoft::WRL::ComPtr<ID3D12Resource>      indexResource_         = nullptr;
    D3D12_INDEX_BUFFER_VIEW                     indexBufferView         = {};

    void InitializeReources();
    void CreateResources();
    void Map();

};

