// 入力テクスチャ
Texture2D<float4> gInputTexture : register(t0);
// 出力テクスチャ
RWTexture2D<float4> gOutputTexture : register(u0);

cbuffer KernelSizeBuffer : register(b0)
{
    float4 vignetteColor; // ビネットカラー
    float scale; // スケール（強度）
    float power; // パワー（減衰カーブ）
    float2 pad; // パディング
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // テクスチャサイズ取得
    uint width, height;
    gInputTexture.GetDimensions(width, height);

    // 範囲外チェック
    if (id.x >= width || id.y >= height)
        return;

    // 入力色を取得
    float4 inputColor = gInputTexture[id.xy];

    // 正規化座標を計算（0.0 ～ 1.0）
    float2 uv = float2(id.xy) / float2(width - 1, height - 1);

    // 中心からの距離を計算（0.0 = 中心、1.0 = 角）
    float2 centerOffset = uv - 0.5f;
    float distanceFromCenter = length(centerOffset) * 2.0f; // 対角線で正規化

    // ビネット係数を計算
    float vignette = 1.0f - saturate(distanceFromCenter * scale);
    vignette = pow(vignette, power);

    // 色にビネット効果を適用
    float4 outputColor = inputColor;
    outputColor.rgb = lerp(vignetteColor.rgb, inputColor.rgb, vignette);

    outputColor.a = 1.0f;
    // 出力
    gOutputTexture[id.xy] = outputColor;
}