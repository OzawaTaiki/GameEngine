#include "UICollisionManager.h"
#include <algorithm>

UICollisionManager* UICollisionManager::GetInstance()
{
    static UICollisionManager instance;
    return &instance;
}

void UICollisionManager::RegisterElement(IUICollider* _collider, uint16_t _order)
{
    colliders_.emplace_back(_order, _collider);
    needsSort_ = true;
}

void UICollisionManager::UnregisterElement(IUICollider* _collider)
{
    colliders_.remove_if([_collider](const auto& pair)
                         {
                             return pair.second == _collider;
                         });
}

void UICollisionManager::CheckCollision(const Vector2& _point)
{
    SortCollidersIfNeeded();

    std::for_each(colliders_.begin(), colliders_.end(), [](auto& pair)
                  {
                      pair.second->SetHit(false);
                  });

    for (auto& [order, collider] : colliders_)
    {
        if (collider->IsPointInside(_point))
        {
            collider->SetHit(true);
            break; // 最初に衝突したコライダーで終了
        }
    }

    colliders_.clear();
}

void UICollisionManager::SortCollidersIfNeeded()
{
    if (!needsSort_)
        return;

    needsSort_ = false;

    // コライダーをオーダー順にソート (降順)
    colliders_.sort([](const std::pair<uint16_t, IUICollider*>& a, const std::pair<uint16_t, IUICollider*>& b)
                    {
                        return a.first > b.first;
                    });

}
