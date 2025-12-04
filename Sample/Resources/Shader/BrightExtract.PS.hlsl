
cbuffer BrightExtractParams : register(b0)
{
    float threshold; // 1.0～2.0が標準
    float intensity; // 1.0が標準
    float softKnee; // 0.5が標準（0.0～1.0）
    float padding;
};

Texture2D<float4> sceneTexture : register(t0);
SamplerState linearSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 color = sceneTexture.Sample(linearSampler, input.uv);

    // 輝度計算
    float brightness = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

    // 閾値処理
    float knee = threshold * softKnee;
    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);

    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 0.00001);

    // 最終出力
    return float4(color.rgb * contribution * intensity, 1.0);
}