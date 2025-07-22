#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Features/Animation/Sequence/SequenceEvent.h>
#include <Features/TextRenderer/TextParam.h>

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


// TextParam
void to_json(json& _j, const TextParam& _v);
void from_json(const json& _j, TextParam& _v);