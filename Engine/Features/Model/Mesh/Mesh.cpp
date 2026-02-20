#include <Features/Model/Mesh/Mesh.h>
#include <Core/DXCommon/DXCommon.h>
#include <Debug/Debug.h>
#include <Debug/Debug.h>
#include <cassert>
#include <iterator>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std::hashの特殊化はnamespace Engineの外に配置
template <>
struct std::hash<Engine::VertexData> {
    size_t operator()(const Engine::VertexData& v) const {
        size_t h1 = std::hash<Engine::Vector4>{}(v.position);
        size_t h2 = std::hash<Engine::Vector2>{}(v.texcoord);
        size_t h3 = std::hash<Engine::Vector3>{}(v.normal);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
};

namespace Engine {

void Mesh::Initialize(const std::vector<VertexData>& _v, const std::vector<uint32_t>& _i, const std::string& _name)
{
    dxCommon = DXCommon::GetInstance();
    name_ = _name;
    vertices_ = _v;
    indices_ = _i;
    InitializeReources();
    TransferData();

}

void Mesh::TransferData()
{
    std::memcpy(vConstMap_, vertices_.data(), sizeof(VertexData) * vertices_.size());
    std::memcpy(iConstMap_, indices_.data(), sizeof(uint32_t) * indices_.size());

}

void Mesh::QueueCommand(ID3D12GraphicsCommandList* _commandList) const
{
    //*vOut_ = *vConstMap_;

    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    _commandList->IASetIndexBuffer(&indexBufferView_);
}


void Mesh::InitializeReources()
{
    CreateResources();
    Map();

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<UINT> (sizeof(uint32_t) * indices_.size());
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

}

void Mesh::CreateResources()
{
    vertexResource_ = dxCommon->CreateBufferResource(static_cast<uint32_t>(sizeof(VertexData) * vertices_.size()));
    indexResource_ = dxCommon->CreateBufferResource(static_cast<uint32_t>(sizeof(uint32_t) * indices_.size()));
}

void Mesh::Map()
{
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMap_));
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&iConstMap_));
};

} // namespace Engine
