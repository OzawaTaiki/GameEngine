#include "ViewProjection.h"

#include "VectorFunction.h"
#include "MatrixFunction.h"

void ViewProjection::Initilize()
{
	cBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}

void ViewProjection::UpdateMatrix()
{
	Vector3 scale = { 1.0f,1.0f ,1.0f };
	Matrix4x4 matWorld = MakeAffineMatrix(scale, rotation_, translation_);
	matView_ = Inverse(matWorld);
	matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearZ_, farZ_);

	TransferMatrix();
}

void ViewProjection::TransferMatrix()
{
	constMap_->matView = matView_;
	constMap_->matProjection = matProjection_;
	constMap_->cameraWorldPos = translation_;
}
