#include "MargedMesh.h"

#include <Core/DXCommon/DXCommon.h>

void MargedMesh::Initialize(const std::vector<std::unique_ptr<Mesh>>& _meshes)
{
    dxCommon_ = DXCommon::GetInstance();
    meshCount_ = _meshes.size();

    if (meshCount_ == 0) {
        return;
    }

    vertexCount_ = 0;
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;

    uint32_t currentVertexOffset = 0;
    uint32_t currentIndexOffset = 0;

    size_t currentMeshCount = 0;

    // 最初のメッシュの頂点データを設定
    for (const auto& mesh : _meshes)
    {
        if (!mesh) {
            continue;
        }

        indexToNameMap_[currentMeshCount] = mesh->GetName();
        nameToIndexMap_[mesh->GetName()] = currentMeshCount;

        vbvOffset_.push_back(currentVertexOffset);
        ibvOffset_.push_back(currentIndexOffset);

        for (auto& v : mesh->GetVertices())
        {
            vertices.push_back(v);
        }
        for (uint32_t index : mesh->GetIndices())
        {
            indices.push_back(index);
        }

        currentVertexOffset += mesh->GetVertexNum();
        currentIndexOffset += mesh->GetIndexNum();

        ++currentMeshCount;

        indexCountPerMesh_.push_back(mesh->GetIndexNum());
    }

    vertexCount_ = vertices.size();
    indexCount_ = indices.size();

    CreateResources();
    Map();

    std::memcpy(vConstMap_, vertices.data(), sizeof(VertexData) * vertexCount_);
    std::memcpy(iConstMap_, indices.data(), sizeof(uint32_t) * indexCount_);

    skinnedVertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    skinnedVertexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(VertexData) * vertexCount_);
    skinnedVertexBufferView_.StrideInBytes = sizeof(VertexData);

    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(uint32_t) * indexCount_);
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

}

void MargedMesh::QueueCommand(ID3D12GraphicsCommandList* _commandList) const
{
    _commandList->IASetVertexBuffers(0, 1, &skinnedVertexBufferView_);
    _commandList->IASetIndexBuffer(&indexBufferView_);
}

UINT MargedMesh::GetVertexOffset(size_t _meshIndex) const
{
    if (_meshIndex >= vbvOffset_.size())
    {
        return 0;
    }

    return static_cast<UINT>(vbvOffset_[_meshIndex]);
}

UINT MargedMesh::GetIndexOffset(size_t _meshIndex) const
{
    if (_meshIndex >= ibvOffset_.size())
    {
        return 0;
    }

    return static_cast<UINT>(ibvOffset_[_meshIndex]);
}

UINT MargedMesh::GetIndexCount(size_t _meshIndex) const
{
    if (_meshIndex >= indexCountPerMesh_.size())
    {
        return 0;
    }

    return static_cast<UINT>(indexCountPerMesh_[_meshIndex]);
}

UINT MargedMesh::GetVertexOffset(const std::string& _meshName) const
{
    if (nameToIndexMap_.contains(_meshName))
    {
        return GetVertexOffset(nameToIndexMap_.at(_meshName));
    }
    return 0;
}

UINT MargedMesh::GetIndexOffset(const std::string& _meshName) const
{
    if (nameToIndexMap_.contains(_meshName))
    {
        return GetIndexOffset(nameToIndexMap_.at(_meshName));
    }
    return 0;
}

UINT MargedMesh::GetIndexCount(const std::string& _meshName) const
{
    if (nameToIndexMap_.contains(_meshName))
    {
        return GetIndexCount(nameToIndexMap_.at(_meshName));
    }
    return 0;
}

void MargedMesh::SetSkinnedVertexBufferView(Microsoft::WRL::ComPtr<ID3D12Resource> _resource)
{
    skinnedVertexResource_ = _resource;

    skinnedVertexBufferView_.BufferLocation = skinnedVertexResource_->GetGPUVirtualAddress();
}

void MargedMesh::CreateResources()
{
    vertexResource_ = dxCommon_->CreateBufferResource(static_cast<uint32_t>(sizeof(VertexData) * vertexCount_));
    indexResource_ = dxCommon_->CreateBufferResource(static_cast<uint32_t>(sizeof(uint32_t) * indexCount_));

}

void MargedMesh::Map()
{
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMap_));
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&iConstMap_));

}
