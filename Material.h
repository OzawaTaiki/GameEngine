#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transformation.h"

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>
#include <string>

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
	Vector4					color_						= { 1.0f,1.0f ,1.0f ,1.0f };	// カラー
	bool					enableLighting_				= true;							// ライティングの有無
	float					shiness_					= 40.0f;						// 光沢度
	Vector2					scale_						= { 1.0f ,1.0f };				// スケール
	float					rotate_						= 0.0f;							// 回転	
	Vector2					transform_					= { 0.0f ,0.0f };				// 移動
	std::string				textureFileName_			= {};							// テクスチャファイル名

	Material() = default;
	~Material() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initilize();

	/// <summary>
	/// テクスチャの読み込み 
	/// </summary>
	/// <param name="_dirPath"></param>
	void LoadTexture(const std::string& _dirPath);

	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns></returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return cBuffer_; }

	/// <summary>
	/// テクスチャハンドルの取得
	/// </summary>
	/// <returns></returns>
	const int32_t GetTextureHandle() const { return textureHandle_; }

private:
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> cBuffer_;
	// Map用変数
	CBufferDate* constMap_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_;
	// コピー禁止
	Material(const Material&) = delete;
	Material operator=(const Material&) = delete;

};