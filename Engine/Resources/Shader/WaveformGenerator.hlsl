
struct VertexInput
{
    float2 waveformData : POSITION0; // x: time, y: amplitude
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


cbuffer Constants : register(b0)
{
    float4 color;

    float2 leftTop;
    float startTime;
    float displayDuration;

    float displayHeight;
    float displayWidth;

    float2 pad;
};

VertexOutput VSmain(VertexInput _input)
{
    VertexOutput output;

    float ratio = (_input.waveformData.x - startTime) / displayDuration;
    float x = lerp(0, displayWidth, ratio) + leftTop.x;

    ratio = (_input.waveformData.y + 1.0) / 2.0; // -1.0 ~ 1.0 -> 0.0 ~ 1.0
    float y = lerp(leftTop.y + displayHeight, leftTop.y, ratio);

    output.position = float4(x, y, 0.0f, 1.0f);
    output.color = color;

    return output;

}

float4 PSmain(VertexOutput input) : SV_TARGET
{
    return input.color;
}