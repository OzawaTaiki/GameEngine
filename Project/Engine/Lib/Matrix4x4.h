#pragma once


#ifdef _DEBUG
#include <imgui.h>
#include <string>
#endif // _DEBUG

#include <Vector3.h>


struct Matrix4x4
{
	float m[4][4];

	static Matrix4x4 Identity() {
		return Matrix4x4{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	}

	Vector3 GetScale() const;

	Vector3 GetTranslate() const;


	Matrix4x4 operator*(const Matrix4x4& _mat);

	Matrix4x4& operator*=(const Matrix4x4& _mat);
		

#ifdef _DEBUG
	void ShowData(const std::string& _label, bool _newWindow = true) const;
#endif // _DEBUG


};