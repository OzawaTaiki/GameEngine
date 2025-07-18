#include "QuadTree.h"

#include <Debug/Debug.h>
#include <Math/Vector/VectorFunction.h>

#include <iostream>
#include <cmath>
#include <list>

QuadTree::~QuadTree()
{
    for (auto& cell : cells_)
    {
        delete cell;
        cell = nullptr;
    }
    cells_.clear();
}

void QuadTree::Initialize(const Vector2& _rootSize, int32_t _level, const Vector2& _leftBottom)
{
    if (_rootSize.x <= 0 || _rootSize.y <= 0 || _level > kMaxLevel_)
        return;

    rootSize_ = _rootSize;
    level_ = _level;
    leftBottom_ = _leftBottom;

    quadNodesPerLevel_[0] = 1;
    for (size_t i = 1; i <= kMaxLevel_; ++i)
    {
        quadNodesPerLevel_[i] = quadNodesPerLevel_[i - 1] * 4;
    }

    cellCount_ = (quadNodesPerLevel_[level_ + 1] - 1) / 3;
    cells_.resize(cellCount_);
    cells_[0] = new Cell();

    minSpaceSize_ = rootSize_ / std::powf(2.0f, static_cast<float>(level_));

    std::cout << "QuadTree initialized with root size: (" << rootSize_.x << ", " << rootSize_.y << ")\n";
    std::cout << "Level: " << level_ << "\n";
    std::cout << "Cell count: " << cellCount_ << "\n";
    std::cout << "Minimum space size: (" << minSpaceSize_.x << ", " << minSpaceSize_.y << ")\n";
    std::cout << "Quad nodes per level: ";
    for (const auto& count : quadNodesPerLevel_)
    {
        std::cout << count << " ";
    }
    std::cout << "\n----------------------------------------\n";
}

void QuadTree::RegisterObj(Collider* _obj)
{
    if (!_obj) return;

    auto oft = std::make_shared<ObjectForTree>();
    oft->SetData(_obj);

    Vector2 pos(_obj->GetWorldTransform()->GetWorldPosition().x, _obj->GetWorldTransform()->GetWorldPosition().z);
    Vector3 offset = _obj->GetOffset();
    Vector3 worldOffset = Transform(offset, _obj->GetWorldTransform()->quaternion_.ToMatrix());
    pos += Vector2(worldOffset.x, worldOffset.z);
    Vector2 size(_obj->GetSize().x, _obj->GetSize().z);

    MortonResult result = CalculateObjectMortonNumberAndLevel(pos, size);
    uint32_t belongingSpaceIndex = CalculateLinearIndexFromLevelAndNumber(result);

    if (belongingSpaceIndex < cellCount_)
    {
        if (!cells_[belongingSpaceIndex])
        {
            CreateCell(belongingSpaceIndex);
        }
        cells_[belongingSpaceIndex]->RegisterData(oft);
    }

    std::cout << "belongingSpace Level: " << result.level << "\n";
    std::cout << "mortonNumber: " << result.mortonNumber << "\n";
    std::cout << "registed index: " << belongingSpaceIndex << "\n";
    std::cout << "----------------------------------------\n";
}

void QuadTree::GetCollisionPair(uint32_t _index, std::vector<std::pair<Collider*, Collider*>>& _pair, std::list<Collider*>& _stac)
{
    auto cell = cells_[_index];
    auto data = cell->GetFirstData();

    while (data)
    {
        auto nextData = data->GetNextData();
        while (nextData)
        {
            _pair.emplace_back(data->GetData(), nextData->GetData());
            nextData = nextData->GetNextData();
        }

        for (auto& collider : _stac)
        {
            _pair.emplace_back(data->GetData(), collider);
        }

        data = data->GetNextData();
    }

    bool isChild = false;
    uint32_t objCount = 0;

    for (uint32_t i = 0; i < 4; ++i)
    {
        uint32_t childIndex = _index * 4 + 1 + i;
        if (childIndex < cellCount_ && cells_[childIndex])
        {
            if (!isChild)
            {
                auto data = cell->GetFirstData();
                while (data)
                {
                    _stac.push_back(data->GetData());
                    ++objCount;
                    data = data->GetNextData();
                }
            }
            isChild = true;
            GetCollisionPair(childIndex, _pair, _stac);
        }
    }

    if (isChild)
    {
        for (uint32_t i = 0; i < objCount; ++i)
        {
            _stac.pop_back();
        }
    }
}

void QuadTree::Reset()
{
    for (auto& cell : cells_)
    {
        if (cell)
        {
            cell->Reset();
        }
    }
}

int32_t QuadTree::ConvertPointToMortonCode(const Vector2& _pos) const
{
    int32_t index = 0;
    int32_t x = static_cast<int32_t>((_pos.x - leftBottom_.x) / minSpaceSize_.x);
    int32_t y = static_cast<int32_t>((_pos.y -leftBottom_.y) / minSpaceSize_.y);

    for (int32_t i = 0; i < level_; ++i)
    {
        index |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
    }

    return index;
}

MortonResult QuadTree::CalculateObjectMortonNumberAndLevel(const Vector2& _pos, const Vector2& _size)
{
    Vector2 halfSize = _size / 2.0f;
    int32_t lt_index = ConvertPointToMortonCode(_pos - halfSize);
    int32_t rb_index = ConvertPointToMortonCode(_pos + halfSize);

    std::cout << "Left Top Index: " << lt_index << "\n";
    std::cout << "Right Bottom Index: " << rb_index << "\n";

    if (lt_index < 0 || rb_index < 0)
    {
        Debug::Log(std::format("Invalid Morton code: lt_index = {}, rb_index = {}\n", lt_index, rb_index));
        Debug::Log(std::format("Object Pos:x_{},y_{}", _pos.x, _pos.y));
        Debug::Log(std::format("Object Size:x_{},y_{}", _size.x, _size.y));
        return { 0, 0 };
    }

    int32_t xorResult = lt_index ^ rb_index;
    if (xorResult == 0)
        return { lt_index , level_ };

    int32_t shiftCount = 0;
    for (int i = 30; i >= 0; i -= 2)
    {
        int32_t bitPair = (xorResult >> i) & 0x3;
        if (bitPair != 0)
        {
            shiftCount = i + 2;
            break;
        }
    }

    int32_t levelDiff = shiftCount / 2;
    int32_t objectLevel = level_ - levelDiff;
    int32_t belongingNumber = rb_index >> shiftCount;

    return { belongingNumber, objectLevel };
}

uint32_t QuadTree::CalculateLinearIndexFromLevelAndNumber(MortonResult _result)
{
    float pow = std::powf(4.0f, static_cast<float>(_result.level));
    return (static_cast<uint32_t>(pow) - 1) / 3 + _result.mortonNumber;
}

void QuadTree::CreateCell(uint32_t _index)
{
    if (_index >= cellCount_) return;

    while (cells_[_index] == nullptr)
    {
        cells_[_index] = new Cell();
        _index = (_index - 1) >> 2;
        if (_index >= cellCount_) break;
    }
}
