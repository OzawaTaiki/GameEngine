#include "CollisionLayer.h"

#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>

CollisionLayer::CollisionLayer()
    : layer_(0)
    , layerMask_(0)
{
}

void CollisionLayer::SetLayer(const std::string& _layer)
{
    layer_ |= CollisionLayerManager::GetInstance()->GetLayer(_layer);
}

void CollisionLayer::SetLayerMask(const std::string& _layer)
{
    layerMask_ |= ~CollisionLayerManager::GetInstance()->GetLayer(_layer);
}
