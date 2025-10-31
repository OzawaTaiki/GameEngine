#pragma once

#include <Features/UI/Collider/Interface/IUICollider.h>

class UICollisionManager
{
public:

    static UICollisionManager* GetInstance();

    void RegisterElement(IUICollider* _collider, uint16_t _order = 0);
    void UnregisterElement(IUICollider* _collider);

    void CheckCollision(const Vector2& _point);

private:

    void SortCollidersIfNeeded();

private:

    std::list < std::pair<uint16_t, IUICollider*>> colliders_;

    bool needsSort_ = false;

private:
    UICollisionManager() = default;
    ~UICollisionManager() = default;
    UICollisionManager(const UICollisionManager&) = delete;
    UICollisionManager& operator=(const UICollisionManager&) = delete;

};
