#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>

#include <Features/Json/JsonBinder.h>

#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <string>

namespace Engine {

class BezierCurve3D{
public:

    BezierCurve3D(const std::string& _name);
    explicit BezierCurve3D(const std::string& _name,const std::vector<Vector3>& _controlPoints);
    ~BezierCurve3D();

    void Draw(const Vector4& _color = { 1.0f, 0.0f, 0.0f, 1.0f }) const;
    void DrawWithControlPoints(const Vector4& _curveColor = { 1.0f, 0.0f, 0.0f, 1.0f },
        const Vector4& _controlPointColor = { 0.0f, 0.0f, 1.0f, 1.0f }) const;
    // 動作確認用 十字がライン上を動く
    void DrawMovingCross(float _progress, float _crossSize = 0.4f, const Vector4& _color = { 1.0f, 1.0f, 0.0f, 1.0f }) const;


    void Draw(const Vector3& _worldPos, const Vector4& _color = { 1.0f, 0.0f, 0.0f, 1.0f }) const;
    void DrawWithControlPoints(const Vector3& _worldPos, const Vector4& _curveColor = { 1.0f, 0.0f, 0.0f, 1.0f },
        const Vector4& _controlPointColor = { 0.0f, 0.0f, 1.0f, 1.0f }) const;
    // 動作確認用 十字がライン上を動く
    void DrawMovingCross(const Vector3& _worldPos, float _progress, float _crossSize = 0.4f, const Vector4& _color = { 1.0f, 1.0f, 0.0f, 1.0f }) const;

    void Load(const std::string& _filePath = "Resources/data/Bezier/");

    void Save();


    // 制御点の操作
    void SetControlPoints(const std::vector<Vector3>& _points);
    void AddControlPoint(const Vector3& _point);
    void UpdateControlPoint(size_t _index, const Vector3& _point);
    const std::vector<Vector3>& GetControlPoints() const;

    // 曲線上の点の計算
    Vector3 CalculatePoint(float _t) const;

    // 3次ベジエ曲線の計算（最適化のため）
    Vector3 CalculateCubicPoint(float _t, const Vector3& _p0, const Vector3& _p1,const Vector3& _p2, const Vector3& _p3) const;

    // 曲線の分割数（解像度）を設定
    void SetResolution(int _resolution);
    int GetResolution() const;

    // 曲線上の点を生成
    std::vector<Vector3> GenerateCurvePoints() const;

    // 曲線の全長を計算
    float CalculateTotalLength() const;
    float CalculateTotalLength(const std::vector<Vector3>& _points) const;

    void SetWorldPosition(const Vector3& _position) { worldPosition_ = _position; InvalidateCache(); }

    // 等間隔の点を生成
    std::vector<Vector3> GenerateEquallySpacedPoints(int _numPoints = 50) const;

    // 指定した距離の位置にある点を取得
    Vector3 GetPointAtDistance(float _distance) const;

    // 指定した距離に対応するtパラメータを取得（逆マッピング）
    float GetParameterAtDistance(float _distance) const;

    // 等間隔移動のためのtパラメータ配列を生成
    std::vector<float> GenerateEquallySpacedTValues(int _numPoints = 50) const;

    // 進行度に基づいて等間隔移動のための位置を計算
    Vector3 GetPositionAtEqualDistanceProgress(float _progress, const std::vector<float>* _tValues = nullptr) const;
    // キャッシュを無効化
    void InvalidateCache();


private:
    // 制御点
    std::vector<Vector3> controlPoints_;

    // ワールド空間での位置
    Vector3 worldPosition_;

    // 曲線の解像度（分割数）
    int resolution_;

    // キャッシュされた曲線上の点
    mutable std::vector<Vector3> cachedPoints_;

    // キャッシュされた等間隔の点
    mutable std::vector<Vector3> cachedEquallySpacedPoints_;

    // キャッシュされた等間隔移動のためのtパラメータ
    mutable std::vector<float> cachedEquallySpacedTValues_;

    // キャッシュが有効かどうか
    mutable bool isCacheValid_;

    bool isDraw_ = true;

    // de Casteljau アルゴリズムによるベジエ曲線の再帰的計算
    Vector3 CalculateBezierPointRecursive(float _t, const std::vector<Vector3>& _points) const;

    // デバッグ用ImGui
    void ImGui();
    // でばっぐWindowの名前
    std::string name_ = "BezierCurve3D";

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;
};

} // namespace Engine