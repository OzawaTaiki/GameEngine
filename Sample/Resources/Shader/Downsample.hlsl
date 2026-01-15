cbuffer DownsampleParams : register(b0)
{
    float2 texelSize; // 入力テクスチャの1ピクセルサイズ (1.0/width, 1.0/height)
    float2 pad;
};

Texture2D<float4> inputTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    // 4タップボックスフィルタ（バイリニア補間利用）
    float4 d = texelSize.xyxy * float4(-0.5, -0.5, 0.5, 0.5);

    float4 s;
    s = inputTexture.Sample(linearSampler, input.uv + d.xy); // 左上
    s += inputTexture.Sample(linearSampler, input.uv + d.zy); // 右上
    s += inputTexture.Sample(linearSampler, input.uv + d.xw); // 左下
    s += inputTexture.Sample(linearSampler, input.uv + d.zw); // 右下

    // 平均化
    return s * 0.25;
}