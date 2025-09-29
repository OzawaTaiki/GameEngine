
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

    float2 screenSize;

    float4x4 vp;
};

VertexOutput VSmain(VertexInput _input)
{
    VertexOutput output;

    float ratio = 0;
    float x = _input.waveformData.x + leftTop.x;

    ratio = (_input.waveformData.y + 1.0) / 2.0; // -1.0 ~ 1.0 -> 0.0 ~ 1.0
    //ratio = _input.waveformData.y;
    float y = lerp(leftTop.y + displayHeight, leftTop.y, ratio);

    output.position = mul(float4(x, y, 0.0f, 1.0f), vp);

    output.color = color;

    return output;

}

float4 PSmain(VertexOutput input) : SV_TARGET
{
    return input.color;
}