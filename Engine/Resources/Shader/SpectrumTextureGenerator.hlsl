
struct ConstantBuff
{
    uint TextureWidth;
    uint TextureHeight;
    float maxMagnitude; // 最大の強度
    uint spectrumDataCount; // データの数

    uint spectrumDrawCount; // 描画するデータの数
    float pieceWidth; // 幅
    float pieceMargin; // 余白
    float rms; // RMS値
};

cbuffer cb : register(b0)
{
    ConstantBuff cb;
}

StructuredBuffer<float> spectrumData : register(t0);
StructuredBuffer<int2> fftRanges : register(t1);

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

    float sum = 0.0f;
    int count = 0;

    for (uint i = fftRanges[instanceID].x; i < fftRanges[instanceID].y; ++i)
    {
        sum += spectrumData[i];
        ++count;
    }

    float avgMagnitude = 0.0f;
    if (count > 0) // 0割り防止
        avgMagnitude = sum / count;

    // dbに変換
    float db = 20.0f * log10(avgMagnitude + 1e-12f);
    float normalized = (db + 60.0f) / 50.0f;
    normalized = clamp(normalized, 0.0f, 1.0f);

    // 形状 : 音量 = 7 : 3 で加算合成して音量による変化をつける
    //float baseHeight = saturate(magnitude / cb.maxMagnitude) * 0.8f; // 0~1に正規化
    //float volumeBoost = cb.rms * 0.2;
    //float t = baseHeight + volumeBoost;
    float y = localpos.y * normalized; // 高さを変える


    float offset = instanceID * (cb.pieceWidth + cb.pieceMargin) + (cb.pieceMargin); //端にmarginを入れる
    float x = localpos.x * cb.pieceWidth / 2 + offset;
    x /= (float) cb.TextureWidth; // 0~1に正規化

    return float4(x * 2.0 - 1.0, y * 2.0 - 1.0, 0, 1);
}


float4 PSmain() : SV_Target
{
    return float4(1, 1, 1, 1);
}