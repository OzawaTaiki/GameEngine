#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Material.h"
#include "Mesh.h"

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

	void Draw(WorldTransform* _transform, Camera* _camera, uint32_t _textureHandle, ObjectColor* _color);

	static Model* CreateFromObj(const std::string& _filePath);

	static const std::string defaultDirpath_;

private:

	std::string name_ = {};

	std::unique_ptr<Mesh>				mesh_			= nullptr;
	std::unique_ptr<Material>			material_		= nullptr;

	void LoadMesh(const std::string& _filePath);
	void LoadMaterial(const std::string& _filePath);

	void TransferData();

};