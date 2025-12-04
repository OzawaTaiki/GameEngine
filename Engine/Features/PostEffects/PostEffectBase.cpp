#include "PostEffectBase.h"

#include <Core/DXCommon/DXCommon.h>

void PostEffectBase::CreateConstantBuffer(size_t dataSize)
{
    // 定数バッファのリソースを作成
    constantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(dataSize));

    // マッピングしてデータポインタを取得
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constantBufferData_));

    // 初期化
    memset(constantBufferData_, 0, dataSize);

}

void PostEffectBase::UpdateConstantBuffer(const void* data, size_t size)
{
    if (constantBufferData_ && size > 0)
    {
        memcpy(constantBufferData_, data, size);
    }
    else
    {
        // エラーハンドリング: データが無効な場合
        assert(false && "Invalid constant buffer data or size");
    }
}

void PostEffectBase::CreateConstantBufferResource(size_t dataSize, Microsoft::WRL::ComPtr<ID3D12Resource>& outResource, void** outDataPtr)
{
    // 定数バッファのリソースを作成
    outResource = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(dataSize));
    // マッピングしてデータポインタを取得
    outResource->Map(0, nullptr, outDataPtr);
    // 初期化
    memset(*outDataPtr, 0, dataSize);
}
