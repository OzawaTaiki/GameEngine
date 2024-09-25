#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transformation.h"

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>

class Material
{
	struct CBufferDate
	{
		Vector4 color;
		uint32_t enableLighting;
		float shiness;
		float pad[2];
		Matrix4x4 uvTransform;
	};

public:
	// カラー
	Vector4					color_						= { 1.0f,1.0f ,1.0f ,1.0f };
	// ライティングの有無
	bool					enableLighting_				= true;
	// 光沢度
	float					shiness_					= 40.0f;
	// スケール
	Vector2					scale_						= { 1.0f ,1.0f };
	// 回転
	float					rotate_						= 0.0f;
	// 移動
	Vector2					transform_					= { 0.0f ,0.0f };

	Material() = default;
	~Material() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initilize();

	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns></returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return cBuffer_; }

private:
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> cBuffer_;
	// Map用変数
	CBufferDate constMap_;
	// コピー禁止
	Material(const Material&) = delete;
	Material operator=(const Material&) = delete;

};