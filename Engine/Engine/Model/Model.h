#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/Matrix4x4.h>
#include "Material.h"
#include "Mesh.h"
#include "Animation/ModelAnimation.h"
#include "Animation/Node/Node.h"
#include "Animation/Skeleton/Skeleton.h"
#include "Animation/SkinCluster/SkinCluster.h"
#include <Light/LightGroup.h>

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
    void Update();
    void Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, ObjectColor* _color);
    void Draw(const WorldTransform& _transform, const Camera* _camera, ObjectColor* _color);

    void DrawSkeleton(const Matrix4x4& _wMat);

    void ShowImGui(const std::string& _name);

    static Model* CreateFromObj(const std::string& _filePath);

    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList) const;
    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList,uint32_t _textureHandle) const;

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = std::unique_ptr<LightGroup>(_lightGroup); }

    void SetUVTransform(const Vector2& _transform,uint32_t _index);
    void SetUVScale(const Vector2& _scale, uint32_t _index);
    void SetUVRotation(float _rotation, uint32_t _index);

    Mesh* GetMeshPtr() { return mesh_[0].get(); }
    Material* GetMaterialPtr() { return material_[0].get(); }

    Vector3 GetMin(size_t _index = -1) const;
    Vector3 GetMax(size_t _index = -1) const;

    Matrix4x4 GetAnimationMatrix()const;
    Matrix4x4 GetNodeMatrix()const { return node_.GetLocalMatrix(); }


    static const std::string defaultDirpath_;
private:

    std::string name_ = {};

    std::vector<std::unique_ptr<Mesh>> mesh_ = {};
    std::vector<std::unique_ptr<Material>> material_ = {};
    std::vector<std::unique_ptr<ModelAnimation>> animation_ = {};
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