#include "BezierCurve3D.h"

#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiDebugManager.h>

// デフォルトコンストラクタ
BezierCurve3D::BezierCurve3D(const std::string& _name)
    : resolution_(100)  // デフォルトの解像度を100に設定
    , isCacheValid_(false)
{
    name_ = ImGuiDebugManager::GetInstance()->AddDebugWindow(_name, [&]() {
        ImGui();
        });
}

// 制御点を指定するコンストラクタ
BezierCurve3D::BezierCurve3D(const std::string& _name,const std::vector<Vector3>& _controlPoints)
    : controlPoints_(_controlPoints)
    , resolution_(100)  // デフォルトの解像度を100に設定
    , isCacheValid_(false)
{
    name_ = ImGuiDebugManager::GetInstance()->AddDebugWindow(_name, [&]() {
        ImGui();
        });
}

// デストラクタ
BezierCurve3D::~BezierCurve3D()
{
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow(name_);
}

void BezierCurve3D::Draw(const Vector4& _color) const
{
    if (!isDraw_)  return;

    // 曲線上の点を生成
    std::vector<Vector3> points = GenerateCurvePoints();

    // 連続する点間に線を描画
    for (size_t i = 1; i < points.size(); ++i)
    {
        LineDrawer::GetInstance()->RegisterPoint(points[i - 1], points[i], _color);
    }
}

void BezierCurve3D::DrawWithControlPoints(const Vector4& _curveColor, const Vector4& _controlPointColor) const
{
    if (!isDraw_)  return;

    // 曲線を描画
    Draw(_curveColor);

    // 制御点間に線を描画（制御多角形）
    for (size_t i = 1; i < controlPoints_.size(); ++i)
    {
        LineDrawer::GetInstance()->RegisterPoint(controlPoints_[i - 1], controlPoints_[i], { 0.0f, 1.0f, 0.0f, 0.5f }); // 半透明の緑色
    }

    // 制御点を視覚化
    const float pointSize = 0.1f; // 十字のサイズ

    for (const auto& point : controlPoints_)
    {
        // 各制御点に小さな十字を描画
        Vector3 horizontal1 = point + Vector3(-pointSize, 0.0f, 0.0f);
        Vector3 horizontal2 = point + Vector3(pointSize, 0.0f, 0.0f);
        Vector3 vertical1 = point + Vector3(0.0f, -pointSize, 0.0f);
        Vector3 vertical2 = point + Vector3(0.0f, pointSize, 0.0f);

        LineDrawer::GetInstance()->RegisterPoint(horizontal1, horizontal2, _controlPointColor);
        LineDrawer::GetInstance()->RegisterPoint(vertical1, vertical2, _controlPointColor);
    }
}

void BezierCurve3D::Load(const std::string& _filePath)
{
    if (jsonBinder_ == nullptr)
        jsonBinder_ = std::make_unique<JsonBinder>(name_, _filePath);

    // JSONから制御点を読み込む
    jsonBinder_->RegisterVariable("ControlPoints", &controlPoints_);
    jsonBinder_->RegisterVariable("Resolution", &resolution_);

}

void BezierCurve3D::Save()
{
    // 初期化のためLoadを呼ぶ
    if (jsonBinder_ == nullptr)        Load();

    // JSONファイルに保存
    jsonBinder_->Save();
}

// 制御点を設定する関数
void BezierCurve3D::SetControlPoints(const std::vector<Vector3>& _points)
{
    // 新しい制御点の配列で既存の配列を置き換える
    controlPoints_ = _points;

    // 制御点が変更されたので、キャッシュを無効化する
    InvalidateCache();
}

// 制御点を追加する関数
void BezierCurve3D::AddControlPoint(const Vector3& _point)
{
    // 既存の制御点リストの末尾に新しい点を追加
    controlPoints_.push_back(_point);

    // 制御点が変更されたので、キャッシュを無効化する
    InvalidateCache();
}

// 指定したインデックスの制御点を更新する関数
void BezierCurve3D::UpdateControlPoint(size_t _index, const Vector3& _point)
{
    // インデックスが有効範囲内かチェック
    if (_index < controlPoints_.size())
    {
        // 指定されたインデックスの制御点を新しい点で更新
        controlPoints_[_index] = _point;

        // 制御点が変更されたので、キャッシュを無効化する
        InvalidateCache();
    }
    // 範囲外の場合は何もしない
    // オプション: 例外をスローするか、エラーログを出力することも可能
}

// 制御点の配列を取得する関数
const std::vector<Vector3>& BezierCurve3D::GetControlPoints() const
{
    // 内部の制御点配列への参照を返す
    return controlPoints_;
}

// 曲線上の点を計算する関数
Vector3 BezierCurve3D::CalculatePoint(float _t) const
{
    // tの値を0〜1の範囲に制限
    if (_t <= 0.0f)
    {
        return controlPoints_.front(); // 始点を返す
    }
    if (_t >= 1.0f)
    {
        return controlPoints_.back(); // 終点を返す
    }

    // 制御点が不足している場合
    if (controlPoints_.size() < 2)
    {
        return controlPoints_.empty() ? Vector3() : controlPoints_[0];
    }

    // 特別な最適化：制御点が4つ（3次ベジエ曲線）の場合
    if (controlPoints_.size() == 4)
    {
        return CalculateCubicPoint(_t,
            controlPoints_[0],
            controlPoints_[1],
            controlPoints_[2],
            controlPoints_[3]);
    }

    // 一般的なケースでは再帰的なde Casteljauアルゴリズムを使用
    return CalculateBezierPointRecursive(_t, controlPoints_);
}

// 3次ベジエ曲線の計算（最適化のため）
Vector3 BezierCurve3D::CalculateCubicPoint(float _t, const Vector3& _p0, const Vector3& _p1,
    const Vector3& _p2, const Vector3& _p3) const
{
    // 3次ベジエ曲線の公式を直接使用（より効率的）
    float u = 1.0f - _t;
    float tt = _t * _t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * _t;

    // B(t) = (1-t)^3 * P0 + 3(1-t)^2 * t * P1 + 3(1-t) * t^2 * P2 + t^3 * P3
    Vector3 point;

    // X座標の計算
    point.x = uuu * _p0.x +
        3.0f * uu * _t * _p1.x +
        3.0f * u * tt * _p2.x +
        ttt * _p3.x;

    // Y座標の計算
    point.y = uuu * _p0.y +
        3.0f * uu * _t * _p1.y +
        3.0f * u * tt * _p2.y +
        ttt * _p3.y;

    // Z座標の計算
    point.z = uuu * _p0.z +
        3.0f * uu * _t * _p1.z +
        3.0f * u * tt * _p2.z +
        ttt * _p3.z;

    return point;
}

// de Casteljau アルゴリズムによるベジエ曲線の再帰的計算
Vector3 BezierCurve3D::CalculateBezierPointRecursive(float _t, const std::vector<Vector3>& _points) const
{
    // 再帰の終了条件：点が1つになった場合
    if (_points.size() == 1)
    {
        return _points[0];
    }

    // 新しい制御点を計算
    std::vector<Vector3> newPoints(_points.size() - 1);

    for (size_t i = 0; i < newPoints.size(); ++i)
    {
        // 隣接する点間を線形補間
        newPoints[i].x = (1.0f - _t) * _points[i].x + _t * _points[i + 1].x;
        newPoints[i].y = (1.0f - _t) * _points[i].y + _t * _points[i + 1].y;
        newPoints[i].z = (1.0f - _t) * _points[i].z + _t * _points[i + 1].z;
    }

    // 再帰的に計算を続ける
    return CalculateBezierPointRecursive(_t, newPoints);
}


// 曲線の分割数（解像度）を設定する関数
void BezierCurve3D::SetResolution(int _resolution)
{
    if (_resolution > 0 && _resolution != resolution_)
    {
        resolution_ = _resolution;
        InvalidateCache();
    }
}

// 曲線の分割数（解像度）を取得する関数
int BezierCurve3D::GetResolution() const
{
    return resolution_;
}

// 曲線上の点を生成する関数
std::vector<Vector3> BezierCurve3D::GenerateCurvePoints() const
{
    // キャッシュが有効であれば、キャッシュされた点を返す
    if (isCacheValid_ && !cachedPoints_.empty())
    {
        return cachedPoints_;
    }

    // 制御点が不足している場合
    if (controlPoints_.size() < 2)
    {
        return controlPoints_;
    }

    // 曲線上の点を生成
    std::vector<Vector3> points;
    points.reserve(resolution_ + 1); // メモリを事前に確保（効率化）

    // t = 0から1まで分割して点を計算
    for (int i = 0; i <= resolution_; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(resolution_);
        points.push_back(CalculatePoint(t));
    }

    // 計算結果をキャッシュに保存
    cachedPoints_ = points;

    return points;
}

// 曲線の全長を計算する関数
float BezierCurve3D::CalculateTotalLength() const
{
    // 曲線上の点を取得（キャッシュがあればそれを使用）
    std::vector<Vector3> points = GenerateCurvePoints();

    // 点の配列から曲線の長さを計算
    return CalculateTotalLength(points);
}

// 指定された点の配列から曲線の全長を計算する関数
float BezierCurve3D::CalculateTotalLength(const std::vector<Vector3>& _points) const
{
    // 点が不足している場合
    if (_points.size() < 2)
    {
        return 0.0f;
    }

    float totalLength = 0.0f;

    // 隣接する点間の距離を合計
    for (size_t i = 1; i < _points.size(); ++i)
    {
        totalLength += (_points[i - 1] - _points[i]).Length();
    }

    return totalLength;
}

// 等間隔の点を生成する関数
std::vector<Vector3> BezierCurve3D::GenerateEquallySpacedPoints(int _numPoints) const
{
    // キャッシュが有効であれば、キャッシュされた点を返す
    if (isCacheValid_ && !cachedEquallySpacedPoints_.empty() &&
        static_cast<int>(cachedEquallySpacedPoints_.size()) == _numPoints)
    {
        return cachedEquallySpacedPoints_;
    }

    // 制御点が不足している場合
    if (controlPoints_.size() < 2)
    {
        return controlPoints_;
    }

    // _numPointsが小さすぎる場合は調整
    _numPoints = (std::max)(_numPoints, 2);

    // 高解像度の曲線上の点を生成（より正確な近似のため）
    const int highResolution = (std::max)(resolution_ * 10, _numPoints * 10);
    std::vector<Vector3> finePoints;
    finePoints.reserve(highResolution + 1);

    for (int i = 0; i <= highResolution; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(highResolution);
        finePoints.push_back(CalculatePoint(t));
    }

    // 曲線の全長を計算
    float totalLength = CalculateTotalLength(finePoints);

    // 等間隔の点を生成
    std::vector<Vector3> equallySpacedPoints;
    equallySpacedPoints.reserve(_numPoints);

    // 始点を追加
    equallySpacedPoints.push_back(finePoints.front());

    if (totalLength > 0.0f && _numPoints > 1)
    {
        float segmentLength = totalLength / static_cast<float>(_numPoints - 1);
        float currentLength = 0.0f;
        float targetLength = segmentLength;

        // 始点はすでに追加済み
        size_t currentPointIndex = 1;

        for (size_t i = 1; i < finePoints.size(); ++i)
        {
            float segmentDistance = (finePoints[i - 1]- finePoints[i]).Length();

            if (currentLength + segmentDistance >= targetLength)
            {
                // 補間して新しい点を作成
                float t = (targetLength - currentLength) / segmentDistance;
                Vector3 newPoint = Vector3::Lerp(finePoints[i - 1], finePoints[i], t);
                equallySpacedPoints.push_back(newPoint);

                // 次の目標距離へ
                targetLength += segmentLength;
                currentPointIndex++;

                // 必要な点の数に達したら終了
                if (currentPointIndex >= static_cast<size_t>(_numPoints))
                {
                    break;
                }

                // 現在のセグメント内で複数の等距離点が必要な場合があるため
                // iをデクリメントして同じセグメントを再評価
                --i;
            }
            else
            {
                currentLength += segmentDistance;
            }
        }
    }

    // 必要に応じて終点を追加
    if (equallySpacedPoints.size() < static_cast<size_t>(_numPoints))
    {
        equallySpacedPoints.push_back(finePoints.back());
    }

    // 計算結果をキャッシュに保存
    cachedEquallySpacedPoints_ = equallySpacedPoints;

    return equallySpacedPoints;
}

// 指定した距離の位置にある点を取得する関数
Vector3 BezierCurve3D::GetPointAtDistance(float _distance) const
{
    // 曲線上の点を取得
    std::vector<Vector3> points = GenerateCurvePoints();

    // 点が不足している場合
    if (points.size() < 2)
    {
        return points.empty() ? Vector3() : points[0];
    }

    // 曲線の全長を計算
    float totalLength = CalculateTotalLength(points);

    // 距離が範囲外の場合は始点または終点を返す
    if (_distance <= 0.0f)
    {
        return points.front();
    }
    if (_distance >= totalLength)
    {
        return points.back();
    }

    // 指定した距離に対応する点を探索
    float currentDistance = 0.0f;

    for (size_t i = 1; i < points.size(); ++i)
    {
        float segmentLength = (points[i - 1]- points[i]).Length();

        if (currentDistance + segmentLength >= _distance)
        {
            // 補間して点を計算
            float t = (_distance - currentDistance) / segmentLength;
            return Vector3::Lerp(points[i - 1], points[i], t);
        }

        currentDistance += segmentLength;
    }

    // 万が一ここまで来た場合は終点を返す
    return points.back();
}

// 指定した距離に対応するtパラメータを取得する関数（逆マッピング）
float BezierCurve3D::GetParameterAtDistance(float _distance) const
{
    // 曲線上の点を取得
    std::vector<Vector3> points = GenerateCurvePoints();

    // 点が不足している場合
    if (points.size() < 2)
    {
        return 0.0f;
    }

    // 曲線の全長を計算
    float totalLength = CalculateTotalLength(points);

    // 距離が範囲外の場合はtの限界値を返す
    if (_distance <= 0.0f)
    {
        return 0.0f;
    }
    if (_distance >= totalLength)
    {
        return 1.0f;
    }

    // 指定した距離に対応するtパラメータを探索
    float currentDistance = 0.0f;

    for (size_t i = 1; i < points.size(); ++i)
    {
        float segmentLength = (points[i - 1]- points[i]).Length();

        if (currentDistance + segmentLength >= _distance)
        {
            // セグメント内での比率を計算
            float segmentT = (_distance - currentDistance) / segmentLength;

            // tパラメータの線形補間
            float t0 = static_cast<float>(i - 1) / static_cast<float>(points.size() - 1);
            float t1 = static_cast<float>(i) / static_cast<float>(points.size() - 1);

            return t0 + segmentT * (t1 - t0);
        }

        currentDistance += segmentLength;
    }

    // 万が一ここまで来た場合は1.0を返す
    return 1.0f;
}

// 等間隔移動のためのtパラメータ配列を生成する関数
std::vector<float> BezierCurve3D::GenerateEquallySpacedTValues(int _numPoints) const
{
    // キャッシュが有効であれば、キャッシュされたt値を返す
    if (isCacheValid_ && !cachedEquallySpacedTValues_.empty() &&
        static_cast<int>(cachedEquallySpacedTValues_.size()) == _numPoints)
    {
        return cachedEquallySpacedTValues_;
    }

    // 等距離の点を生成
    std::vector<Vector3> equallySpacedPoints = GenerateEquallySpacedPoints(_numPoints);

    // 各点に対応するtの値を計算
    std::vector<float> tValues;
    tValues.reserve(_numPoints);

    // 始点のt値は0
    tValues.push_back(0.0f);

    if (equallySpacedPoints.size() > 1)
    {
        // 曲線の全長を計算
        float totalLength = CalculateTotalLength();

        // 始点からの累積距離
        float accumulatedDistance = 0.0f;

        // 始点を除く各点について、対応するtの値を計算
        for (size_t i = 1; i < equallySpacedPoints.size() - 1; ++i)
        {
            // 前の点からの距離を計算
            accumulatedDistance += (equallySpacedPoints[i - 1] - equallySpacedPoints[i]).Length();

            // 距離からtの値を逆算
            float t = GetParameterAtDistance(accumulatedDistance);
            tValues.push_back(t);
        }

        // 終点のt値は1
        tValues.push_back(1.0f);
    }

    // 計算結果をキャッシュに保存
    cachedEquallySpacedTValues_ = tValues;

    return tValues;
}

// 進行度に基づいて等間隔移動のための位置を計算する関数
Vector3 BezierCurve3D::GetPositionAtEqualDistanceProgress(float _progress,
    const std::vector<float>* _tValues) const
{
    // progressを0〜1の範囲に制限
    _progress = (std::max)(0.0f, (std::min)(1.0f, _progress));

    // tValues配列の取得（または生成）
    std::vector<float> localTValues;
    if (_tValues == nullptr)
    {
        // デフォルトでは100個のt値を使用
        localTValues = GenerateEquallySpacedTValues(100);
        _tValues = &localTValues;
    }

    // 制御点が不足している場合や、t値が空の場合
    if (controlPoints_.size() < 2 || _tValues->empty())
    {
        return controlPoints_.empty() ? Vector3() : controlPoints_[0];
    }

    // progressが0または1の場合は、始点または終点を直接返す
    if (_progress <= 0.0f)
    {
        return CalculatePoint(0.0f);
    }
    if (_progress >= 1.0f)
    {
        return CalculatePoint(1.0f);
    }

    // progressに基づいてtValues配列の適切なインデックスを計算
    float indexF = _progress * static_cast<float>(_tValues->size() - 1);
    int index = static_cast<int>(indexF);
    index = (std::min)(index, static_cast<int>(_tValues->size() - 2));

    // 2つのt値の間を補間
    float segmentProgress = indexF - static_cast<float>(index);
    float t1 = (*_tValues)[index];
    float t2 = (*_tValues)[index + 1];
    float interpolatedT = t1 + (t2 - t1) * segmentProgress;

    // 補間されたtを使って位置を計算
    return CalculatePoint(interpolatedT);
}

// キャッシュを無効化する関数
void BezierCurve3D::InvalidateCache()
{
    isCacheValid_ = false;
    cachedPoints_.clear();
    cachedEquallySpacedPoints_.clear();
    cachedEquallySpacedTValues_.clear();
}

void BezierCurve3D::DrawMovingCross(float _progress, float _crossSize, const Vector4& _color) const
{
    // 等速移動のためのt値が必要
    static std::vector<float> equallySpacedTValues;

    // 必要に応じてt値を生成（一度だけ計算してキャッシュ）
    if (equallySpacedTValues.empty())
    {
        equallySpacedTValues = GenerateEquallySpacedTValues(100);
    }

    // 進行度を0〜1の範囲に制限
    _progress = (std::max)(0.0f, (std::min)(1.0f, _progress));

    // 等速移動の位置を計算
    Vector3 position = GetPositionAtEqualDistanceProgress(_progress, &equallySpacedTValues);

    // 十字を描画
    Vector3 horizontal1 = position + Vector3(-_crossSize, 0.0f, 0.0f);
    Vector3 horizontal2 = position + Vector3(_crossSize, 0.0f, 0.0f);
    Vector3 vertical1 = position + Vector3(0.0f, -_crossSize, 0.0f);
    Vector3 vertical2 = position + Vector3(0.0f, _crossSize, 0.0f);

    LineDrawer::GetInstance()->RegisterPoint(horizontal1, horizontal2, _color);
    LineDrawer::GetInstance()->RegisterPoint(vertical1, vertical2, _color);
}

void BezierCurve3D::ImGui()
{
#ifdef _DEBUG

    ImGui::PushID(this);

    // 曲線の解像度を編集
    int resolution = resolution_;
    if (ImGui::SliderInt("Resolution", &resolution, 10, 200))
    {
        SetResolution(resolution);
    }

    ImGui::Checkbox("DrawLine", &isDraw_);

    // 各制御点を編集
    ImGui::Text("Control Points");
    ImGui::Indent();

    bool pointsChanged = false;
    for (size_t i = 0; i < controlPoints_.size(); i++)
    {
        std::string pointlabelStr = "Point " + std::to_string(i);

        // 制御点の座標を編集
        if (ImGui::DragFloat3(pointlabelStr.c_str(), &controlPoints_[i].x, 0.01f))
        {
            pointsChanged = true;
        }

        // 制御点削除ボタン（始点と終点は最低限必要なので削除しない）
        if (controlPoints_.size() > 2 && i > 0 && i < controlPoints_.size() - 1)
        {
            ImGui::SameLine();
            std::string deleteLabelStr = "Delete##" + std::to_string(i);
            if (ImGui::Button(deleteLabelStr.c_str()))
            {
                controlPoints_.erase(controlPoints_.begin() + i);
                pointsChanged = true;
                i--; // インデックスを調整
            }
        }
    }

    // 制御点追加ボタン
    if (ImGui::Button("Add Control Point"))
    {
        // 新しい制御点を追加する位置を決定
        // デフォルトでは最後の制御点の位置に少しオフセットを加えた位置に追加
        Vector3 newPoint;
        if (controlPoints_.empty())
        {
            newPoint = Vector3(0.0f, 0.0f, 0.0f);
        }
        else
        {
            newPoint = controlPoints_.back() + Vector3(20.0f, 0.0f, 0.0f);
        }

        controlPoints_.push_back(newPoint);
        pointsChanged = true;
    }

    // 制御点のリセットボタン
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        // 制御点をJSONファイルに保存
        Save();
    }
    ImGui::Separator();

    if (ImGui::Button("Reset Points"))
    {
        // デフォルトの制御点に戻す
        controlPoints_.clear();
        controlPoints_.push_back(Vector3(0.0f, 0.0f, 0.0f));
        controlPoints_.push_back(Vector3(1.0f, 1.0f, 0.0f));
        controlPoints_.push_back(Vector3(2.0f, 1.0f, 0.0f));
        controlPoints_.push_back(Vector3(3.0f, 0.0f, 0.0f));
        pointsChanged = true;
    }

    // 点が変更された場合はキャッシュを無効化
    if (pointsChanged)
    {
        InvalidateCache();
    }

    // 曲線の情報表示
    if (!controlPoints_.empty())
    {
        ImGui::Separator();
        ImGui::Text("Curve Information:");
        float length = CalculateTotalLength();
        ImGui::Text("Total Length: %.2f", length);
        ImGui::Text("Control Points: %zu", controlPoints_.size());
    }

    ImGui::Unindent();
    ImGui::PopID();

#endif // _DEBUG
}