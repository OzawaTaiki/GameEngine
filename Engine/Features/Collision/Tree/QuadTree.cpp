#include "QuadTree.h"
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

void QuadTree::Initialize(const Vector2& _rootSize, uint32_t _level)
{
    if (_rootSize.x <= 0 || _rootSize.y <= 0 || _level > kMaxLevel_)
        return;

    rootSize_ = _rootSize;
    level_ = _level;

    quadNodesPerLevel_[0] = 1;
    for (uint32_t i = 1; i <= kMaxLevel_; ++i)
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

    Vector2 pos(_obj->GetWorldTransform()->GetWorldPosition().x, _obj->GetWorldTransform()->GetWorldPosition().y);
    Vector2 size(_obj->GetSize().x, _obj->GetSize().y);

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

uint32_t QuadTree::ConvertPointToMortonCode(const Vector2& _pos) const
{
    uint32_t index = 0;
    uint32_t x = static_cast<uint32_t>(_pos.x / minSpaceSize_.x);
    uint32_t y = static_cast<uint32_t>(_pos.y / minSpaceSize_.y);

    for (uint32_t i = 0; i < level_; ++i)
    {
        index |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
    }

    return index;
}

MortonResult QuadTree::CalculateObjectMortonNumberAndLevel(const Vector2& _pos, const Vector2& _size)
{
    Vector2 halfSize = _size / 2.0f;
    uint32_t lt_index = ConvertPointToMortonCode(_pos - halfSize);
    uint32_t rb_index = ConvertPointToMortonCode(_pos + halfSize);

    std::cout << "Left Top Index: " << lt_index << "\n";
    std::cout << "Right Bottom Index: " << rb_index << "\n";

    uint32_t xorResult = lt_index ^ rb_index;
    if (xorResult == 0)
        return { lt_index , level_ };

    uint32_t shiftCount = 0;
    for (int i = 30; i >= 0; i -= 2)
    {
        uint32_t bitPair = (xorResult >> i) & 0x3;
        if (bitPair != 0)
        {
            shiftCount = i + 2;
            break;
        }
    }

    uint32_t levelDiff = shiftCount / 2;
    uint32_t objectLevel = level_ - levelDiff;
    uint32_t belongingNumber = rb_index >> shiftCount;

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
