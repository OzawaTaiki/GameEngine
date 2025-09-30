
struct ConstantBuff
{
    uint TextureWidth;
    uint TextureHeight;
    float maxMagnitude; // 最大の強度
    uint spectrumDataCount; // データの数

    uint spectrumDrawCount; // 描画するデータの数
    float pieceWidth; // 幅
    float pieceMargin; // 余白
};

cbuffer cb : register(b0)
{
    ConstantBuff cb;
}

StructuredBuffer<float> spectrumData : register(t0);

// 1x1 の矩形 下中央が原点
static const float2 quad[6] =
{
    { -0.5f, 0.0f },
    {  0.5f, 0.0f },
    { -0.5f, 1.0f },

    {  0.5f, 0.0f },
    { -0.5f, 1.0f },
    {  0.5f, 1.0f }
};

float4 VSmain(uint instanceID : SV_InstanceID, uint Vertexid : SV_VertexID) : SV_Position
{
    float4 localpos = float4(quad[Vertexid], 0, 1);

    float magnitude = 0;

    uint margeDataCount = cb.spectrumDataCount / cb.spectrumDrawCount;
    for (uint i = 0; i < margeDataCount; ++i)
    {
        magnitude = max(magnitude, spectrumData[instanceID * margeDataCount + i]);
        //magnitude += spectrumData[instanceID * margeDataCount + i];
    }
    //magnitude /= margeDataCount; // 平均を取る

    float t = saturate(magnitude / cb.maxMagnitude); // 0~1に正規化
    float y = localpos.y * t; // 高さを変える


    float offset = instanceID * (cb.pieceWidth + cb.pieceMargin) + (cb.pieceMargin); //端にmarginを入れる
    float x = localpos.x * cb.pieceWidth / 2 + offset;
    x /= (float) cb.TextureWidth; // 0~1に正規化

    return float4(x * 2.0 - 1.0, y * 2.0 - 1.0, 0, 1);
}


float4 PSmain() : SV_Target
{
    return float4(1, 1, 1, 1);
}