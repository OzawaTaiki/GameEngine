#define NOMINMAX
#include "MyLib.h"
#include <algorithm>
#include <numbers>
#include <limits>

void DrawGrid(const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix)
{
	const float kGridHalfWidth = 2.0f;                                          // Gridの半分の幅
	const uint32_t kSubdivision = 10;                                           // 分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);     // １つ分の長さ

	// 奥から手前への線を引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex)
	{
		// 上の情報を使ってワールド座標系状の始点と終点を求める
		float x = kGridHalfWidth - xIndex * kGridEvery;
		Vector3 startPos = { -kGridHalfWidth,0,x };
		Vector3 endPos = { kGridHalfWidth,0,x };

		// スクリーン座標系まで変換をかける
		Vector3 temp = Transform(startPos, _viewProjectionMatrix);
		startPos = Transform(temp, _viewportMatrix);

		temp = Transform(endPos, _viewProjectionMatrix);
		endPos = Transform(temp, _viewportMatrix);

		// 変換した座標を使って表示。
		////Novice::DrawLine((int)startPos.x, (int)startPos.y, (int)endPos.x, (int)endPos.y, xIndex == (kSubdivision / 2) ? 0xff : 0xaaaaaaff);
	}

	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex)
	{
		// 上の情報を使ってワールド座標系状の始点と終点を求める
		float z = kGridHalfWidth - zIndex * kGridEvery;
		Vector3 startPos = { z,0,-kGridHalfWidth };
		Vector3 endPos = { z,0,kGridHalfWidth };

		// スクリーン座標系まで変換をかける
		Vector3 temp = Transform(startPos, _viewProjectionMatrix);
		startPos = Transform(temp, _viewportMatrix);


		temp = Transform(endPos, _viewProjectionMatrix);
		endPos = Transform(temp, _viewportMatrix);

		// 変換した座標を使って表示。
		//Novice::DrawLine((int)startPos.x, (int)startPos.y, (int)endPos.x, (int)endPos.y, zIndex == (kSubdivision / 2) ? 0xff : 0xaaaaaaff);
	}
}


void DrawSphere(const Sphere& _sphere, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	const uint32_t kSubdivision = 10;                                   // 分割数
	const float kLatEvery = (float)std::numbers::pi_v<float> / (float)kSubdivision;          // 緯度分割１つ分の角度
	const float kLonEvery = (float)std::numbers::pi_v<float> * 2.0 / (float)kSubdivision;    // 経度分割１つ分の角度

	//緯度の方向に分割   -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++)
	{
		float lat = -(float)std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;         // 現在の緯度

		// 経度の方向に分割   0 ~ π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++)
		{
			float lon = lonIndex * kLonEvery;                           // 現在の経度
			// world座標系でのa,b,cを求める
			Vector3 point[3] = {
				{
					std::cosf(lat) * std::cosf(lon),
					std::sinf(lat),
					std::cosf(lat) * std::sinf(lon)
				},
				{
					std::cosf(lat + kLatEvery) * std::cosf(lon),
					std::sinf(lat + kLatEvery),
					std::cosf(lat + kLatEvery) * std::sinf(lon)
				},
				{
					std::cosf(lat) * std::cosf(lon + kLonEvery),
					std::sinf(lat),
					std::cosf(lat) * std::sinf(lon + kLonEvery)
				}
			};

			for (int i = 0; i < 3; i++)
				point[i] = Add(_sphere.center, Multiply(_sphere.radius, point[i]));

			// a,b,cをScreen座標系まで変換
			Vector3 drawPoint[3];
			for (int i = 0; i < 3; i++)
			{
				Vector3 temp = Transform(point[i], _viewProjectionMatrix);
				drawPoint[i] = Transform(temp, _viewportMatrix);
			}

			// ab,acで線を引く
			//Novice::DrawLine((int)drawPoint[0].x, (int)drawPoint[0].y, (int)drawPoint[1].x, (int)drawPoint[1].y, _color);
			//Novice::DrawLine((int)drawPoint[0].x, (int)drawPoint[0].y, (int)drawPoint[2].x, (int)drawPoint[2].y, _color);

		}
	}
}

void Drawline(const Vector3& _origin, const Vector3& _diff, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 start = _origin;
	Vector3 end = Add(_origin, _diff);

	Vector3 temp = Transform(start, _viewProjectionMatrix);
	start = Transform(temp, _viewportMatrix);


	temp = Transform(end, _viewProjectionMatrix);
	end = Transform(temp, _viewportMatrix);

	//Novice::DrawLine((int)start.x, (int)start.y, (int)end.x, (int)end.y, _color);

}

void Drawline_se(const Vector3& _start, const Vector3& _end, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 start = _start;
	Vector3 end = _end;

	Vector3 temp = Transform(start, _viewProjectionMatrix);
	start = Transform(temp, _viewportMatrix);


	temp = Transform(end, _viewProjectionMatrix);
	end = Transform(temp, _viewportMatrix);

	//Novice::DrawLine((int)start.x, (int)start.y, (int)end.x, (int)end.y, _color);

}

void DrawPlane(const Plane& _plane, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 center = _plane.normal * _plane.distance; // 1
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(_plane.normal)); // 2
	perpendiculars[1] = { -perpendiculars[0].x, -perpendiculars[0].y, -perpendiculars[0].z }; // 3
	perpendiculars[2] = Cross(_plane.normal, perpendiculars[0]); // 4
	perpendiculars[3] = { -perpendiculars[2].x, -perpendiculars[2].y, -perpendiculars[2].z }; // 5
	// 6
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(2.0f, perpendiculars[index]);
		Vector3 point = center + extend;
		points[index] = Transform(Transform(point, _viewProjectionMatrix), _viewportMatrix);
	}
	//Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, _color);
	//Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[3].x, (int)points[3].y, _color);
	//Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[2].x, (int)points[2].y, _color);
	//Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, _color);

}

void DrawTriangle(const Triangle& _triangle, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 vertices[3];

	for (int i = 0; i < 3; i++)
	{
		vertices[i] = Transform(Transform(_triangle.vertices[i], _viewProjectionMatrix), _viewportMatrix);
	}
	/*Novice::DrawTriangle(int(vertices[0].x), int(vertices[0].y),
	int(vertices[1].x), int(vertices[1].y),
		int(vertices[2].x), int(vertices[2].y),
		_color, kFillModeWireFrame);*/
}


void DrawAABB(const AABB& _aabb, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 vertices[8];

	vertices[0] = { _aabb.min.x,_aabb.min.y ,_aabb.min.z };
	vertices[1] = { _aabb.max.x,_aabb.min.y ,_aabb.min.z };
	vertices[2] = { _aabb.min.x,_aabb.max.y ,_aabb.min.z };
	vertices[3] = { _aabb.max.x,_aabb.max.y ,_aabb.min.z };
	vertices[4] = { _aabb.min.x,_aabb.min.y ,_aabb.max.z };
	vertices[5] = { _aabb.max.x,_aabb.min.y ,_aabb.max.z };
	vertices[6] = { _aabb.min.x,_aabb.max.y ,_aabb.max.z };
	vertices[7] = { _aabb.max.x,_aabb.max.y ,_aabb.max.z };

	for (Vector3& v : vertices)
	{
		v = Transform(Transform(v, _viewProjectionMatrix), _viewportMatrix);
	}

	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[1].x, (int)vertices[1].y, _color);
	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[2].x, (int)vertices[2].y, _color);
	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[4].x, (int)vertices[4].y, _color);
	//Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[3].x, (int)vertices[3].y, _color);
	//Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[5].x, (int)vertices[5].y, _color);
	//Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[6].x, (int)vertices[6].y, _color);
	//Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[3].x, (int)vertices[3].y, _color);
	//Novice::DrawLine((int)vertices[3].x, (int)vertices[3].y, (int)vertices[7].x, (int)vertices[7].y, _color);
	//Novice::DrawLine((int)vertices[4].x, (int)vertices[4].y, (int)vertices[5].x, (int)vertices[5].y, _color);
	//Novice::DrawLine((int)vertices[4].x, (int)vertices[4].y, (int)vertices[6].x, (int)vertices[6].y, _color);
	//Novice::DrawLine((int)vertices[5].x, (int)vertices[5].y, (int)vertices[7].x, (int)vertices[7].y, _color);
	//Novice::DrawLine((int)vertices[6].x, (int)vertices[6].y, (int)vertices[7].x, (int)vertices[7].y, _color);

}

void DrawOBB(const OBB& _obb, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color)
{
	Vector3 vertices[8];

	_obb.CaluculateVertices(vertices);

	for (Vector3& v : vertices)
	{
		v = Transform(Transform(v, _viewProjectionMatrix), _viewportMatrix);
	}

	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[1].x, (int)vertices[1].y, _color);
	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[2].x, (int)vertices[2].y, _color);
	//Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[4].x, (int)vertices[4].y, _color);
	//Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[3].x, (int)vertices[3].y, _color);
	//Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[5].x, (int)vertices[5].y, _color);
	//Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[6].x, (int)vertices[6].y, _color);
	//Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[3].x, (int)vertices[3].y, _color);
	//Novice::DrawLine((int)vertices[3].x, (int)vertices[3].y, (int)vertices[7].x, (int)vertices[7].y, _color);
	//Novice::DrawLine((int)vertices[4].x, (int)vertices[4].y, (int)vertices[5].x, (int)vertices[5].y, _color);
	//Novice::DrawLine((int)vertices[4].x, (int)vertices[4].y, (int)vertices[6].x, (int)vertices[6].y, _color);
	//Novice::DrawLine((int)vertices[5].x, (int)vertices[5].y, (int)vertices[7].x, (int)vertices[7].y, _color);
	//Novice::DrawLine((int)vertices[6].x, (int)vertices[6].y, (int)vertices[7].x, (int)vertices[7].y, _color);
}

void DrawBezier(const Bezier& _bezier, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color, bool _isDrawPoint)
{
	float subDivision = 32;//ポイント間の分割数

	for (int i = 0; i < subDivision; i++)
	{
		float t1, t2;
		t1 = float(i) / subDivision;
		t2 = float(i + 1) / subDivision;

		Vector3 point[2];
		point[0] = CalculatePointBezier(_bezier, t1);
		point[1] = CalculatePointBezier(_bezier, t2);


		Drawline_se(point[0], point[1], _viewProjectionMatrix, _viewportMatrix, _color);
	}
	if (_isDrawPoint)
	{
		Sphere point[3];
		for (int i = 0; i < 3; i++)
		{
			point[i].center = _bezier.point[i];
			point[i].radius = 0.01f;
			//DrawSphere(point[i], _viewProjectionMatrix, _viewportMatrix, BLACK);
		}
	}
}

void DrawCatmullRom(const Vector3& _cPoint0, const Vector3& _cPoint1, const Vector3& _cPoint2, const Vector3& _cPoint3, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix, uint32_t _color, bool _isDrawPoint)
{
	float subDivision = 32;//ポイント間の分割数

	for (int i = 0; i < subDivision; i++)
	{
		float t1, t2;
		t1 = float(i) / subDivision;
		t2 = float(i + 1) / subDivision;

		Vector3 point[3][2];
		point[0][0] = CalculatePointCatmullRom(_cPoint0, _cPoint0, _cPoint1, _cPoint2, t1);
		point[0][1] = CalculatePointCatmullRom(_cPoint0, _cPoint0, _cPoint1, _cPoint2, t2);

		point[1][0] = CalculatePointCatmullRom(_cPoint0, _cPoint1, _cPoint2, _cPoint3, t1);
		point[1][1] = CalculatePointCatmullRom(_cPoint0, _cPoint1, _cPoint2, _cPoint3, t2);

		point[2][0] = CalculatePointCatmullRom(_cPoint1, _cPoint2, _cPoint3, _cPoint3, t1);
		point[2][1] = CalculatePointCatmullRom(_cPoint1, _cPoint2, _cPoint3, _cPoint3, t2);

		Drawline_se(point[0][0], point[0][1], _viewProjectionMatrix, _viewportMatrix, _color);
		Drawline_se(point[1][0], point[1][1], _viewProjectionMatrix, _viewportMatrix, _color);
		Drawline_se(point[2][0], point[2][1], _viewProjectionMatrix, _viewportMatrix, _color);
	}
	if (_isDrawPoint)
	{
		Sphere point[4];
		point[0].center = _cPoint0;
		point[1].center = _cPoint1;
		point[2].center = _cPoint2;
		point[3].center = _cPoint3;
		for (int i = 0; i < 4; i++)
		{
			point[i].radius = 0.01f;
			//DrawSphere(point[i], _viewProjectionMatrix, _viewportMatrix, BLACK);
		}
	}
}

void DrawBall(const Ball& _ball, const Matrix4x4& _viewProjectionMatrix, const Matrix4x4& _viewportMatrix)
{
	Sphere ball{
		.center = _ball.position,
		.radius = _ball.radius
	};
	DrawSphere(ball, _viewProjectionMatrix, _viewportMatrix, _ball.color);
}


float LerpShortAngle(float _a, float _b, float _t) {
	// 角度差を計算
	float diff = _b - _a;
	// 差を [-pi, pi] の範囲に調整
	float rotate = std::remainder(diff, std::numbers::pi_v<float> *2.0f);

	// 最短角度を使って補間
	return _a + rotate * _t;
}

Quaternion Slerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
{
    Quaternion q0 = _q1;
    float dot = _q1.Dot(_q2);
    if (dot < 0.0f)
	{
		q0 = -_q1;
        dot = -dot;
	}
	if (dot > 1.0f - 1e-6f)
	{		
		return Lerp(q0, _q2, _t);
	}
    float theta = std::acosf(dot);
	float scale0, scale1;

    scale0 = std::sinf((1.0f - _t) * theta);
    scale1 = std::sinf(_t * theta);

    return (q0 * scale0 + _q2 * scale1) / std::sinf(theta);
}

float Lerp(float _a, float _b, float _t)
{
    return _a + (_b - _a) * _t;		
}

Vector2 Lerp(const Vector2& _v1, const Vector2& _v2, float _t)
{
    return Vector2(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t));
}

Vector3 Lerp(const Vector3& _v1, const Vector3& _v2, float _t)
{
    return Vector3(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t), Lerp(_v1.z, _v2.z, _t));
}

Vector4 Lerp(const Vector4& _v1, const Vector4& _v2, float _t)
{
    return Vector4(Lerp(_v1.x, _v2.x, _t), Lerp(_v1.y, _v2.y, _t), Lerp(_v1.z, _v2.z, _t), Lerp(_v1.w, _v2.w, _t));
}

Quaternion Lerp(const Quaternion& _q1, const Quaternion& _q2, float _t)
{
	return Quaternion(Lerp(_q1.x, _q2.x, _t), Lerp(_q1.y, _q2.y, _t), Lerp(_q1.z, _q2.z, _t), Lerp(_q1.w, _q2.w, _t));
}

Vector3 Project(const Vector3& _v1, const Vector3& _v2)
{
	Vector3 normalize = Normalize(_v2);

	Vector3 result = Multiply(Dot(_v1, normalize), normalize);

	return result;
}

Vector3 ClosestPoint(const Vector3& _point, const Segment& _segment)
{
	Vector3 cp = Add(_segment.origin, Project(Subtract(_point, _segment.origin), _segment.diff));

	return cp;
}

Vector3 Perpendicular(const Vector3& _v)
{
	if (_v.x != 0.0f || _v.y != 0.0f)
	{
		return { -_v.y,_v.x,0.0f };
	}
	return { 0.0f, -_v.z,_v.y };
}

Vector3 Reflect(const Vector3& _input, const Vector3& _normal)
{
	Vector3 result;

	result = _input - 2.0f * (Dot(_input, _normal) * _normal);

	return Vector3(result);
}

void GetPlaneVertex(const Plane& _plane, Vector3* _vertex)
{
	Vector3 center = _plane.normal * _plane.distance;

	_vertex[0] = Normalize(Perpendicular(_plane.normal));
	_vertex[1] = { -_vertex[0].x, -_vertex[0].y, -_vertex[0].z };
	_vertex[2] = Cross(_plane.normal, _vertex[0]);
	_vertex[3] = { -_vertex[2].x, -_vertex[2].y, -_vertex[2].z };

	for (int i = 0; i < 4; i++)
		_vertex[i] = _vertex[i] * _plane.scalar + center;
}

bool IsCollision(const Sphere& _s1, const Sphere& _s2)
{
	float distance = Length(Subtract(_s1.center, _s2.center));
	return distance <= _s1.radius + _s2.radius ? true : false;
}

bool IsCollision(const Sphere& _s, const Plane& _p)
{
	float distance = Dot(_p.normal, _s.center) - _p.distance;
	distance = distance < 0 ? -distance : distance;

	if (distance <= _s.radius)
		return true;
	return false;
}

bool IsCollision(const Plane& _plane, const Segment& _segment)
{
	float dot = Dot(_plane.normal, _segment.diff);

	if (dot == 0.0f)
	{
		return false;
	}

	float t = (_plane.distance - Dot(_segment.origin, _plane.normal)) / dot;

	if (t < 0.0f || t>1.0f)
		return false;

	return true;
}

bool IsCollision(const Triangle& _triangle, const Segment& _segment)
{
	Plane lPlane = CalculatePlane(_triangle);

	float dot = Dot(lPlane.normal, _segment.diff);

	if (dot == 0.0f)
	{
		return false;
	}

	float t = (lPlane.distance - Dot(_segment.origin, lPlane.normal)) / dot;

	if (t < 0.0f || t>1.0f)
		return false;

	Vector3 point = _segment.origin + _segment.diff * t;

	Vector3 v01 = _triangle.vertices[1] - _triangle.vertices[0];
	Vector3 v1p = point - _triangle.vertices[1];

	Vector3 v12 = _triangle.vertices[2] - _triangle.vertices[1];
	Vector3 v2p = point - _triangle.vertices[2];

	Vector3 v20 = _triangle.vertices[0] - _triangle.vertices[2];
	Vector3 v0p = point - _triangle.vertices[0];

	Vector3 cross01 = Cross(v01, v1p);
	Vector3 cross12 = Cross(v12, v2p);
	Vector3 cross20 = Cross(v20, v0p);

	if (Dot(cross01, lPlane.normal) >= 0.0f &&
		Dot(cross12, lPlane.normal) >= 0.0f &&
		Dot(cross20, lPlane.normal) >= 0.0f)
	{
		return true;
	}
	return false;

}

bool IsCollision(const AABB& _a, const AABB& _b)
{
	if ((_a.min.x <= _b.max.x && _a.max.x >= _b.min.x) && // x
		(_a.min.y <= _b.max.y && _a.max.y >= _b.min.y) && // y
		(_a.min.z <= _b.max.z && _a.max.z >= _b.min.z)) { // z
		//衝突
		return true;
	}

	return false;
}


bool IsCollision(const AABB& _aabb, const Vector3& _point)
{
	Vector3 closestPoint;
	closestPoint.x = std::clamp(_point.x, _aabb.min.x, _aabb.max.x);
	closestPoint.y = std::clamp(_point.y, _aabb.min.y, _aabb.max.y);
	closestPoint.z = std::clamp(_point.z, _aabb.min.z, _aabb.max.z);

	float distance = Length(closestPoint - _point);

	if (distance <= 0)
	{
		return true;
	}

	return false;
}

bool IsCollision(const AABB& _a, const Sphere& _s)
{

	Vector3 closestPoint;
	closestPoint.x = std::clamp(_s.center.x, _a.min.x, _a.max.x);
	closestPoint.y = std::clamp(_s.center.y, _a.min.y, _a.max.y);
	closestPoint.z = std::clamp(_s.center.z, _a.min.z, _a.max.z);

	float distance = Length(closestPoint - _s.center);

	if (distance <= _s.radius)
	{
		return true;
	}

	return false;
}

bool IsCollision(const AABB& _aabb, const Segment& _segment)
{
	Vector3 tminVec;
	Vector3 tmaxVec;

	tminVec.y = (_aabb.min.y - _segment.origin.y) / _segment.diff.y;
	tminVec.x = (_aabb.min.x - _segment.origin.x) / _segment.diff.x;
	tminVec.z = (_aabb.min.z - _segment.origin.z) / _segment.diff.z;

	tmaxVec.x = (_aabb.max.x - _segment.origin.x) / _segment.diff.x;
	tmaxVec.y = (_aabb.max.y - _segment.origin.y) / _segment.diff.y;
	tmaxVec.z = (_aabb.max.z - _segment.origin.z) / _segment.diff.z;

	Vector3 tNear;
	tNear.x = std::min(tminVec.x, tmaxVec.x);
	tNear.y = std::min(tminVec.y, tmaxVec.y);
	tNear.z = std::min(tminVec.z, tmaxVec.z);

	Vector3 tFar;
	tFar.x = std::max(tminVec.x, tmaxVec.x);
	tFar.y = std::max(tminVec.y, tmaxVec.y);
	tFar.z = std::max(tminVec.z, tmaxVec.z);

	float tmin = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tmax = std::min(std::min(tFar.x, tFar.y), tFar.z);

	if (tmin <= tmax &&
		tmax >= 0.0f &&
		tmin <= 1.0f)
	{
		//衝突
		return true;
	}

	return false;
}

bool IsCollision(const OBB& _obb, const Sphere& _sphere)
{
	Matrix4x4 obbWolrdMat = MakeAffineMatrix({ 1.0f,1.0f ,1.0f }, _obb.rotate, _obb.center);
	Matrix4x4 obbWorldMatInv = Inverse(obbWolrdMat);

	Vector3  centerInOBBLocalSphere = Transform(_sphere.center, obbWorldMatInv);
	AABB aabbOBBLocal{ .min = -_obb.size,.max = _obb.size };
	Sphere sphereOBBLocal{ centerInOBBLocalSphere,_sphere.radius };

	return IsCollision(aabbOBBLocal, sphereOBBLocal);

}

bool IsCollision(const OBB& _obb, const Segment& _segment)
{
	Matrix4x4 obbWolrdMat = MakeAffineMatrix({ 1.0f,1.0f ,1.0f }, _obb.rotate, _obb.center);
	Matrix4x4 obbWorldMatInv = Inverse(obbWolrdMat);
	Vector3 localOrigin = Transform(_segment.origin, obbWorldMatInv);
	Vector3 localEnd = Transform(_segment.origin + _segment.diff, obbWorldMatInv);

	AABB localAABB{
		{-_obb.size.x, -_obb.size.y, -_obb.size.z},
		{+_obb.size.x, +_obb.size.y, +_obb.size.z},
	};
	Segment localSegment;
	localSegment.origin = localOrigin;
	localSegment.diff = localEnd - localOrigin;

	return IsCollision(localAABB, localSegment);
}

bool IsCollision(const OBB& _obb1, const OBB& _obb2)
{
	/// 分離軸候補の計算
	Vector3 axes[15];
	axes[0] = _obb1.orientations[0];
	axes[1] = _obb1.orientations[1];
	axes[2] = _obb1.orientations[2];

	axes[3] = _obb2.orientations[0];
	axes[4] = _obb2.orientations[1];
	axes[5] = _obb2.orientations[2];

	int index = 6;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			axes[index++] = Cross(_obb1.orientations[i], _obb2.orientations[j]);
		}
	}

	for (auto axis : axes)
	{
		float minObb1, maxObb1;
		float minObb2, maxObb2;

		//軸に射影および点の最大と最小を求める
		CalculateProjectionRange(_obb1, axis, minObb1, maxObb1);
		CalculateProjectionRange(_obb2, axis, minObb2, maxObb2);

		float l1, l2;
		l1 = maxObb1 - minObb1;
		l2 = maxObb2 - minObb2;

		float sumSpan = l1 + l2;

		float longSpan = std::max(maxObb1, maxObb2) - std::min(minObb1, minObb2);
		if (sumSpan < longSpan)
		{	//分離している	//すなわち衝突していない
			return false;
		}
	}
	return true;
}

Plane CalculatePlane(const Triangle& _triangle)
{
	Plane result{};

	//ab,bcを求める
	Vector3 edge1 = _triangle.vertices[1] - _triangle.vertices[0];
	Vector3 edge2 = _triangle.vertices[2] - _triangle.vertices[1];

	//法線計算
	result.normal = Cross(edge1, edge2);
	result.normal = Normalize(result.normal);

	//平面方程式を用いて距離を求める
	result.distance = Dot(_triangle.vertices[0], result.normal);

	return result;
}

void CalculateProjectionRange(const OBB& _obb, const Vector3& _axis, float& _min, float& _max)
{
	Vector3 verties[8];
	_obb.CaluculateVertices(verties);

	_min = std::numeric_limits<float>::infinity();
	_max = -(float)std::numeric_limits<float>::infinity();

	for (auto vertex : verties)
	{
		float proj = Dot(_axis, vertex);
		_min = std::min(_min, proj);
		_max = std::max(_max, proj);
	}

	return;
}

Vector3 CalculatePointBezier(const Bezier& _bezier, float _t)
{
	Vector3 point[3];
	//point[0] = Lerp(_bezier.point[0], _bezier.point[1], _t);
	//point[1] = Lerp(_bezier.point[1], _bezier.point[2], _t);
	//point[2] = Lerp(point[0], point[1], _t);

	return point[2];
}

Vector3 CalculatePointCatmullRom(const Vector3& _cPoint0, const Vector3& _cPoint1, const Vector3& _cPoint2, const Vector3& _cPoint3, float _t)
{
	Vector3 result;

	result.x = 1.0f / 2.0f * ((-_cPoint0.x + 3 * _cPoint1.x - 3 * _cPoint2.x + _cPoint3.x) * _t * _t * _t +
							  (2 * _cPoint0.x - 5 * _cPoint1.x + 4 * _cPoint2.x - _cPoint3.x) * _t * _t +
							  (-_cPoint0.x + _cPoint2.x) * _t +
							  2 * _cPoint1.x);

	result.y = 1.0f / 2.0f * ((-_cPoint0.y + 3 * _cPoint1.y - 3 * _cPoint2.y + _cPoint3.y) * _t * _t * _t +
							  (2 * _cPoint0.y - 5 * _cPoint1.y + 4 * _cPoint2.y - _cPoint3.y) * _t * _t +
							  (-_cPoint0.y + _cPoint2.y) * _t +
							  2 * _cPoint1.y);

	result.z = 1.0f / 2.0f * ((-_cPoint0.z + 3 * _cPoint1.z - 3 * _cPoint2.z + _cPoint3.z) * _t * _t * _t +
							  (2 * _cPoint0.z - 5 * _cPoint1.z + 4 * _cPoint2.z - _cPoint3.z) * _t * _t +
							  (-_cPoint0.z + _cPoint2.z) * _t +
							  2 * _cPoint1.z);

	return result;
}

void AABB::Update()
{
	this->min.x = (std::min)(this->min.x, this->max.x);
	this->max.x = (std::max)(this->min.x, this->max.x);
	this->min.y = (std::min)(this->min.y, this->max.y);
	this->max.y = (std::max)(this->min.y, this->max.y);
	this->min.z = (std::min)(this->min.z, this->max.z);
	this->max.z = (std::max)(this->min.z, this->max.z);
}

void OBB::CalculateOrientations()
{
	Matrix4x4 rotateMatrix = MakeRotateMatrix(this->rotate);


	this->orientations[0].x = rotateMatrix.m[0][0];
	this->orientations[0].y = rotateMatrix.m[0][1];
	this->orientations[0].z = rotateMatrix.m[0][2];

	this->orientations[1].x = rotateMatrix.m[1][0];
	this->orientations[1].y = rotateMatrix.m[1][1];
	this->orientations[1].z = rotateMatrix.m[1][2];

	this->orientations[2].x = rotateMatrix.m[2][0];
	this->orientations[2].y = rotateMatrix.m[2][1];
	this->orientations[2].z = rotateMatrix.m[2][2];

	this->orientations[0] = Normalize(this->orientations[0]);
	this->orientations[1] = Normalize(this->orientations[1]);
	this->orientations[2] = Normalize(this->orientations[2]);
}

void OBB::CaluculateVertices(Vector3* vertices) const
{
	Vector3 rotateAxis[3];
	rotateAxis[0] = this->orientations[0] * this->size.x;
	rotateAxis[1] = this->orientations[1] * this->size.y;
	rotateAxis[2] = this->orientations[2] * this->size.z;

	vertices[0] = this->center + rotateAxis[0] + rotateAxis[1] + rotateAxis[2];
	vertices[1] = this->center + rotateAxis[0] + rotateAxis[1] - rotateAxis[2];
	vertices[2] = this->center + rotateAxis[0] - rotateAxis[1] + rotateAxis[2];
	vertices[3] = this->center + rotateAxis[0] - rotateAxis[1] - rotateAxis[2];
	vertices[4] = this->center - rotateAxis[0] + rotateAxis[1] + rotateAxis[2];
	vertices[5] = this->center - rotateAxis[0] + rotateAxis[1] - rotateAxis[2];
	vertices[6] = this->center - rotateAxis[0] - rotateAxis[1] + rotateAxis[2];
	vertices[7] = this->center - rotateAxis[0] - rotateAxis[1] - rotateAxis[2];
}

