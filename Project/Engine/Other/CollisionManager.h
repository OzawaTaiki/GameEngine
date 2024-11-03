#pragma once

#include "Vector3.h"

#include <list>
#include <unordered_map>
#include <string>


class Collider;
class CollisionManager {
public:
	static CollisionManager* GetInstance();
	void SetCollider(Collider* _collider);
	void ListReset() { colliders_.clear(); };

	void CheckAllCollisions();

	uint32_t GetAtttibute(const std::string& _atrribute);
	uint32_t GetMask(const std::string& _atrribute);

private:
	void CheckCollisionPair(Collider* _colliderA, Collider* _colliderB);

	std::list<Collider*> colliders_;
	std::unordered_map<std::string, uint32_t> map_;

    struct Sphere
    {
        Vector3 center; //中心点
        float radius;   //半径
    };
    struct AABB
    {
        Vector3 center;
        Vector3 min;
        Vector3 max;

    };
    struct OBB
    {
        Vector3 center;				//中心座標
        Vector3 orientations[3];	//座標軸 正規化・直行必須
        Vector3 size;				//サイズ
        Vector3 rotate;				//回転角

    public:
        void CalculateOrientations();
        /// <summary>
        /// 頂点の計算
        /// </summary>
        /// <param name="vertices">計算結果を格納するVecter3配列</param>
        void CaluculateVertices(Vector3* vertices)const;
    };

    bool IsCollision(const Sphere& _s1, const Sphere& _s2);
    bool IsCollision(const AABB& _a, const Sphere& _s);
    bool IsCollision(const Sphere& _s, const AABB& _a);
    bool IsCollision(const OBB& _obb, const Sphere& _sphere);
    bool IsCollision(const Sphere& _sphere, const OBB& _obb);
    bool IsCollision(const AABB& _aabb, const OBB& _obb);
    bool IsCollision(const OBB& _obb, const AABB& _aabb);  
    bool IsCollision(const OBB& _obb1, const OBB& _obb2);
    void CalculateProjectionRange(const OBB& _obb, const Vector3& _axis, float& _min, float& _max);

    OBB ConvertToAABBToOBB(const AABB& _aabb);

};