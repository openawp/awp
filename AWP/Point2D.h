#pragma once
#include "stdafx.h"

__device__ struct Point2D{
	float x, y;
	__device__ __host__ Point2D(){}
	__device__ __host__ Point2D(float _x, float _y) :x(_x), y(_y){}
	__device__ __host__ Point2D operator + (Point2D t)
	{
		return Point2D(x + t.x, y + t.y);
	}

	__device__ __host__ Point2D operator - (Point2D t)
	{
		return Point2D(x - t.x, y - t.y);
	}

	__device__ __host__ Point2D operator * (float a)
	{
		return Point2D(a*x, a*y);
	}

	__device__ __host__ Point2D operator / (float a)
	{
		return Point2D(x / a, y / a);
	}

	__device__ __host__ float operator * (Point2D t)
	{
		return x*t.y - t.x*y;
	}

	__device__ __host__ float len()
	{
		return sqrt(x*x + y*y);
	}

};