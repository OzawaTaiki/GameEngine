#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <d3d12.h>
#include <wrl.h>

class WorldTransform
{
	struct WorldTransformCBuffer
	{
		Matrix4x4 matWorld;
	};

public:
	Vector3					scale_				= { 1.0f,1.0f ,1.0f };		// スケール
	Vector3					rotation_			= { 0.0f,0.0f ,0.0f };		// 回転
	Vector3					translation_		= { 0.0f,0.0f ,0.0f };		// 平行移動

	Matrix4x4				matWorld_;

	const WorldTransform*	parent_				= nullptr;					// 親

	WorldTransform()	= default;
	~WorldTransform()	= default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initilize();

	/// <summary>
	/// 行列の更新
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// 親のポインタを登録
	/// </summary>
	/// <param name="_parent">親のポインタ</param>
	void SetParent(WorldTransform* _parent) { parent_ = _parent; }

	/// <summary>
	/// ワールド座標を取得
	/// </summary>
	/// <returns>ワールド座標</returns>
	Vector3 GetWorldPosition();

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return cBuffer_; }


private:
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> cBuffer_;
	// マップ用
	WorldTransformCBuffer* constMap_;

	WorldTransform(const WorldTransform&) = delete;
	WorldTransform operator=(const WorldTransform&) = delete;
};