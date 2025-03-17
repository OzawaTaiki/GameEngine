#pragma once

#include <Math/Matrix/Matrix4x4.h>
#include <vector>
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <span>

#include <d3d12.h>
#include <wrl.h>



struct VertexWeightData
{
    float Weight;
    uint32_t vertexIndex;
};
struct JointWeightData
{
    Matrix4x4 inverseBindPoseMatrix;
    std::vector<VertexWeightData> vertexWeights;
};

static const uint32_t influenceNum_ = 4;
struct VertexInfluenceData
{
    std::array<float, influenceNum_> weights;
    std::array<uint32_t, influenceNum_> jointIndices;
};
struct WellForGPU
{
    Matrix4x4 skeletonSpaceMatrix;
    Matrix4x4 skeletonSpaceInverseTransposeMatrix;
};

class Joint;
struct aiBone;
class SkinCluster
{

public:
    SkinCluster() = default;
    ~SkinCluster() = default;

    void CreateResources(uint32_t _jointsSize, uint32_t _vertexSize, const std::map<std::string, int32_t>& _jointMap);
    void Update(std::vector<Joint>& _joints);
    void Draw();

    void CreateSkinCluster(aiBone* _bone);

    VertexInfluenceData* GetMappedInfluence() { return mappedInfluence_.data(); }
    WellForGPU* GetMappedPalette() { return mappedPalette_.data(); }

    ID3D12Resource* GetInfluenceResource() { return influenceResource_.Get(); }
    ID3D12Resource* GetPaletteResource() { return paletteResource_.Get(); }

    D3D12_VERTEX_BUFFER_VIEW GetInfluenceBufferView()const  { return influenceBufferView_; }
    void QueueCommand(ID3D12GraphicsCommandList* _commandList)const;

private:
    std::map<std::string, JointWeightData> skinClusterData_ = {};

    std::vector<Matrix4x4> inverseBindPoseMatrices_;
    D3D12_VERTEX_BUFFER_VIEW influenceBufferView_;
    std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle_;

    Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource_;
    std::span<VertexInfluenceData> mappedInfluence_;

    Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource_;
    std::span<WellForGPU> mappedPalette_;
};
