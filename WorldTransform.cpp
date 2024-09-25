#include "WorldTransform.h"

#include "MatrixFunction.h"

void WorldTransform::Initilize()
{
	cBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}

void WorldTransform::UpdateMatrix()
{
	matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);

	if (parent_) {
		matWorld_ = matWorld_ * parent_->matWorld_;
	}
		
	constMap_->matWorld = matWorld_;
}

Vector3 WorldTransform::GetWorldPosition()
{
	Vector3 result;

	result.x = matWorld_.m[3][0];
	result.y = matWorld_.m[3][1];
	result.z = matWorld_.m[3][2];

	return result;
}
