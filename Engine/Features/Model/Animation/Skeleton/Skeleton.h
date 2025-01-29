#pragma once

#include <Features/Model/Animation/Joint/Joint.h>
#include <vector>
#include <map>
#include <string>

class Node;
class Skeleton
{
public:

    Skeleton() = default;
    ~Skeleton() = default;

    void Initialize();
    void Update();
    void Draw(const Matrix4x4& _wMat);

    void CreateSkeleton(const Node& _node);
    std::vector<Joint>& GetJoints() { return joints_; }
    std::map<std::string, int32_t>& GetJointMap() { return jointMap_; }
    Matrix4x4 GetSkeletonSpaceMatrix(uint32_t _index = 0)const { return joints_[_index].GetSkeletonSpaceMatrix(); }
private:
    int32_t rootIndex_ = 0;
    std::map<std::string, int32_t> jointMap_ = {};
    std::vector<Joint> joints_ = {};

};
