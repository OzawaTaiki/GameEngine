#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Features/Animation/Sequence/SequenceEvent.h>
#include <Features/TextRenderer/TextParam.h>
#include <Features/Model/Primitive/Creater/PrimitiveCreator.h>
#include <Features/UI/Collider/UIColliderSerializer.h>

#include <json.hpp>

using json = nlohmann::json;


// Vector2
void to_json(json& _j, const Vector2& _v);
void from_json(const json& _j, Vector2& _v);

// Vector3
void to_json(json& _j, const Vector3& _v);
void from_json(const json& _j, Vector3& _v);

// Vector4
void to_json(json& _j, const Vector4& _v);
void from_json(const json& _j, Vector4& _v);

// Quaternion
void to_json(json& _j, const Quaternion& _v);
void from_json(const json& _j, Quaternion& _v);


/// AnimationSequence.h

// ParameterValue
void to_json(json& _j, const ParameterValue& _v);
void from_json(const json& _j, ParameterValue& _v);

// SequenceEvent
void to_json(json& _j, const SequenceEvent::KeyFrame& _v);
void from_json(const json& _j, SequenceEvent::KeyFrame& _v);

// SequenceEvent
void to_json(json& _j, const SequenceEvent& _v);
void from_json(const json& _j, SequenceEvent& _v);



/// TextParam
void to_json(json& _j, const TextParam& _v);
void from_json(const json& _j, TextParam& _v);


/// PrimitiveCreater.h

// PrimitiveType
void to_json(json& _j, const PrimitiveType& _type);
void from_json(const json& _j, PrimitiveType& _type);

// PrimitiveSettings
void to_json(json& _j, const PrimitiveSettings& _settings);
void from_json(const json& _j, PrimitiveSettings& _settings);

// PrimitiveSettings
void to_json(json& _j, const PrimitiveSettings::PlaneData& _plane);
void from_json(const json& _j, PrimitiveSettings::PlaneData& _plane);

void to_json(json& _j, const PrimitiveSettings::TriangleData& _triangle);
void from_json(const json& _j, PrimitiveSettings::TriangleData& _triangle);

void to_json(json& _j, const PrimitiveSettings::CylinderData& _cylinder);
void from_json(const json& _j, PrimitiveSettings::CylinderData& _cylinder);

void to_json(json& _j, const PrimitiveSettings::RingData& _ring);
void from_json(const json& _j, PrimitiveSettings::RingData& _ring);

void to_json(json& _j, const PrimitiveSettings::CubeData& _cube);
void from_json(const json& _j, PrimitiveSettings::CubeData& _cube);

// CreatedPrimitive
void to_json(json& _j, const CreatedPrimitive& _data);
void from_json(const json& _j, CreatedPrimitive& _data);


/// UIColliderSerializer.h

// ColliderType
void to_json(json& _j, const ColliderType& _type);
void from_json(const json& _j, ColliderType& _type);

// IUICollider::TransformMode
void to_json(json& _j, const IUICollider::TransformMode& _mode);
void from_json(const json& _j, IUICollider::TransformMode& _mode);

// UIColliderData
void to_json(json& _j, const UIColliderData& _data);
void from_json(const json& _j, UIColliderData& _data);
