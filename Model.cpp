#include "Model.h"

#include "DXCommon.h"

#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const std::string Model::defaultDirpath_ = "Resources/obj/";

void Model::Initialize()
{

}

void Model::Draw()
{
}

void Model::CreateFromObj(const std::string& _name, const std::string& _extension)
{
    name_ = _name+_extension;

    mesh_ = std::make_unique<Mesh>();
    mesh_->Initialize();
    mesh_->LoadFile(_name + _extension);

    material_ = std::make_unique<Material>();
    material_->Initialize(mesh_->GetTexturePath());

}

void Model::TransferData()
{
}

void Model::LoadFilejWithAssimp(const std::string& _filename)
{	

}
