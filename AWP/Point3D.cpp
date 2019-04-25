#include"stdafx.h"
#include"Point3D.h"

/*二元运算符：两个三维向量的加法*/
Point3D operator + (const Point3D& a, const Point3D& b) {
	return Point3D(a.x + b.x, a.y + b.y, a.z + b.z);
}

/*二元运算符：两个三维向量的加法*/
Point3D operator - (const Point3D& a, const Point3D& b) {
	return Point3D(a.x - b.x, a.y - b.y, a.z - b.z);
}

/*二元运算符：两个三维向量的叉乘*/
Point3D operator * (const Point3D& a, const Point3D& b) {
	return Point3D(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

/*二元运算符：数乘*/
Point3D operator * (const float ti, const Point3D& b) {
	return Point3D(ti*b.x, ti*b.y, ti*b.z);
}

/*两个三维向量的点乘*/
float dotProduct(const Point3D& a, const Point3D& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*三个点所构成的三角形的面积*/
float getTriangleArea(const Point3D& a, const Point3D& b, const Point3D& c) {
	Point3D crossProduct = (b - a) * (c - b);
	return 0.5 * crossProduct.getLength();
}

/*两个向量之间构成的角度大小*/
float getAngleBetween(Point3D a, Point3D b) {
	float cosAngle = dotProduct(a, b) / (a.getLength() * b.getLength());
	if (cosAngle - 1 > -EPS)
	{
		cosAngle = 1;
	}
	else if (cosAngle + 1 < EPS)
	{
		cosAngle = -1;
	}
	return acos(cosAngle);
}

/*向量oe和向量os构成的角度大小*/
float getAngleBetween(Point3D o, Point3D s, Point3D e) {
	return getAngleBetween(o - e, o - s);
}

/*两点之间的距离*/
float getDistance(Point3D a, Point3D b) {
	return (b - a).getLength();
}

/*判断a, b, c三点是否共线，共线返回true， 否则返回false*/
bool isCollineation(Point3D a, Point3D b, Point3D c) {
	return ((b - a)*(c - a)).getLength() < EPS;
}

/*求a到线段bc的垂足*/
Point3D getFootPoint(Point3D a, Point3D b, Point3D c) {
	Point3D footPoint;
	float dx = b.x - c.x;
	float dy = b.y - c.y;
	float dz = b.z - c.z;

	if (abs(dx) < EPS && abs(dy) < EPS && abs(dz) < EPS) {
		return b;
	}
	float u = (a.x - b.x)*(b.x - c.x) +
		(a.y - b.y)*(b.y - c.y) + (a.z - b.z)*(b.z - c.z);
	u = u / ((dx*dx) + (dy*dy) + (dz*dz));
	footPoint.x = b.x + u*dx;
	footPoint.y = b.y + u*dy;
	footPoint.z = b.z + u*dz;
	return footPoint;
}