#pragma once

#include <Physics/Math/Vector2.h>
#include <Physics/Math/Vector3.h>
#include <Physics/Math/Vector4.h>
#include <Physics/Math/Matrix4x4.h>
#include "Material.h"
#include "Mesh.h"
#include <Rendering/Model/Animation/ModelAnimation.h>
#include <Rendering/Model/Animation/Node/Node.h>
#include <Rendering/Model/Animation/Skeleton/Skeleton.h>
#include <Rendering/Model/Animation/SkinCluster/SkinCluster.h>
#include <Rendering/Light/LightingSystem.h>

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

    void ShowImGui(const std::string& _name);

    static Model* CreateFromObj(const std::string& _filePath);

    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, bool _animation = false) const;
    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList,uint32_t _textureHandle, bool _animation = false) const;

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = std::unique_ptr<LightGroup>(_lightGroup); }
    void SetAnimation(const std::string& _name, bool _loop = false);
    void StopAnimation() { currentAnimation_ = nullptr; }
    void ToIdle(float _timeToIdle);

    void SetUVTransform(const Vector2& _transform,uint32_t _index);
    void SetUVScale(const Vector2& _scale, uint32_t _index);
    void SetUVRotation(float _rotation, uint32_t _index);

    Mesh* GetMeshPtr() { return mesh_[0].get(); }
    Material* GetMaterialPtr() { return material_[0].get(); }

    Vector3 GetMin(size_t _index = -1) const;
    Vector3 GetMax(size_t _index = -1) const;

    //Matrix4x4 GetAnimationMatrix()const;
    //Matrix4x4 GetNodeMatrix()const { return node_.GetLocalMatrix(); }
    Matrix4x4 GetSkeletonSpaceMatrix(uint32_t _index = 0)const { return skeleton_.GetSkeletonSpaceMatrix(_index); }
    bool IsAllAnimationEnd() { return currentAnimation_ == nullptr || !currentAnimation_->IsPlaying(); }

    static const std::string defaultDirpath_;
private:

    std::string name_ = {};

    std::vector<std::unique_ptr<Mesh>> mesh_ = {};
    std::vector<std::unique_ptr<Material>> material_ = {};
    std::map<std::string,std::unique_ptr<ModelAnimation>> animation_ = {};
    ModelAnimation* currentAnimation_ = nullptr;
    ModelAnimation* preAnimation_= nullptr;
    Node node_ = {};
    Skeleton skeleton_ = {};
    SkinCluster skinCluster_ = {};


    std::unique_ptr<LightGroup> lightGroup_ = nullptr;

    void LoadFile(const std::string& _filepath);
    void LoadMesh(const aiScene* _scene);
    void LoadMaterial(const aiScene* _scene);
    void LoadAnimation(const aiScene* _scene);
    void LoadNode(const aiScene* _scene);
    void CreateSkeleton();
    void CreateSkinCluster(const aiMesh* _mesh);

    void TransferData();

};
