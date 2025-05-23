#include "SpiralHashGird.h"

#include <set>

void SpiralHashGrid::AddCollider(Collider* _collider)
{
    std::array<int32_t, 4> cellIndices = GetCellIndices(_collider);

    for (int32_t x = cellIndices[0]; x <= cellIndices[1]; ++x)
    {
        for (int32_t y = cellIndices[2]; y <= cellIndices[3]; ++y)
        {
            uint64_t hashKey = GetHashKey(x, y);
            grid_[hashKey].push_back(_collider);
        }
    }
}

std::vector<Collider*> SpiralHashGrid::CheckCollision(Collider* _col) const
{
    std::set<Collider*> result;

    AABB bounds = _col->GetBounds();

    std::array<int32_t, 4> cellIndices = GetCellIndices(_col);

    for (int32_t x = cellIndices[0]; x <= cellIndices[1]; ++x)
    {
        for (int32_t y = cellIndices[2]; y <= cellIndices[3]; ++y)
        {
            uint64_t hashKey = GetHashKey(x, y);
            auto it = grid_.find(hashKey);
            if (it != grid_.end())
            {
                for (Collider* collider : it->second)
                {
                    // 同じコライダーは除外
                    if (collider != _col)
                    {
                        // 衝突判定を行う
                        if (collider->GetBounds().Intersect(bounds))
                        {
                            result.insert(collider);
                        }
                    }
                }
            }
        }
    }

    return std::vector<Collider*>(result.begin(), result.end());
}

uint64_t SpiralHashGrid::GetHashKey(const Vector2& _position) const
{
    int32_t x = static_cast<int32_t>(_position.x / cellSize_);
    int32_t y = static_cast<int32_t>(_position.y / cellSize_);

    return GetHashKey(x, y);

}

uint64_t SpiralHashGrid::GetHashKey(int32_t _x, int32_t _y) const
{
    // 負の値も正しく処理するためのオフセット
    uint32_t ux = static_cast<uint32_t>(_x);
    uint32_t uy = static_cast<uint32_t>(_y);

    return (static_cast<uint64_t>(ux) << 32) | static_cast<uint64_t>(uy);
}

std::array<int32_t, 4> SpiralHashGrid::GetCellIndices(Collider* _col) const
{
    Vector3 position3 = _col->GetWorldTransform()->GetWorldPosition();
    Vector2 center = Vector2(position3.x, position3.z);
    Vector3 size = _col->GetSize();

    Vector2 min = center - Vector2(size.x, size.z) * 0.5f;
    Vector2 max = center + Vector2(size.x, size.z) * 0.5f;

    int32_t minX = static_cast<int32_t>(min.x / cellSize_);
    int32_t minY = static_cast<int32_t>(min.y / cellSize_);
    int32_t maxX = static_cast<int32_t>(max.x / cellSize_);
    int32_t maxY = static_cast<int32_t>(max.y / cellSize_);

    return { minX, maxX,minY, maxY };

}
