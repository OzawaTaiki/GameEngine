#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Material.h"
#include "Mesh.h"
#include "LightGroup.h"

#include <vector>
#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

class Camera;
class WorldTransform;
class ObjectColor;

class Model
{
public:
    void Initialize();

    void Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, ObjectColor* _color);
    void Draw(const WorldTransform& _transform, const Camera* _camera, ObjectColor* _color);
    void Draw(const WorldTransform& _transform, const Camera* _camera, const Vector4& _color = { 1,1,1,1 });
    void Draw(const WorldTransform& _transform, const Camera* _camera, uint32_t _textureHandle, const Vector4& _color = { 1,1,1,1 });

    void ShowImGui(const std::string& _name);

    static Model* CreateFromObj(const std::string& _filePath);

    void SetUVRotate(float _rot);
    void SetUVScale(const Vector2& _scale);
    void SetUVTrans(const Vector2& _trans);

    Mesh* GetMeshPtr() { return mesh_.get(); }
    Material* GetMaterialPtr() { return material_.get(); }

    static const std::string defaultDirpath_;

    void SetLightGroup(LightGroup* _lightGroup) { lightGroup_ = _lightGroup; }

    ~Model();
private:

    std::string name_ = {};

    std::unique_ptr<Mesh>				mesh_			= nullptr;
    std::unique_ptr<Material>			material_		= nullptr;

    LightGroup*					        lightGroup_		= nullptr;
    ObjectColor*                        color_          = nullptr;

    void LoadMesh(const std::string& _filePath);
    void LoadMaterial(const std::string& _filePath);

    void TransferData();

};