#pragma once

#include <iostream>

using namespace std;


class V3 {
public:
	float xyz[3];
	V3() {};
	V3(float x);
	V3(float x, float y, float z);
	V3 UnitVector();
	float Length();
	void SetColor(unsigned int color);
	unsigned int GetColor();
	V3 cross(V3 v1);
	V3 Rotate(V3 a, float angled);    // Rotate in arbitrary axis 
	V3 RotateThisPointAboutArbitraryAxis(V3 O, V3 a, float angled);
	V3 Reflect(V3 n);
	V3 Refract(V3 n, float ratio);
	float& operator[](int i);
	float operator*(V3 v1);
	V3 operator*(float scf);
	V3 operator/(float scf);
	V3 operator+(V3 v1);
	V3 operator+(float f);
	V3 operator-(V3 v1);
	V3 operator^(V3 v1);
	bool operator==(V3 v1);
	bool operator!=(V3 v1);
	friend istream& operator>>(istream& ist, V3 &v);
	friend ostream& operator<<(ostream& ost, V3 v);

	inline float x() const { return xyz[0]; }
	inline float y() const { return xyz[1]; }
	inline float z() const { return xyz[2]; }

	inline float r() const { return xyz[0]; }
	inline float g() const { return xyz[1]; }
	inline float b() const { return xyz[2]; }
};