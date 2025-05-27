#pragma once

#include <Math/Vector/Vector2.h>
#include <Features/Collision/Collider/Collider.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

class SpiralHashGrid
{
public:
    SpiralHashGrid(float _cellSize) : cellSize_(_cellSize) {};
    ~SpiralHashGrid() = default;

    void Clear() { grid_.clear(); }
    void Reset() { Clear(); }

    void AddCollider(Collider* _collider);
    void RemoveCollider(Collider* _collider);

    std::vector<Collider*> CheckCollision(Collider* _col) const;

private:

    uint64_t GetHashKey(const Vector2& _position) const;
    uint64_t GetHashKey(int32_t _x, int32_t _y) const;

    std::array<int32_t, 4> GetCellIndices(Collider* _col) const;
    std::array<int32_t, 4> GetCellIndices(const AABB& _aabb) const;

private:

    float cellSize_;
    std::unordered_map<uint64_t, std::vector<Collider*>> grid_;


};