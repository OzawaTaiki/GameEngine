#define NOMINMAX
#include "Mesh.h"
#include "DXCommon.h"
#include "Debug.h"
#include "../SkinCluster/SkinCluster.h"
#include <cassert>
#include <iterator>
#include <chrono>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

template <>
struct std::hash<Mesh::VertexData> {
    size_t operator()(const Mesh::VertexData& v) const {
        size_t h1 = std::hash<Vector4>{}(v.position);
        size_t h2 = std::hash<Vector2>{}(v.texcoord);
        size_t h3 = std::hash<Vector3>{}(v.normal);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
};

void Mesh::Initialize()
{
    dxCommon = DXCommon::GetInstance();
    vertices_.clear();
    indices_.clear();
}

void Mesh::TransferData()
{
    InitializeReources();
    std::memcpy(vConstMap_, vertices_.data(), sizeof(VertexData) * vertices_.size());
    std::memcpy(iConstMap_, indices_.data(), sizeof(uint32_t) * indices_.size());
}

void Mesh::QueueCommand(ID3D12GraphicsCommandList* _commandList) const
{
    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    _commandList->IASetIndexBuffer(&indexBufferView_);
}

void Mesh::QueueCommand(ID3D12GraphicsCommandList* _commandList, const D3D12_VERTEX_BUFFER_VIEW& _vbv) const
{
    D3D12_VERTEX_BUFFER_VIEW vbv[2] = { vertexBufferView_, _vbv };

    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 2, vbv);
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
