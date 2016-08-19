
#ifndef __MathVector_h__
#define __MathVector_h__

#include "BinReader.h"

struct Vector2
{
	float	X, Y;

	Vector2()
		: X(0), Y(0)
	{}

	Vector2(float x, float y)
		: X(x), Y(y)
	{}

	void Read(BinReader & reader)
	{
		X = reader.Read<float>();
		Y = reader.Read<float>();
	}
};

struct Vector3 : Vector2
{
	float	Z;

	Vector3()
		: Z(0)
	{}

	Vector3(float x, float y, float z)
		: Vector2(x, y), Z(z)
	{}

	void Read(BinReader & reader)
	{
		Vector2::Read(reader);
		Z = reader.Read<float>();
	}

	Vector3 Mul(float v) const
	{
		return Vector3(X * v, Y * v, Z * v);
	}

	Vector3 Sub(const Vector3 & other) const
	{
		return Vector3(X - other.X, Y - other.Y, Z - other.Z);
	}

	Vector3 Add(const Vector3 & other) const
	{
		return Vector3(X + other.X, Y + other.Y, Z + other.Z);
	}

	float Length() const
	{
		return sqrt(X * X + Y * Y + Z * Z);
	}

	float Normalize()
	{
		float len = Length();

		if (len != 0)
		{
			float invLen = 1 / len;

			X *= invLen;
			Y *= invLen;
			Z *= invLen;
		}

		return len;
	}

	static Vector3 Cross(const Vector3 & left, const Vector3 & right)
	{
		return Vector3(
			(left.Y * right.Z) - (right.Y * left.Z),
			(right.X * left.Z) - (left.X * right.Z),
			(left.X * right.Y) - (right.X * left.Y));
	}
};

struct Vector4 : Vector3
{
	float	W;

	Vector4()
		: W(0)
	{}

	void Read(BinReader & reader)
	{
		Vector3::Read(reader);
		W = reader.Read<float>();
	}
};

#endif