#pragma once

#include "Matrix4x4.h"
#include <vector>
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <span>

#include <d3d12.h>
#include <wrl.h>

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

    D3D12_VERTEX_BUFFER_VIEW GetInfluenceBufferView()const  { return influenceBufferView_; }
    void QueueCommand(ID3D12GraphicsCommandList* _commandList)const;

private:
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
    std::map<std::string, JointWeightData> skinClusterData_ = {};

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

    std::vector<Matrix4x4> inverseBindPoseMatrices_;
    Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource_;
    D3D12_VERTEX_BUFFER_VIEW influenceBufferView_;
    std::span<VertexInfluenceData> mappedInfluence_;
    Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource_;
    std::span<WellForGPU> mappedPalette_;
    std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle_;
};