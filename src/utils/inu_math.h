#pragma once

#include "vectors.h"

class Vector2D {
public: 
	float x;
	float y;


	//constructors
	Vector2D();
	Vector2D(float x, float y);

	//destructors
	~Vector2D(); 

	Vector2D addition(const Vector2D& vect) const;
	Vector2D subtraction(const Vector2D& vect) const;
	Vector2D multiplication(float scalar) const;
	Vector2D multiplication(const Vector2D& vect) const;
	Vector2D division(float scalar) const;
	Vector2D division(const Vector2D& vect) const;

	float magnitude() const;
	Vector2D normalize() const;
	float dotproduct(const Vector2D& vect) const;
	float crossproduct(const Vector2D& vect) const;


};

class Vector3D {
public:
	float x;
	float y;
	float z;


	//constructors
	Vector3D();
	Vector3D(float x, float y, float z);

	//destructors
	~Vector3D();

	Vector3D addition(const Vector3D& vect) const;
	Vector3D subtraction(const Vector3D& vect) const;
	Vector3D multiplication(float scalar) const;
	Vector3D multiplication(const Vector3D& vect) const;
	Vector3D division(float scalar) const;
	Vector3D division(const Vector3D& vect) const;

	float magnitude() const;
	Vector3D normalize() const;
	float dotproduct(const Vector3D& vect) const;
	float crossproduct(const Vector3D& vect) const;


};

