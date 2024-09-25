#include "Material.h"
#include "MatrixFunction.h"


void Material::Initilize()
{
	cBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
	
	constMap_->color				= color_;
	constMap_->enableLighting		= static_cast<uint32_t>(enableLighting_);
	constMap_->shiness				= shiness_;
	constMap_->uvTransform			= MakeIdentity4x4();
}

void Material::LoadTexture(const std::string& _dirPath)
{
	textureFileName_ = _dirPath;
}
