
struct constantBuffer
{
    float4x4 monoUVTranform;
    float4x4 colorUVTranform;
};

cbuffer gBuffer : register(b0)
{
    constantBuffer buff;
}

Texture2D<float4> monokTexture : register(t0);
Texture2D<float4> colorTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

[numthreads(8,8,1)]
void main(uint3 id : SV_DispatchThreadID)
{
   // 出力テクスチャサイズ取得
    uint outputWidth, outputHeight;
    outputTexture.GetDimensions(outputWidth, outputHeight);

    if (id.x >= outputWidth || id.y >= outputHeight)
        return;

   // 各テクスチャのサイズ取得
    uint colorWidth, colorHeight;
    uint monoWidth, monoHeight;
    colorTexture.GetDimensions(colorWidth, colorHeight);
    monokTexture.GetDimensions(monoWidth, monoHeight);

   // UV座標計算（0.0-1.0）
    float2 uv = float2((float) id.x / (float) outputWidth,
                      (float) id.y / (float) outputHeight);

    // monoテクスチャのUV座標計算
    uint2 monoCoord = uint2(uv.x * monoWidth, uv.y * monoHeight);
    monoCoord = min(monoCoord, uint2(monoWidth - 1, monoHeight - 1));
    float4 monoColor = monokTexture[monoCoord];
    monoCoord = mul(float4(monoCoord, 0.0f, 1.0f), buff.monoUVTranform).xy;
    // monoテクスチャの輝度を計算
    float luminance = monoColor.r * 0.299f + monoColor.g * 0.587f + monoColor.b * 0.114f;

   // 輝度をそのままUにしてcolorテクスチャをサンプリング
    uint2 colorCoord = uint2(luminance * colorWidth, uv.y * colorHeight);
   // 境界チェック
    colorCoord = min(colorCoord, uint2(colorWidth - 1, colorHeight - 1));

    colorCoord = mul(float4(colorCoord, 0.0f, 1.0f), buff.colorUVTranform).xy;


   // サンプリング
    float4 colorImage = colorTexture[colorCoord];


    outputTexture[id.xy] = colorImage;
    outputTexture[id.xy].a = 1.0f;

}
