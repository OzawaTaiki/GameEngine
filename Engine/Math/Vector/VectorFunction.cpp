#include <Math/Vector/VectorFunction.h>
#include <cmath>
#include <assert.h>

static const int kColumnWidth = 60;

Vector3  Add(const Vector3& _v1, const Vector3& _v2)
{
	Vector3 result;
	result.x = _v1.x + _v2.x;
	result.y = _v1.y + _v2.y;
	result.z = _v1.z + _v2.z;

	return result;
}

Vector3  Subtract(const Vector3& _v1, const Vector3& _v2)
{
	Vector3 result;
	result.x = _v1.x - _v2.x;
	result.y = _v1.y - _v2.y;
	result.z = _v1.z - _v2.z;

	return result;
}

Vector3  Multiply(float _scalar, const Vector3& _v)
{
	Vector3 result;
	result.x = _v.x * _scalar;
	result.y = _v.y * _scalar;
	result.z = _v.z * _scalar;

	return result;
}

Vector3  Multiply(const Vector3& _v1, const Vector3& _v2)
{
	Vector3 result;
	result.x = _v1.x * _v2.x;
	result.y = _v1.y * _v2.y;
	result.z = _v1.z * _v2.z;

	return result;
}

float  Dot(const Vector3& _v1, const Vector3& _v2)
{
	float result = _v1.x * _v2.x + _v1.y * _v2.y + _v1.z * _v2.z;

	return result;
}

Vector3  Cross(const Vector3& _v1, const Vector3& _v2)
{
	Vector3 result;

	result.x = _v1.y * _v2.z - _v1.z * _v2.y;
	result.y = _v1.z * _v2.x - _v1.x * _v2.z;
	result.z = _v1.x * _v2.y - _v1.y * _v2.x;

	return result;
}

float  Length(const Vector3& _v)
{
	float result;
	result = std::sqrtf(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
	return result;
}

Vector3  Normalize(const Vector3& _v)
{
	Vector3 result;
	float _length = Length(_v);
	if (_length == 0)
		return { 0.0f };
	result.x = _v.x / _length;
	result.y = _v.y / _length;
	result.z = _v.z / _length;

	return result;
}



Vector3  Transform(const Vector3& _vector, const Matrix4x4& _matrix)
{
	Vector3 result;

	result.x = _vector.x * _matrix.m[0][0] + _vector.y * _matrix.m[1][0] + _vector.z * _matrix.m[2][0] + 1.0f * _matrix.m[3][0];
	result.y = _vector.x * _matrix.m[0][1] + _vector.y * _matrix.m[1][1] + _vector.z * _matrix.m[2][1] + 1.0f * _matrix.m[3][1];
	result.z = _vector.x * _matrix.m[0][2] + _vector.y * _matrix.m[1][2] + _vector.z * _matrix.m[2][2] + 1.0f * _matrix.m[3][2];
	float _w = _vector.x * _matrix.m[0][3] + _vector.y * _matrix.m[1][3] + _vector.z * _matrix.m[2][3] + 1.0f * _matrix.m[3][3];
	assert(_w != 0.0f);
	result.x /= _w;
	result.y /= _w;
	result.z /= _w;
	return result;
}

Vector3 TransformNormal(const Vector3& _v, const Matrix4x4& _m) {
	Vector3 result{
		_v.x * _m.m[0][0] + _v.y * _m.m[1][0] + _v.z * _m.m[2][0],
		_v.x * _m.m[0][1] + _v.y * _m.m[1][1] + _v.z * _m.m[2][1],
		_v.x * _m.m[0][2] + _v.y * _m.m[1][2] + _v.z * _m.m[2][2] };
	return result;
}
float Distance(const Vector3& v1, const Vector3& v2) {

	Vector3 diff = Subtract(v1, v2);
	float leng = Length(diff);

	return leng;
}
float DistanceXZ(const Vector3& v1, const Vector3& v2) {
	Vector3 diff = Subtract(Vector3{ v1.x, 0, v1.z }, Vector3{ v2.x, 0, v2.z });

	float leng = Length(diff);

	return leng;
};
