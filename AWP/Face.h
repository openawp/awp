#pragma once
#include"stdafx.h"

/*三角网格中的面*/
class Face {
public:

	/*面上的三个点的id*/
	int vertex[3];

public:

	/*无参数的构造函数*/
	Face(){}

	/*带参数的构造函数*/
	Face(int id1, int id2, int id3) {
		vertex[0] = id1;
		vertex[1] = id2;
		vertex[2] = id3;
	}

	/*定义运算符，可通过类似数组的方式索引*/
	int& operator[](int index) {
		return vertex[index];
	}

	/*定义运算符，可通过类似数组的方式索引*/
	int operator[](int index) const {
		return vertex[index];
	}
};
