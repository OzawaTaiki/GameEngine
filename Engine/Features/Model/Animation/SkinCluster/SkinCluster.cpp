#include <Features/Model/Animation/SkinCluster/SkinCluster.h>
#include <Features/Model/Animation/Joint/Joint.h>
#include <Core/DXCommon/DXCommon.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Math/Matrix/MatrixFunction.h>
#include <assimp/scene.h>
#include <algorithm>
#include <cassert>

#include <Debug/Debug.h>

void SkinCluster::CreateResources(uint32_t _jointsSize, uint32_t _vertexSize, const std::map<std::string, int32_t>& _jointMap)
{
    paletteResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(WellForGPU) * _jointsSize);
    WellForGPU* mappedPalette = nullptr;
    paletteResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    mappedPalette_ = { mappedPalette, _jointsSize};
    uint32_t index = SRVManager::GetInstance()->Allocate();
    paletteSrvHandle_.first = SRVManager::GetInstance()->GetCPUSRVDescriptorHandle(index);
    paletteSrvHandle_.second = SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(index);

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(index,paletteResource_.Get(), _jointsSize, sizeof(WellForGPU));

    influenceResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(VertexInfluenceData) * _vertexSize);
    VertexInfluenceData* mappedInfluence = nullptr;
    influenceResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    std::memset(mappedInfluence, 0, sizeof(VertexInfluenceData) * _vertexSize);
    mappedInfluence_ = { mappedInfluence, _vertexSize };

    influenceBufferView_.BufferLocation = influenceResource_->GetGPUVirtualAddress();
    influenceBufferView_.SizeInBytes = UINT(sizeof(VertexInfluenceData) * _vertexSize);
    influenceBufferView_.StrideInBytes = sizeof(VertexInfluenceData);

    inverseBindPoseMatrices_.resize(_jointsSize);
    std::generate(inverseBindPoseMatrices_.begin(), inverseBindPoseMatrices_.end(), MakeIdentity4x4);

    for (const auto& jointweight : skinClusterData_)
    {
        auto it = _jointMap.find(jointweight.first);
        if (it == _jointMap.end())
        {
            continue;
        }
        inverseBindPoseMatrices_[it->second] = jointweight.second.inverseBindPoseMatrix;
        int i = 0;
        for (const auto& vertexWeight : jointweight.second.vertexWeights)
        {
            VertexInfluenceData& influenceData = mappedInfluence_[vertexWeight.vertexIndex];
            for (uint32_t i = 0; i < influenceNum_; ++i)
            {
                if (influenceData.weights[i] == 0.0f)
                {
                    influenceData.weights[i] = vertexWeight.Weight;
                    influenceData.jointIndices[i] = it->second;
                    break;
                }
            }
        }
    }
}

void SkinCluster::Update(std::vector<Joint>& _joints)
{
    for (size_t index = 0; index < _joints.size(); ++index)
    {
        assert(index < inverseBindPoseMatrices_.size());
        mappedPalette_[index].skeletonSpaceMatrix = inverseBindPoseMatrices_[index] * (*_joints[index].GetSkeletonSpaceMatrix());
        mappedPalette_[index].skeletonSpaceInverseTransposeMatrix = Transpose(Inverse(mappedPalette_[index].skeletonSpaceMatrix));
    }
}

void SkinCluster::Draw()
{
}

void SkinCluster::CreateSkinCluster(aiBone* _bone,uint32_t _meshoffset)
{
    std::string jointName = _bone->mName.C_Str();
    JointWeightData& jointWeightData = skinClusterData_[jointName];

    aiMatrix4x4 bindPoseMatrixAssimp = _bone->mOffsetMatrix.Inverse();
    aiVector3D scale, translate;
    aiQuaternion rotate;
    bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
    Matrix4x4 bindPoseMatrix = MakeAffineMatrix({ scale.x,scale.y,scale.z }, { rotate.x,-rotate.y,-rotate.z,rotate.w }, { -translate.x,translate.y,translate.z });
    jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

    for (uint32_t weightIndex = 0; weightIndex < _bone->mNumWeights; ++weightIndex)
    {
        jointWeightData.vertexWeights.push_back({ _bone->mWeights[weightIndex].mWeight,_bone->mWeights[weightIndex].mVertexId + _meshoffset });
    }
}

void SkinCluster::QueueCommand(ID3D12GraphicsCommandList* _commandList) const
{
    _commandList->SetGraphicsRootDescriptorTable(6, paletteSrvHandle_.second);
}

