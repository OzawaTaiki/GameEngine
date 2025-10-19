#pragma once

#include <Features/Model/Primitive/Primitive.h>
#include <Features/Model/Primitive/Plane.h>
#include <Features/Model/Primitive/Triangle.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/Model/Primitive/Ring.h>
#include <Features/Model/Primitive/Cube.h>
#include <Features/Model/Model.h>
#include <Features/Json/Loader/JsonFileIO.h>

#include <memory>
#include <string>
#include <vector>

/// <summary>
/// プリミティブの種類
/// </summary>
enum class PrimitiveType
{
    Plane,
    Triangle,
    Cylinder,
    Ring,
    Cube
};

/// <summary>
/// プリミティブ作成用の設定データ
/// </summary>
struct PrimitiveSettings
{
    // 共通設定
    std::string name = "NewPrimitive";
    uint32_t divide = 16;
    bool flipU = false;
    bool flipV = false;

    // Plane用設定
    struct PlaneData
    {
        Vector3 normal = { 0.0f, 0.0f, 1.0f };
        Vector2 size = { 2.0f, 2.0f };
        Vector3 pivot = { 0.0f, 0.0f, 0.0f };
    } plane;

    // Triangle用設定
    struct TriangleData
    {
        Vector3 vertex0 = { 0.0f, 1.0f, 0.0f };
        Vector3 vertex1 = { -1.0f, -1.0f, 0.0f };
        Vector3 vertex2 = { 1.0f, -1.0f, 0.0f };
        Vector3 normal = { 0.0f, 0.0f, -1.0f };
    } triangle;

    // Cylinder用設定
    struct CylinderData
    {
        float topRadius = 1.0f;
        float bottomRadius = 1.0f;
        float height = 2.0f;
        bool hasTop = false;
        bool hasBottom = false;
        float startAngle = 0.0f;
        float endAngle = 6.28318f; // 2π
        bool loop = true;
    } cylinder;

    // Ring用設定
    struct RingData
    {
        float innerRadius = 0.5f;
        float outerRadius = 1.0f;
        float startAngle = 0.0f;
        float endAngle = 6.28318f; // 2π
        float startOuterRadiusRatio = 1.0f;
        float endOuterRadiusRatio = 1.0f;
    } ring;

    // Cube用設定
    struct CubeData
    {
        Vector3 size = { 1.0f, 1.0f, 1.0f };
        Vector3 pivot = { 0.0f, 0.0f, 0.0f };
        bool hasTop = true;
        bool hasBottom = true;
    } cube;
};

/// <summary>
/// 作成されたプリミティブ情報
/// </summary>
struct CreatedPrimitive
{
    PrimitiveType type;
    PrimitiveSettings settings;
    Model* model;
    bool isSaved = false;

    CreatedPrimitive(PrimitiveType t, const PrimitiveSettings& s, Model* m)
        : type(t), settings(s), model(m) {
    }
};

/// <summary>
/// プリミティブ作成・管理クラス
/// </summary>
class PrimitiveCreator
{
public:
    PrimitiveCreator();
    ~PrimitiveCreator();

    /// <summary>
    /// ImGuiでプリミティブ作成UIを描画
    /// </summary>
    void DrawImGui();

    /// <summary>
    /// プリミティブを作成
    /// </summary>
    /// <param name="type">作成するプリミティブの種類</param>
    /// <param name="settings">プリミティブ設定</param>
    /// <returns>作成されたModel</returns>
    Model* CreatePrimitive(PrimitiveType type, const PrimitiveSettings& settings);

    /// <summary>
    /// 作成されたプリミティブの一覧を取得
    /// </summary>
    const std::vector<CreatedPrimitive>& GetCreatedPrimitives() const { return createdPrimitives_; }

private:
    /// <summary>
    /// パラメータ設定UIを描画
    /// </summary>
    void DrawParameterSettings();

    /// <summary>
    /// 作成済みプリミティブ一覧を描画
    /// </summary>
    void DrawCreatedPrimitivesList();

    /// <summary>
    /// 選択されたプリミティブのパラメータを表示
    /// </summary>
    void DrawSelectedPrimitiveParameters();

    /// <summary>
    /// 各プリミティブの設定UI
    /// </summary>
    void DrawPlaneSettings();
    void DrawTriangleSettings();
    void DrawCylinderSettings();
    void DrawRingSettings();
    void DrawCubeSettings();

    /// <summary>
    /// プリミティブをファイルに保存
    /// </summary>
    void SavePrimitiveToFile(const CreatedPrimitive& primitive);

    /// <summary>
    /// プリミティブタイプを文字列に変換
    /// </summary>
    const char* PrimitiveTypeToString(PrimitiveType type);

private:
    // 現在の設定
    PrimitiveSettings currentSettings_;
    PrimitiveType currentType_;

    // 作成されたプリミティブのリスト
    std::vector<CreatedPrimitive> createdPrimitives_;

    // UI状態
    int selectedPrimitiveIndex_;
};