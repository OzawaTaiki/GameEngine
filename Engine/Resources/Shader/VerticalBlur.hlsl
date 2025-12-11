cbuffer HorizontalBlurParams : register(b0)
{
    float2 texelSize; // (1.0/width, 1.0/height)
    float blurRadius; // 例: 2.0～8.0
    float pad;
    //uint sampleCount; // タップ数（通常5～13）
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
    const float weights[9] =
    {
        0.13298, 0.23227, 0.13298, 0.05618,
        0.01468, 0.00268, 0.00033, 0.00003, 0.00000
    };

    float4 result = inputTexture.Sample(linearSampler, input.uv) * weights[0];

    for (int i = 1; i < 9; ++i)
    {
        float offset = i * blurRadius * texelSize.y; // Y方向！
        result += inputTexture.Sample(linearSampler, input.uv + float2(0.0, offset)) * weights[i];
        result += inputTexture.Sample(linearSampler, input.uv - float2(0.0, offset)) * weights[i];
    }

    return result;
}