#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <d3d12.h>
#include <wrl.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Material.h"
#include "Mesh.h"
#include <memory>
#include <map>

class Model
{
public:
	void Initialize();

	void Draw();

	void CreateFromObj(const std::string& _name,const std::string& _extension);

	static const std::string defaultDirpath_;

private:

	std::string name_ = {};

	std::map<std::string, std::unique_ptr<Mesh>>meshes_;

	void TransferData();

	void LoadFilejWithAssimp(const std::string& _filename);

};