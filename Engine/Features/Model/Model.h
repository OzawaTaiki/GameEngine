#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Model/Material/Material.h>
#include <Features/Model/Mesh/Mesh.h>
#include <Features/Model/Mesh/MargedMesh.h>
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
    static Model* CreateFromFile(const std::string& _filePath);
    static Model* CreateFromMesh(std::unique_ptr<Mesh> _mesh);
    static Model* CreateFromVertices(std::vector<VertexData> _vertices, std::vector<uint32_t> _indices, const std::string& _name);

    void Initialize();

    void Update(float _deltaTime);

    void Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, ObjectColor* _color);
    void Draw(const WorldTransform& _transform, const Camera* _camera, ObjectColor* _color);

    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList) const;
    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList,uint32_t _textureHandle) const;

    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, MargedMesh* _margedMesh) const;
    void QueueCommandAndDraw(ID3D12GraphicsCommandList* _commandList, MargedMesh* _margedMesh, uint32_t _textureHandle) const;

    void QueueCommandForShadow(ID3D12GraphicsCommandList* _commandList) const;

    void QueueLightCommand(ID3D12GraphicsCommandList* _commandList, uint32_t _index) const;

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = std::unique_ptr<LightGroup>(_lightGroup); }

    void LoadAnimation(const std::string& _filePath, const std::string& _name);

    const Node& GetNode() const { return node_; }
    const SkinCluster& GetSkinCluster() const { return skinCluster_; }
    const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return mesh_; }
    const ModelAnimation* GetAnimation(const std::string& _name) const;


    UVTransform& GetUVTransform(uint32_t _index = 0) { return material_[_index]->GetUVTransform(); }

    Mesh* GetMeshPtr() { return mesh_[0].get(); }
    Material* GetMaterialPtr() { return material_[0].get(); }

    Vector3 GetMin(size_t _index = -1) const;
    Vector3 GetMax(size_t _index = -1) const;


    bool HasAnimation() const;

    static const std::string defaultDirpath_;
private:

    std::string name_ = {};

    std::vector<std::unique_ptr<Mesh>> mesh_ = {};
    std::vector<std::unique_ptr<Material>> material_ = {};
    std::map<std::string,std::unique_ptr<ModelAnimation>> animation_ = {};
    Node node_ = {};
    SkinCluster skinCluster_ = {};

    std::unique_ptr<LightGroup> lightGroup_ = nullptr;

    void LoadFile(const std::string& _filepath);
    void LoadMesh(const aiScene* _scene);
    void LoadMaterial(const aiScene* _scene);
    void LoadAnimation(const aiScene* _scene, const std::string& _filepath, const std::string& _name);
    void LoadNode(const aiScene* _scene);


};
