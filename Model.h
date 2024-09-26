#pragma once
// TODO : ディレクリの整理を。shaderまとめるとか

#include "Mesh.h"
#include "Material.h"

#include <string>
#include <memory>
#include <vector>

class Model
{

public:

	static Model* CreateFromObj(const std::string& _modelname, const std::string& _extension = "obj");

private:

	static const std::string defaultDirectory_;

	std::string name_;

	std::vector<std::unique_ptr<Mesh>> mesh_;
	std::vector<std::unique_ptr<Material>>material_;

	void LoadModel(const std::string& _modelname, const std::string& _extension);

	void LoadMaterial(const std::string& fileName);
};