#include "Node.h"
#include "MatrixFunction.h"
#include <assimp/scene.h>

void Node::ReadNode(const aiNode* _node)
{

    aiVector3D scale, translation;
    aiQuaternion rotation;
    _node->mTransformation.Decompose(scale, rotation, translation);

    transform_.scale = Vector3(scale.x, scale.y, scale.z);
    transform_.translate = Vector3(-translation.x, translation.y, translation.z);
    transform_.rotation = Quaternion(rotation.x, -rotation.y, -rotation.z, rotation.w).Normalize();
    localMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translate);


    name_ = _node->mName.C_Str();
    children_.resize(_node->mNumChildren);
    for (uint32_t i = 0; i < _node->mNumChildren; ++i)
    {
        children_[i].ReadNode(_node->mChildren[i]);
    }
}

