#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <wrl.h>
#include <d3d12.h>

#include <cstdint>
#include <string>
#include <vector>

class Material;

class Mesh
{
	struct VertexData
	{
		Vector4 position;
		Vector2 uv;
		Vector3 normal;
	};

public:
	Mesh() = default;
	~Mesh() = default;


	Material* GetMaterial() const { return material_; }
	const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return vertexBufferView_; }
	const D3D12_INDEX_BUFFER_VIEW& GetIBView() const { return indexBufferView_; }


private:

	std::string									name_ = {};					// 名前

	Microsoft::WRL::ComPtr<ID3D12Resource>		vertexResource_;			// インデックスバッファ
	D3D12_VERTEX_BUFFER_VIEW					vertexBufferView_;			// 頂点バッファビュー
	std::vector<VertexData>						verties_;					// 頂点データ配列

	Microsoft::WRL::ComPtr<ID3D12Resource>		indexResource_;				// 頂点バッファ
	D3D12_INDEX_BUFFER_VIEW						indexBufferView_;			// インデックスバッファビュー
	std::vector<uint32_t>						indices_;					// 頂点インデックス配列

	Material*									material_ = nullptr;		// マテリアル


	Mesh(const Mesh&) = delete;
	Mesh operator=(const Mesh&) = delete;

};