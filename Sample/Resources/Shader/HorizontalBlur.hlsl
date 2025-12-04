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
    // ガウシアンの重み
    const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

    float4 result = inputTexture.Sample(linearSampler, input.uv) * weights[0];

    for (int i = 1; i < 5; ++i)
    {
        // blurRadiusで範囲を調整
        float offset = i * blurRadius * texelSize.x;
        result += inputTexture.Sample(linearSampler, input.uv + float2(offset, 0.0)) * weights[i];
        result += inputTexture.Sample(linearSampler, input.uv - float2(offset, 0.0)) * weights[i];
    }

    return result;
}