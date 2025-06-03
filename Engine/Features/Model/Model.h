#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Model/Material/Material.h>
#include <Features/Model/Mesh/Mesh.h>
#include <Features/Model/Animation/ModelAnimation.h>
#include <Features/Model/Animation/Node/Node.h>
#include <Features/Model/Animation/SkinningCS.h>
#include <Features/Model/Animation/Skeleton/Skeleton.h>
#include <Features/Model/Animation/SkinCluster/SkinCluster.h>
#include <Features/Light/System/LightingSystem.h>

#include <vector>
#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

class Camera;
class WorldTransform;
class ObjectColor;
struct aiScene;
struct aiMesh;
class Model
{
public:
    void Initialize();
    void Update(float _deltaTime);
    void Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, ObjectColor* _color);
    void Draw(const WorldTransform& _transform, const Camera* _camera, ObjectColor* _color);

    void DrawSkeleton(const Matrix4x4& _wMat);

    static Model* CreateFromFile(const std::string& _filePath);
    static Model* CreateFromMesh(std::unique_ptr<Mesh> _mesh);
    static Model* CreateFromVertices(std::vector<VertexData> _vertices, std::vector<uint32_t> _indices, const std::string& _name);

    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList) const;
    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList,uint32_t _textureHandle) const;

    void QueueCommandForShadow(ID3D12GraphicsCommandList* _commandList) const;

    void QueueLightCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index) const;

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = std::unique_ptr<LightGroup>(_lightGroup); }
    void SetAnimation(const std::string& _name, bool _loop = false);
    void ChangeAnimation(const std::string& _name, float _blendTime, bool _loop = false);
    void StopAnimation() { currentAnimation_ = nullptr; }
    void ToIdle(float _timeToIdle);

    void LoadAnimation(const std::string& _filePath, const std::string& _name);

    UVTransform& GetUVTransform(uint32_t _index = 0) { return material_[_index]->GetUVTransform(); }

    Mesh* GetMeshPtr() { return mesh_[0].get(); }
    Material* GetMaterialPtr() { return material_[0].get(); }

    uint32_t GetVertexSrvIndex() { return skinningCS_ ? skinningCS_->GetInputVertexSrvIndex() : 0; }

    ID3D12Resource* GetIndexResource(size_t _index = -1);

    Vector3 GetMin(size_t _index = -1) const;
    Vector3 GetMax(size_t _index = -1) const;

    //Matrix4x4 GetAnimationMatrix()const;
    //Matrix4x4 GetNodeMatrix()const { return node_.GetLocalMatrix(); }
    Matrix4x4 GetSkeletonSpaceMatrix(uint32_t _index = 0)const { return skeleton_.GetSkeletonSpaceMatrix(_index); }
    bool IsAllAnimationEnd() { return currentAnimation_ == nullptr || !currentAnimation_->IsPlaying(); }
    bool IsIdle() { return currentAnimation_ == nullptr || currentAnimation_->IsIdle(); }

    bool HasAnimation() const;
    bool IsAnimationPlaying() const;

    static const std::string defaultDirpath_;
private:

    std::string name_ = {};

    std::vector<std::unique_ptr<Mesh>> mesh_ = {};
    std::vector<std::unique_ptr<Material>> material_ = {};
    std::map<std::string,std::unique_ptr<ModelAnimation>> animation_ = {};
    std::unique_ptr<ModelAnimation> currentAnimation_ = nullptr;
    ModelAnimation* preAnimation_= nullptr;
    Node node_ = {};
    Skeleton skeleton_ = {};
    SkinCluster skinCluster_ = {};

    std::unique_ptr<SkinningCS> skinningCS_ = nullptr;



    std::unique_ptr<LightGroup> lightGroup_ = nullptr;

    void LoadFile(const std::string& _filepath);
    void LoadMesh(const aiScene* _scene);
    void LoadMaterial(const aiScene* _scene);
    void LoadAnimation(const aiScene* _scene, const std::string& _filepath, const std::string& _name);
    void LoadNode(const aiScene* _scene);
    void CreateSkeleton();
    void CreateSkinCluster(const aiMesh* _mesh);


};
