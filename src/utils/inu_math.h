#pragma once

#include "vectors.h"

// instead of creating the vector2d class here...use the vec2/ivec2/vec3 structs from vectors.h
// for right now...don't do anything for vec4 from that file

// The idea behind this is that vectors.h just contains the vector data structures
// but inu_math.h will contain the actual math functionality with these data structures
// b/c they are more so math helper functions

/*
	EXAMPLE

vec2 addition(const vec2& vect1, const vec2& vect2);
vec2 subtraction(const vec2& vect1, const vec2& vect2);
vec2 multiplication(const vec2& vect, float scalar);
vec2 multiplication(const vec2& vect1, const vec2& vect1);
etc...

ivec2 addition(const ivec2& vect1, const ivec2& vect2);
ivec2 subtraction(const ivec2& vect1, const ivec2& vect2);
ivec2 multiplication(const ivec2& vect, float scalar);
ivec2 multiplication(const ivec2& vect1, const ivec2& vect1);
etc...

vec3 addition(const vec3& vect1, const vec3& vect2);
vec3 subtraction(const vec3& vect1, const vec3& vect2);
vec3 multiplication(const vec3& vect, float scalar);
vec3 multiplication(const vec3& vect1, const vec3& vect1);
etc...

 */

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

