#pragma once

#include <Math/Vector/Vector2.h>
#include <Features/Collision/Collider/Collider.h>
#include <vector>
#include <array>
#include <memory>
#include "Cell.h"

struct MortonResult
{
    int32_t mortonNumber;
    uint32_t level;
};

class QuadTree
{
public:
    QuadTree() = default;
    ~QuadTree();

    void Initialize(const Vector2& _rootSize, uint32_t _level,const Vector2& _leftTop);
    void RegisterObj(Collider* _obj);
    void GetCollisionPair(uint32_t _index, std::vector<std::pair<Collider*, Collider*>>& _pair, std::list<Collider*>& _stac);

    std::vector<Cell*> GetCells() { return cells_; }

private:
    int32_t ConvertPointToMortonCode(const Vector2& _pos) const;
    MortonResult CalculateObjectMortonNumberAndLevel(const Vector2& _pos, const Vector2& _size);
    uint32_t CalculateLinearIndexFromLevelAndNumber(MortonResult _result);
    void CreateCell(uint32_t _index);

    static constexpr size_t kMaxLevel_ = 9;

    Vector2 rootSize_ = { 100.0f, 100.0f };
    Vector2 minSpaceSize_ = { 1.0f, 1.0f };
    Vector2 leftBottom_ = { 0.0f, 0.0f };
    std::array<uint32_t, kMaxLevel_ + 1> quadNodesPerLevel_{};
    uint32_t level_ = 3;
    std::vector<Cell*> cells_;
    uint32_t cellCount_ = 0;
};
