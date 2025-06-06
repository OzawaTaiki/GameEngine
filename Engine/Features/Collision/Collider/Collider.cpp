#include "Collider.h"
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>

#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/LineDrawer/LineDrawer.h>

#include <Debug/ImGuiDebugManager.h>

#include <numbers>

void Collider::Initialize()
{
    if (isInitialized_)
        return;

    defaultTransform_.Initialize();
    isInitialized_ = true;
}

Collider::~Collider()
{
    CollisionManager::GetInstance()->UnregisterCollider(this);

#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow(name_);
#endif // _DEBUG
}

void Collider::OnCollision(Collider* _other, const ColliderInfo& _info)
{
    // 従来のコールバック（あれば）
    if (fOnCollision_) {
        fOnCollision_(_other, _info);
    }
}

bool Collider::IsCollidingWith(Collider* _other) const
{
    std::lock_guard<std::mutex> lock(collisionMutex_);
    auto it = collisionMap_.find(_other);
    if (it != collisionMap_.end()) {
        return it->second.isColliding;
    }
    return false;
}

CollisionState Collider::GetCollisionState(Collider* _other) const
{
    auto it = collisionMap_.find(_other);
    if (it != collisionMap_.end()) {
        const CollisionData& data = it->second;

        if (data.isColliding && !data.wasColliding) {
            return CollisionState::Enter; // 衝突開始
        }
        else if (data.isColliding && data.wasColliding) {
            return CollisionState::Stay;  // 衝突中
        }
        else if (!data.isColliding && data.wasColliding) {
            return CollisionState::Exit;  // 衝突終了
        }
    }

    return CollisionState::None; // 衝突なし
}

WorldTransform* Collider::GetWorldTransform()
{
    if (worldTransform_ == nullptr)
    {
        Initialize();
        return &defaultTransform_;
    }

    return worldTransform_;
}

const WorldTransform* Collider::GetWorldTransform() const
{
    if (worldTransform_ == nullptr)
    {
        return &defaultTransform_;
    }

    return worldTransform_;
}

Vector3 Collider::GetSize() const
{
    Vector3 scale;
    if (worldTransform_ != nullptr)
    {
        scale = worldTransform_->scale_;
    }
    else
    {
        scale = defaultTransform_.scale_;
    }

    return Vector3
    {
        size_.x * scale.x,
        size_.y * scale.y,
        size_.z * scale.z
    };
}

void Collider::AddCurrentCollision(Collider* _other, const ColliderInfo& _info)
{
    std::lock_guard<std::mutex> lock(collisionMutex_);

    // 現在のフレームでの衝突を記録
    currentCollisions_.push_back(_other);

    // 衝突マップに追加または更新
    auto it = collisionMap_.find(_other);
    if (it != collisionMap_.end()) {
        // 既存のエントリを更新
        it->second.info = _info;
        it->second.isColliding = true;
    }
    else {
        // 新しいエントリを追加
        CollisionData data;
        data.info = _info;
        data.isColliding = true;
        data.wasColliding = false;
        collisionMap_[_other] = data;
    }
}

bool Collider::InitJsonBinder(const std::string& _name, const std::string& _folderPath)
{
    if (jsonBinder_ == nullptr)
    {
        jsonBinder_ = new JsonBinder(_name, _folderPath);

        collisionLayer_.RegisterLayer(jsonBinder_);
        jsonBinder_->RegisterVariable("isStatic", &isStatic_);
        jsonBinder_->RegisterVariable("boundingBox", reinterpret_cast<uint32_t*>(&boundingBox_));
        jsonBinder_->RegisterVariable("transform", &defaultTransform_.transform_);
        jsonBinder_->RegisterVariable("scale", &defaultTransform_.scale_);
        jsonBinder_->RegisterVariable("quaternion", &defaultTransform_.quaternion_);

        return true;
    }

    return false;
}

void Collider::ImGui()
{
#ifdef _DEBUG

    if (worldTransform_ == nullptr)
    {
        ImGui::DragFloat3("Position", &defaultTransform_.transform_.x, 0.01f);
        ImGui::DragFloat3("Scale", &defaultTransform_.scale_.x, 0.01f);
        ImGui::DragFloat4("Quaternion", &defaultTransform_.quaternion_.x, 0.01f);
    }

    ImGui::DragFloat3("Offset", &offset_.x, 0.01f);
    ImGui::Text("Layer      : %x", collisionLayer_.GetLayer());
    ImGui::Text("LayerMask  : %x", collisionLayer_.GetLayerMask());
    ImGui::Text("BoundingBox: %s", ToString(boundingBox_).c_str());
    ImGui::Checkbox("Draw", &isDraw_);

#endif // _DEBUG
}

void Collider::UpdateCollisionState()
{
    std::lock_guard<std::mutex> lock(collisionMutex_);

    // すべてのマップエントリについて、現在衝突していないとマーク
    for (auto& pair : collisionMap_) {
        pair.second.isColliding = false;
    }

    // 現在のフレームでの衝突を設定
    for (Collider* other : currentCollisions_) {
        auto it = collisionMap_.find(other);
        if (it != collisionMap_.end()) {
            it->second.isColliding = true;
        }
    }

    // 状態の変化をチェックし、必要に応じてコールバックを実行
    for (auto& pair : collisionMap_) {
        Collider* other = pair.first;
        CollisionData& data = pair.second;

        ColliderInfo info = data.info;

        if (data.isColliding && !data.wasColliding) {
            // 衝突開始
            info.state = CollisionState::Enter;
            if (fOnCollisionEnter_) {
                fOnCollisionEnter_(other, info);
            }
        }
        else if (data.isColliding && data.wasColliding) {
            // 衝突中
            info.state = CollisionState::Stay;
            if (fOnCollisionStay_) {
                fOnCollisionStay_(other, info);
            }
        }
        else if (!data.isColliding && data.wasColliding) {
            // 衝突終了
            info.state = CollisionState::Exit;
            if (fOnCollisionExit_) {
                fOnCollisionExit_(other, info);
            }
        }

        // 現在の状態を前フレームの状態として保存
        data.wasColliding = data.isColliding;
    }

    // 現在のフレームでの衝突リストをクリア
    currentCollisions_.clear();
}

SphereCollider::SphereCollider(const char* _name) : Collider()
{
    SetBoundingBox(BoundingBox::Sphere_3D);
#ifdef _DEBUG
    name_ = ImGuiDebugManager::GetInstance()->AddColliderDebugWindow(_name, [&]() {ImGui(); });
#endif // _DEBUG
}

void SphereCollider::Draw()
{
    if (!isDraw_)
        return;

    // 球を描画
    // 球の中心をワールド空間に変換
    // オフセットをワールド変換（回転）で変換
    Vector3 worldOffset = Transform(offset_, GetWorldTransform()->quaternion_.ToMatrix());
    Vector3 center = GetWorldTransform()->transform_ + worldOffset;
    // 球の半径をスケール分拡大
    float scale = GetWorldTransform()->scale_.x;
    float radius = radius_ * scale;

    Matrix4x4 mat = MakeAffineMatrix({ radius,radius ,radius }, GetWorldTransform()->quaternion_, center);

    // 球を描画
    LineDrawer::GetInstance()->DrawSphere(mat);
}

void SphereCollider::Load(const std::string& _name)
{
    if (InitJsonBinder(_name))
        jsonBinder_->RegisterVariable("radius", &radius_);
}

void SphereCollider::Save(const std::string& _name)
{
    if (InitJsonBinder(_name))
        jsonBinder_->RegisterVariable("radius", &radius_);

    jsonBinder_->Save();
}

void SphereCollider::SetRadius(float _radius)
{
    radius_ = _radius;
    size_ = Vector3(_radius, _radius, _radius);
}

bool SphereCollider::Contains(const Vector3& _point)
{
    // オフセットとワールド変換を考慮した中心からの距離でチェックする
    const WorldTransform* transform = GetWorldTransform();
    Vector3 worldOffset = Transform(offset_, transform->quaternion_.ToMatrix());
    Vector3 center = transform->transform_ + worldOffset;
    float radius = radius_ * transform->scale_.x;

    return (_point - center).Length() <= radius;
}

Vector3 SphereCollider::GetClosestPoint(const Vector3& _point)
{
    // 同様にオフセットとワールド変換を考慮
    const WorldTransform* transform = GetWorldTransform();
    Vector3 worldOffset = Transform(offset_, transform->quaternion_.ToMatrix());
    Vector3 center = transform->transform_ + worldOffset;
    float radius = radius_ * transform->scale_.x;

    Vector3 direction = _point - center;
    float length = direction.Length();

    if (length <= 0.0001f)
        return center + Vector3(radius, 0, 0); // 適当な方向

    return center + direction / length * radius;
}

AABB SphereCollider::GetBounds() const
{
    // ワールド変換を取得
    const WorldTransform* transform = GetWorldTransform();

    // オフセットをワールド座標に変換
    Vector3 worldOffset = Transform(offset_, transform->quaternion_.ToMatrix());
    Vector3 center = transform->transform_ + worldOffset;

    // スケールを考慮した半径
    float worldRadius = radius_ * transform->scale_.x; // 均等スケールを想定

    Vector3 radiusVec(worldRadius, worldRadius, worldRadius);
    return AABB(center - radiusVec, center + radiusVec);
}

void SphereCollider::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    Collider::ImGui();
    ImGui::DragFloat("Radius", &radius_, 0.01f);
    ImGui::PopID();
#endif // _DEBUG
}

 SphereCollider::SphereCollider(bool _isTemporary) : Collider()
{
    SetBoundingBox(BoundingBox::Sphere_3D);
}

AABBCollider::AABBCollider(const char* _name) : Collider()
{
    SetBoundingBox(BoundingBox::AABB_3D);
#ifdef _DEBUG
    name_ = ImGuiDebugManager::GetInstance()->AddColliderDebugWindow(_name, [&]() {ImGui(); });
#endif // _DEBUG
}
void AABBCollider::Draw()
{
    if (!isDraw_)
        return;

    // AABBを描画するために8つの頂点を計算
    std::array<Vector3, 8> vertices;

    // ワールド空間での頂点を計算
    WorldTransform transform = *GetWorldTransform();
    Vector3 pos = transform.transform_ + Transform(offset_, transform.quaternion_.ToMatrix());
    Vector3 scale = transform.scale_;

    // ローカル空間でのAABBの頂点
    Vector3 scaledMin = Vector3(min_.x * scale.x, min_.y * scale.y, min_.z * scale.z);
    Vector3 scaledMax = Vector3(max_.x * scale.x, max_.y * scale.y, max_.z * scale.z);

    vertices[0] = pos + Vector3(scaledMax.x, scaledMax.y, scaledMax.z); // 右上前
    vertices[1] = pos + Vector3(scaledMax.x, scaledMax.y, scaledMin.z); // 右上後
    vertices[2] = pos + Vector3(scaledMax.x, scaledMin.y, scaledMax.z); // 右下前
    vertices[3] = pos + Vector3(scaledMax.x, scaledMin.y, scaledMin.z); // 右下後
    vertices[4] = pos + Vector3(scaledMin.x, scaledMax.y, scaledMax.z); // 左上前
    vertices[5] = pos + Vector3(scaledMin.x, scaledMax.y, scaledMin.z); // 左上後
    vertices[6] = pos + Vector3(scaledMin.x, scaledMin.y, scaledMax.z); // 左下前
    vertices[7] = pos + Vector3(scaledMin.x, scaledMin.y, scaledMin.z); // 左下後

    LineDrawer::GetInstance()->DrawOBB(vertices);
}

void AABBCollider::Load(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("min", &min_);
        jsonBinder_->RegisterVariable("max", &max_);
    }
}

void AABBCollider::Save(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("min", &min_);
        jsonBinder_->RegisterVariable("max", &max_);
    }

    jsonBinder_->Save();
}

void AABBCollider::SetMinMax(const Vector3& _min, const Vector3& _max)
{
    min_ = _min;
    max_ = _max;
    // サイズを更新
    size_ = max_ - min_;
}

bool AABBCollider::Contains(const Vector3& _point)
{
    return min_.x <= _point.x && _point.x <= max_.x &&
        min_.y <= _point.y && _point.y <= max_.y &&
        min_.z <= _point.z && _point.z <= max_.z;
}

Vector3 AABBCollider::GetClosestPoint(const Vector3& _point)
{
    return Vector3(
        std::clamp(_point.x, min_.x, max_.x),
        std::clamp(_point.y, min_.y, max_.y),
        std::clamp(_point.z, min_.z, max_.z)
    );
}

AABB AABBCollider::GetBounds() const
{
    // ワールド変換を取得
    const WorldTransform* transform = GetWorldTransform();

    // オフセットをワールド座標に変換
    Vector3 worldOffset = Transform(offset_, transform->quaternion_.ToMatrix());
    Vector3 center = transform->transform_ + worldOffset;

    // スケールを考慮したAABBの最小値と最大値
    Vector3 scaledMin = min_ * transform->scale_;
    Vector3 scaledMax = max_ * transform->scale_;

    return AABB(center + scaledMin, center + scaledMax);

}

void AABBCollider::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    Collider::ImGui();

    if (ImGui::DragFloat3("Min", &min_.x, 0.01f))
    {
        if (min_.x > max_.x)
            max_.x = min_.x;
        if (min_.y > max_.y)
            max_.y = min_.y;
        if (min_.z > max_.z)
            max_.z = min_.z;
    }
    if (ImGui::DragFloat3("Max", &max_.x, 0.01f))
    {
        if (max_.x < min_.x)
            min_.x = max_.x;
        if (max_.y < min_.y)
            min_.y = max_.y;
        if (max_.z < min_.z)
            min_.z = max_.z;
    }
    ImGui::PopID();
#endif // _DEBUG
}

AABBCollider::AABBCollider(bool _isTemporary) : Collider()
{
    SetBoundingBox(BoundingBox::AABB_3D);
}

OBBCollider::OBBCollider(const char* _name) : Collider()
{
    SetBoundingBox(BoundingBox::OBB_3D);
#ifdef _DEBUG
    name_ = ImGuiDebugManager::GetInstance()->AddColliderDebugWindow(_name, [&]() {ImGui(); });
#endif // _DEBUG
}

void OBBCollider::Draw()
{
    if (!isDraw_)
        return;

    std::vector<Vector3> corners = GetVertices();

    std::array<Vector3, 8> c;
    // 順番を入れ替えて配置
    c[0] = corners[6]; // 右上前 -> (+x, +y, +z)
    c[1] = corners[2]; // 右上後 -> (+x, +y, -z)
    c[2] = corners[5]; // 右下前 -> (+x, -y, +z)
    c[3] = corners[1]; // 右下後 -> (+x, -y, -z)
    c[4] = corners[7]; // 左上前 -> (-x, +y, +z)
    c[5] = corners[3]; // 左上後 -> (-x, +y, -z)
    c[6] = corners[4]; // 左下前 -> (-x, -y, +z)
    c[7] = corners[0]; // 左下後 -> (-x, -y, -z)

    // OBBを描画
    LineDrawer::GetInstance()->DrawOBB(c);
}

void OBBCollider::Load(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("halfExtents", &halfExtents_);
        jsonBinder_->RegisterVariable("localPivot", &localPivot_);
    }
}

void OBBCollider::Save(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("halfExtents", &halfExtents_);
        jsonBinder_->RegisterVariable("localPivot", &localPivot_);
    }
    jsonBinder_->Save();
}

void OBBCollider::SetHalfExtents(const Vector3& _halfExtents)
{
    halfExtents_ = _halfExtents;
    size_ = halfExtents_ * 2.0f; // サイズは半分の大きさの2倍
}

bool OBBCollider::Contains(const Vector3& _point)
{
    Vector3 center = GetCenter();
    WorldTransform transform = *GetWorldTransform();
    Matrix4x4 invRotMat = Inverse(transform.quaternion_.ToMatrix());

    Vector3 localPoint = Transform(_point - center, invRotMat);

    return std::abs(localPoint.x) <= halfExtents_.x &&
        std::abs(localPoint.y) <= halfExtents_.y &&
        std::abs(localPoint.z) <= halfExtents_.z;
}

Vector3 OBBCollider::GetClosestPoint(const Vector3& _point)
{
    Vector3 center = GetCenter();
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 rotMat = transform.quaternion_.ToMatrix();

    Matrix4x4 invRotMat = Inverse(rotMat);
    Vector3 localPoint = Transform(_point - center, invRotMat);

    // スケールを適用したhalfExtentsを計算
    Vector3 scaledHalfExtents = halfExtents_ * transform.scale_;

    // ローカル空間でクランプ（スケール適用後のhalfExtentsを使用）
    Vector3 localClosest(
        std::clamp(localPoint.x, -scaledHalfExtents.x, scaledHalfExtents.x),
        std::clamp(localPoint.y, -scaledHalfExtents.y, scaledHalfExtents.y),
        std::clamp(localPoint.z, -scaledHalfExtents.z, scaledHalfExtents.z)
    );

    // ワールド空間に戻す
    return center + Transform(localClosest, rotMat);
}

std::vector<Vector3> OBBCollider::GetVertices()
{
    std::vector<Vector3> corners(8);
    Vector3 center = GetCenter(); // ピボットではなく実際の中心を使用
    WorldTransform transform = *GetWorldTransform();
    Matrix4x4 rotMat = transform.quaternion_.ToMatrix();

    Vector3 scaledHalfExtents = halfExtents_ * transform.scale_;

    // ローカル空間での頂点（中心を原点とする）
    Vector3 localCorners[8] = {
        Vector3(-scaledHalfExtents.x, -scaledHalfExtents.y, -scaledHalfExtents.z), // 0: 左下後
        Vector3(scaledHalfExtents.x, -scaledHalfExtents.y, -scaledHalfExtents.z), // 1: 右下後
        Vector3(scaledHalfExtents.x,  scaledHalfExtents.y, -scaledHalfExtents.z), // 2: 右上後
        Vector3(-scaledHalfExtents.x,  scaledHalfExtents.y, -scaledHalfExtents.z), // 3: 左上後
        Vector3(-scaledHalfExtents.x, -scaledHalfExtents.y,  scaledHalfExtents.z), // 4: 左下前
        Vector3(scaledHalfExtents.x, -scaledHalfExtents.y,  scaledHalfExtents.z), // 5: 右下前
        Vector3(scaledHalfExtents.x,  scaledHalfExtents.y,  scaledHalfExtents.z), // 6: 右上前
        Vector3(-scaledHalfExtents.x,  scaledHalfExtents.y,  scaledHalfExtents.z)  // 7: 左上前
    };

    // ワールド空間に変換（回転して中心に配置）
    for (int i = 0; i < 8; i++) {
        corners[i] = center + Transform(localCorners[i], rotMat);
    }

    return corners;
}

Vector3 OBBCollider::GetCenter() const 
{
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 affine = MakeAffineMatrix(transform.scale_, transform.rotate_, offset_);

    Vector3 pivot = Transform(localPivot_, affine);

    return transform.transform_ + pivot;

}

AABB OBBCollider::GetBounds() const
{
    const WorldTransform* transform = GetWorldTransform(); // const版を呼び出し

    Vector3 center = GetCenter();
    Vector3 scaledHalfExtents = halfExtents_ * transform->scale_;

    Matrix4x4 rotMat = transform->quaternion_.ToMatrix();

    Vector3 axisX = Vector3(rotMat.m[0][0], rotMat.m[1][0], rotMat.m[2][0]);
    Vector3 axisY = Vector3(rotMat.m[0][1], rotMat.m[1][1], rotMat.m[2][1]);
    Vector3 axisZ = Vector3(rotMat.m[0][2], rotMat.m[1][2], rotMat.m[2][2]);

    Vector3 projection = Vector3(
        scaledHalfExtents.x * std::abs(axisX.x) +
        scaledHalfExtents.y * std::abs(axisY.x) +
        scaledHalfExtents.z * std::abs(axisZ.x),

        scaledHalfExtents.x * std::abs(axisX.y) +
        scaledHalfExtents.y * std::abs(axisY.y) +
        scaledHalfExtents.z * std::abs(axisZ.y),

        scaledHalfExtents.x * std::abs(axisX.z) +
        scaledHalfExtents.y * std::abs(axisY.z) +
        scaledHalfExtents.z * std::abs(axisZ.z)
    );

    return AABB(center - projection, center + projection);
}

void OBBCollider::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    Collider::ImGui();
    ImGui::DragFloat3("HalfExtents", &halfExtents_.x, 0.01f);
    ImGui::DragFloat3("LocalPivot", &localPivot_.x, 0.01f);
    ImGui::PopID();
#endif // _DEBUG
}

OBBCollider::OBBCollider(bool _isTemporary) : Collider()
{
    SetBoundingBox(BoundingBox::OBB_3D);
}

 CapsuleCollider::CapsuleCollider(const char* _name) : Collider()
{
    SetBoundingBox(BoundingBox::Capsule_3D);
#ifdef _DEBUG
    name_ = ImGuiDebugManager::GetInstance()->AddColliderDebugWindow(_name, [&]() {ImGui(); });
#endif // _DEBUG
}

void CapsuleCollider::Draw()
{
    if (!isDraw_)
        return;

    // カプセルの中心線の両端を取得
    Vector3 start, end;
    GetCapsuleSegment(start, end);

    // 方向ベクトルを取得
    Vector3 direction = (end - start).Normalize();

    // 中心線を描画
    LineDrawer::GetInstance()->RegisterPoint(start, end);

    // 法線と垂直な2つのベクトルを見つける
    Vector3 tangent1, tangent2;

    // 方向ベクトルが上向きベクトル(0,1,0)と近い場合は別の基準ベクトルを使用
    if (std::abs(direction.Dot(Vector3(0, 1, 0))) > 0.99f)
    {
        // X軸を基準にする
        tangent1 = Vector3(0, 0, 1);
    }
    else
    {
        // 上向きベクトルと方向の外積で最初の接ベクトルを得る
        tangent1 = direction.Cross(Vector3(0, 1, 0)).Normalize();
    }

    // 方向と最初の接ベクトルの外積で2つ目の接ベクトルを得る
    tangent2 = direction.Cross(tangent1).Normalize();

    // 円周上の点の数
    const int circleSegments = 16;
    const float kEvery = std::numbers::pi_v<float> *2.0f / circleSegments;

    // 胴体部分の円を描画する数
    const int lengthSegments = 4;

    // 胴体部分の円周上の点を記録する配列
    std::vector<std::vector<Vector3>> circlePoints(lengthSegments + 1);

    // 胴体部分の円を描画して、各円周上の点を記録
    for (int i = 0; i <= lengthSegments; i++)
    {
        float t = static_cast<float>(i) / lengthSegments;
        Vector3 center = start + (end - start) * t;

        // この円の点を保存するための配列
        circlePoints[i].resize(circleSegments);

        // 円を描画
        for (int j = 0; j < circleSegments; j++)
        {
            float rad = j * kEvery;
            float nextRad = ((j + 1) % circleSegments) * kEvery;

            // 現在の円周上の点
            Vector3 spos = center +
                tangent1 * std::cos(rad) * radius_ +
                tangent2 * std::sin(rad) * radius_;

            // 次の円周上の点
            Vector3 epos = center +
                tangent1 * std::cos(nextRad) * radius_ +
                tangent2 * std::sin(nextRad) * radius_;

            // 点を記録
            circlePoints[i][j] = spos;

            // 円の線分を登録
            LineDrawer::GetInstance()->RegisterPoint(spos, epos);
        }
    }

    // 胴体の縦線を描画
    for (int j = 0; j < circleSegments; j += 4) // 縦線の数を減らして見やすくする
    {
        for (int i = 0; i < lengthSegments; i++)
        {
            // 現在の円の点と次の円の対応する点を結ぶ
            LineDrawer::GetInstance()->RegisterPoint(circlePoints[i][j], circlePoints[i + 1][j]);
        }
    }

    // デフォルトの上向きベクトル
    Vector3 defaultUp(0, 1, 0);
    // 方向ベクトルを基準にした回転行列を作成
    Matrix4x4 rotMat = DirectionToDirection(defaultUp, direction);

    // 上半球を描画
    Matrix4x4 sphereMat = MakeIdentity4x4();
    // 半球の位置に移動
    sphereMat = MakeTranslateMatrix(end);
    // スケールと回転を適用
    sphereMat = MakeScaleMatrix(Vector3(radius_, radius_, radius_)) * rotMat * sphereMat;

    // 半球の描画
    LineDrawer::GetInstance()->DrawSphere(sphereMat);

    // 下半球を描画
    sphereMat = MakeIdentity4x4();
    sphereMat = MakeTranslateMatrix(start);
    sphereMat = MakeScaleMatrix(Vector3(radius_, radius_, radius_)) * rotMat * sphereMat;

    // 半球の描画
    LineDrawer::GetInstance()->DrawSphere(sphereMat);
}

void CapsuleCollider::Load(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("radius", &radius_);
        jsonBinder_->RegisterVariable("direction", &direction_);
        jsonBinder_->RegisterVariable("localPivot", &localPivot_);
        jsonBinder_->RegisterVariable("height", &height_);
    }
}

void CapsuleCollider::Save(const std::string& _name)
{
    if (InitJsonBinder(_name))
    {
        jsonBinder_->RegisterVariable("radius", &radius_);
        jsonBinder_->RegisterVariable("direction", &direction_);
        jsonBinder_->RegisterVariable("localPivot", &localPivot_);
        jsonBinder_->RegisterVariable("height", &height_);
    }

    jsonBinder_->Save();
}

void CapsuleCollider::SetRadius(float _radius)
{
    radius_ = _radius;
    size_ = GetCapsuleAABBSize();
}

void CapsuleCollider::SetHeight(float _height)
{
    height_ = _height;
    // サイズは半径の2倍と高さ
    size_ = GetCapsuleAABBSize();
}

bool CapsuleCollider::Contains(const Vector3& _point)
{
    Vector3 start, end;
    GetCapsuleSegment(start, end);

    // 点から中心線への最近接点を見つける
    Vector3 closest = ClosestPointOnSegment(_point, start, end);

    // 点と最近接点の距離が半径以下なら内部に含まれる
    float distance = Vector3(_point - closest).Length();
    return distance <= radius_;
}

Vector3 CapsuleCollider::GetClosestPoint(const Vector3& _point)
{
    Vector3 start, end;
    GetCapsuleSegment(start, end);

    // 点から中心線への最近接点を見つける
    Vector3 closest = ClosestPointOnSegment(_point, start, end);

    // 点と最近接点間のベクトル
    Vector3 toPoint = _point - closest;
    float distance = toPoint.Length();

    if (distance < 0.0001f) {
        // 点が中心線上にある場合、ランダムな方向に半径分移動
        // 簡単のため、上方向（または中心線に垂直な方向）を選択
        Vector3 worldUp = Vector3(0, 1, 0);
        Vector3 worldDirection = Transform(direction_, GetWorldTransform()->quaternion_.ToMatrix());

        // 方向ベクトルと上ベクトルが平行に近い場合は別の方向を選択
        if (std::abs(worldDirection.Dot(worldUp)) > 0.9f) {
            worldUp = Vector3(1, 0, 0); // X軸方向を使用
        }

        // 中心線に垂直なベクトルを計算
        Vector3 perpendicular = worldUp.Cross(worldDirection).Normalize();
        if (perpendicular.Length() < 0.0001f) {
            perpendicular = Vector3(0, 0, 1).Cross(worldDirection).Normalize();
        }

        return closest + perpendicular * radius_;
    }

    if (distance <= radius_) {
        // 点がカプセル内部にある場合
        return _point;
    }

    // 点がカプセル外部にある場合、最近接点から半径方向に移動
    return closest + toPoint.Normalize() * radius_;
}

Vector3 CapsuleCollider::GetCenter() const
{
    WorldTransform transform = *GetWorldTransform();

    Matrix4x4 affine = MakeAffineMatrix(transform.scale_, transform.rotate_, offset_);

    Vector3 pivot = Transform(localPivot_, affine);

    return transform.transform_ + pivot;
}

void CapsuleCollider::GetCapsuleSegment(Vector3& _start, Vector3& _end) const
{
    Vector3 center = GetCenter();

    // トランスフォームの回転を方向ベクトルに適用
    Vector3 worldDirection = Transform(direction_, GetWorldTransform()->quaternion_.ToMatrix());

    // カプセルの中央から両端点への距離
    // 注意: カプセルの高さは両端の半球を含む全長なので、
    // 中心線の長さは (height - 2*radius) になる
    float halfLineLength = (height_ - 2.0f * radius_) * 0.5f;

    // 負の方向への端点と正の方向への端点
    _start = center - worldDirection * halfLineLength;
    _end = center + worldDirection * halfLineLength;
}

Vector3 CapsuleCollider::ClosestPointOnSegment(const Vector3& _point, const Vector3& _start, const Vector3& _end) const
{
    Vector3 segment = _end - _start;
    float segmentLength = segment.Length();

    if (segmentLength < 0.0001f) {
        return _start; // 線分の長さがほぼ0の場合
    }

    Vector3 direction = segment / segmentLength;
    float projection = Vector3(_point - _start).Dot(direction);

    // 線分上の位置にクランプ
    projection = std::clamp(projection, 0.0f, segmentLength);

    return _start + direction * projection;
}

void CapsuleCollider::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    Collider::ImGui();
    ImGui::DragFloat("Radius", &radius_, 0.01f, 0.01f, 1000.0f);
    ImGui::DragFloat("Height", &height_, 0.01f, 0.01f, 1000.0f);
    if (ImGui::DragFloat3("Direction", &direction_.x, 0.01f))
    {
        direction_ = direction_.Normalize();
    }
    ImGui::DragFloat3("LocalPivot", &localPivot_.x, 0.01f);
    ImGui::PopID();
#endif // _DEBUG
}

Vector3 CapsuleCollider::GetCapsuleAABBSize()
{
    Vector3 halfVec = direction_ * (height_ * 0.5f);
    Vector3 p1 = GetCenter() - halfVec; // 一方の端点
    Vector3 p2 = GetCenter() + halfVec; // もう一方の端点

    // AABBの最小点と最大点を計算
    Vector3 min = Vector3::Min(p1, p2) - Vector3(radius_, radius_, radius_);
    Vector3 max = Vector3::Max(p1, p2) + Vector3(radius_, radius_, radius_);

    // AABBのサイズを計算
    return max - min;
}

CapsuleCollider::CapsuleCollider(bool _isTemporary) : Collider()
{
    SetBoundingBox(BoundingBox::Capsule_3D);
}

AABB CapsuleCollider::GetBounds() const
{
    const WorldTransform* transform = GetWorldTransform(); // const版を呼び出し

    // GetCapsuleSegmentがconst関数でない場合の回避策
    Vector3 start, end;
    const_cast<CapsuleCollider*>(this)->GetCapsuleSegment(start, end);

    float worldRadius = radius_ * (std::max)({ transform->scale_.x, transform->scale_.y, transform->scale_.z });

    Vector3 minPoint = Vector3::Min(start, end);
    Vector3 maxPoint = Vector3::Max(start, end);

    Vector3 radiusVec(worldRadius, worldRadius, worldRadius);

    return AABB(minPoint - radiusVec, maxPoint + radiusVec);
}

std::string ToString(BoundingBox _box)
{
    switch (_box)
    {
    case BoundingBox::Sphere_3D:
        return "Sphere";
    case BoundingBox::AABB_3D:
        return "AABB";
    case BoundingBox::OBB_3D:
        return "OBB";
    case BoundingBox::Capsule_3D:
        return "Capsule";
    default:
        return "NONE";
    }
}

bool AABB::Intersect(AABB _other) const 
{
    return  min.x <= _other.max.x && max.x >= _other.min.x &&
            min.y <= _other.max.y && max.y >= _other.min.y &&
            min.z <= _other.max.z && max.z >= _other.min.z;

}
