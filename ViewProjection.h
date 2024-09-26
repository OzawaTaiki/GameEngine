#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <wrl.h>
#include <d3d12.h>

class ViewProjection
{

	struct ViewProjectioncBuffer
	{
		Matrix4x4 matView;
		Matrix4x4 matProjection;
		Vector3 cameraWorldPos;
	};

public:

	Vector3					rotation_				= { 0.0f,0.0f ,0.0f };		// 回転角
	Vector3					translation_			= { 0.0f,0.0f ,-50.0f };	// ローカル座標

	float					fovY_					= 0.45f;					// 
	float					aspectRatio_			= 16.0f / 9.0f;				// 
	float					nearZ_					= 0.1f;						// 
	float					farZ_					= 1000.0f;					// 

	Matrix4x4				matView_;											// ビュー行列
	Matrix4x4				matProjection_;										// 射影行列

	void Initilize();
	void UpdateMatrix();
	void TransferMatrix();

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return cBuffer_; }

private:
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> cBuffer_;
	// マップ用
	ViewProjectioncBuffer* constMap_;

	ViewProjection(const ViewProjection&) = delete;
	ViewProjection operator(const ViewProjection&) = delete;

};