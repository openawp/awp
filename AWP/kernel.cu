#include "stdafx.h"
#include <cstdio>
#include <map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <thrust/sort.h> 
#include "Model.h"
#include "Timer.h"
#include "Point.h"
#include "Model_vector.h"
#include "AWP.h"

using namespace std;

#define TID threadIdx.x
#define BID blockIdx.x
#define BMX blockDim.x
#define SIZE_WINDOWS_HE 40//4 
#define huatu 0
#define INF 9999.0
#define TEST_AVG 1
#define EPCH_AVG 1
#define DEBUG 0
#define WU 0
#define TEST_MAX_MIN 0
#define TEST_ZT 1
#define CUT 1
#define SSD 0 
#define MSD 1
/*#define EB 16
#define VB 16
#define EE 13063
#define VV 2178
*/
typedef char uchar;
/*source point: 512
torus.ply 60 4
bunny.obj 512 50 282 0.558262
golf: 1 45 430
dragon: 512 403 40
*/
//Model model = Model("model/bunny_nf144k.m");

int sid = -1;
char outPath[256] = { '\0' };
char fileName[256] = { '\0' };

void dealInput(int argc, char *argv[]) {
	for (int i = 1; i < argc;) {
		if (strcmp(argv[i], "-s") == 0) {
			sid = atoi(argv[i + 1]);
			i += 2;
		}
		else if (strcmp(argv[i], "-o") == 0) {
			strcpy(outPath, argv[i + 1]);
			i += 2;
		}
		else if (strcmp(argv[i], "-m") == 0) {
			strcpy(fileName, argv[i + 1]);
			i += 2;
		}
		else i++;
	}
}

int main(int argc, char* argv[]) {
	dealInput(argc, argv);
	if (sid < 0) {
		puts("No source!");
		return 0;
	}
	if (strlen(fileName) == 0) {
		puts("No model!");
		return 0;
	}
	AWP *awp = new AWP(fileName); 
	//model = Model_vector(argv[1]);
	cout << "------------------------load model begin------------------------\n";
	cout << "File name:\t" << awp->model.GetFileName() << endl;
	try {
		awp->model.LoadModel();
	}
	catch (const char* msg) {
		cout << "ERRORS happen!\n" << msg << endl;
		return 1;
	}
	awp->model.dist = new float[awp->model.GetNumOfVerts()];
	Point3D maxP = awp->model.vertexs[0], minP = awp->model.vertexs[1];
	for (int i = 0; i < awp->model.GetNumOfVerts(); i++) {
		maxP.x = max(maxP.x, awp->model.vertexs[i].x);
		maxP.y = max(maxP.y, awp->model.vertexs[i].y);
		maxP.z = max(maxP.z, awp->model.vertexs[i].z);
		minP.x = min(minP.x, awp->model.vertexs[i].x);
		minP.y = min(minP.y, awp->model.vertexs[i].y);
		minP.z = min(minP.z, awp->model.vertexs[i].z);
	}
	float len = getDistance(maxP, minP) / sqrt(3.0);
	for (int i = 0; i < awp->model.GetNumOfVerts(); i++) {
		awp->model.vertexs[i].x = (awp->model.vertexs[i].x - minP.x) / len;
		awp->model.vertexs[i].y = (awp->model.vertexs[i].y - minP.y) / len;
		awp->model.vertexs[i].z = (awp->model.vertexs[i].z - minP.z) / len;
	}
	awp->model.creatHalfEdge();
	try {
		awp->model.findAllInfoOnVers();
	}
	catch (const char* msg) {
		cout << "ERRORS happen!\n" << endl;
		return 1;
	}
	if (!awp->model.isClose) {
		cout << "It is an open model!\n";
		return 0;
	}
	else {
		cout << "It is a close model\n";
	}
	
	//system("pause"); 
	printf("The number of face:\t%d\n", awp->model.GetNumOfFaces());
	printf("The number of vertex:\t%d\n", awp->model.GetNumOfVerts());
	printf("The number of edge:\t%d\n", awp->model.GetNumOfEdges()); 
	cout << "-------------------------load model end-------------------------\n\n";
	//sid = getnum(argv[2]);
	do { 
		awp->run(sid);
		puts("------------------------Output the result-----------------------");
		puts("Outputing.....");
		strcpy(awp->outPath, outPath);
		awp->outVt();
		awp->outInfo();
		puts("----------------------------The end!----------------------------");

		//break;
		puts("If you want to continue use this model, you can input new id of start vertex and new output mesh file. Such as 512 512_outInfo.obj\n Else if you input -1 to end.");
		scanf("%d", &sid); 
		if (sid < 0 || sid >= awp->model.GetNumOfVerts()) break;
		scanf("%s", outPath);
	} while (sid != -1);
	return 0;
}
/*
0 0 0
0 0 1
0 1 0
1 0 0
1 1 1
2 1 0
2 0 3
0 1 3
*/