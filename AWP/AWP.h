#pragma once
#include "stdafx.h"
#include "Model_vector.h"
#include "Timer.h"

#define TID threadIdx.x
#define BID blockIdx.x
#define BMX blockDim.x
#define SIZE_WINDOWS_HE 40//4 //修改时注意不能超过unsigned char的表示范围
int DEI = 362;
#define huatu 0
#define INF 99999.0
#define TEST_AVG 1
#define EPCH_AVG 1
#define DEBUG 0
#define WU 0
#define TEST_MAX_MIN 1
#define TEST_ZT 0
#define CUT 1
#define SSD 0 
#define MSD 1
double EPS1 = 6665;
__device__ __host__ struct WindowsInAng{
	Point2D I;
	float val;
	__device__ __host__ bool operator<(const WindowsInAng wi)const{
		return val < wi.val;
	}
	__device__ __host__ WindowsInAng(){}
	__device__ __host__ WindowsInAng(float _val, Point2D _p) {
		val = _val;
		I = _p;
	}
};

class AWP{
public:
	AWP(){}
	AWP(char *name) {
		model = Model_vector(name);
	}
	Model_vector model;
	char outPath[255];
	void run(int sid);
	bool *isU;
	WindowsInAng *ang;
	float MAXDIST;
	void outInfo();
	void outVt();
};

void AWP::outVt() {
	model.angOnEdge.clear();
	model.angOnEdge.resize(model.vertexs.size());
	MAXDIST = 0.0;
	for (int i = 0; i < model.vertexs.size(); i++) {
		if (fabs(model.dist[i] - INF) < EPS) continue;
		if (model.dist[i] > MAXDIST) MAXDIST = model.dist[i];
	}
	for (int i = 0; i < model.vertexs.size(); i++) {
		float tm = model.dist[i] / MAXDIST;
		model.angOnEdge.push_back(Point2D(tm, tm));
	}
}

void AWP::outInfo() {
	if (sizeof(outPath) == 0) {
		return;
	}
	/*
	ofstream fout(outPath);
	for (int i = 0; i < model.vertexs.size(); i++) {
		fout << "v " << model.vertexs[i].x << " " << model.vertexs[i].y << " " << model.vertexs[i].z << "\n";
	}
	for (int i = 0; i < model.vertexs.size(); i++) {
		double tm = 1.0*model.dist[i] / MAXDIST;
		fout << "vt " << tm << " " << tm << "\n";
	}
	for (int i = 0; i < model.faces.size(); i++) {
		fout << "f";
		for (int j = 0; j < 3; j++) {
			fout << " " << model.faces[i][j] + 1 << "/" << model.faces[i][j] + 1;
		}
		fout << "\n";
	}
	*/
	ofstream fout2(outPath);
	fout2.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
	fout2.precision(10);  // 设置精度 2
	for (int i = 0; i < model.vertexs.size(); i++) {
		fout2 << model.dist[i] << "\n";
	}
}

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
__device__ __host__ inline float getDistance2D(const Point2D& a, const Point2D& b) {
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

__device__ __host__ struct Windows {
	Point2D I;
	float d;
	float a, b, val;
	float lLen, rLen;
	char flag;
	bool isUseful;
	bool isLeft;
	bool test[29];
	__device__ __host__ bool operator<(const Windows wi) const {
		return a < wi.a;
	}
	__device__ __host__ Windows() {}
	__device__ __host__ Windows(float _d, float _a, float _b, float _l, float _r, float B, char _flag = 2) {
		d = _d, lLen = _l, rLen = _r;
		a = _a, b = _b;
		isUseful = true;
		isLeft = false;
		flag = _flag;
		float ang = (B*B + lLen*lLen - rLen*rLen) / (2 * B*lLen);
		I = Point2D(lLen*ang, lLen*sqrt(max(float(0.0), 1 - ang*ang)));
		if (I.x < a || I.x > b) {
			val = min(getDistance2D(I, Point2D(a, 0.0)), getDistance2D(I, Point2D(b, 0.0)));
		}
		else val = I.y;
		val += d;
	}
};

__device__ __host__ struct WindowsInEdge {
	uchar len[2];
	Windows buf[2][SIZE_WINDOWS_HE];
};

__device__ __host__ inline int idOfNextHalfEdge(int &eid) {
	return eid / 3 * 3 + (eid + 1) % 3;
}

__device__ __host__ inline int idOfPreHalfEdge(int &eid) {
	return eid / 3 * 3 + (eid + 2) % 3;
}

bool init_cuda(bool isOut) {
	cudaError_t cudaStatus;

	int num = 0;
	cudaDeviceProp prop;
	cudaStatus = cudaGetDeviceCount(&num);
	for (int i = 0; i < num; i++)
	{
		cudaGetDeviceProperties(&prop, i);
		if (isOut == false) continue;
		printf("   --- General Information for device %d ---\n", i);
		printf("Name:  %s\n", prop.name);
		printf("Compute capability:  %d.%d\n", prop.major, prop.minor);
		printf("Clock rate:  %d\n", prop.clockRate);
		printf("Device copy overlap:  ");
		if (prop.deviceOverlap)
			printf("Enabled\n");
		else
			printf("Disabled\n");
		printf("Kernel execution timeout :  ");
		if (prop.kernelExecTimeoutEnabled)
			printf("Enabled\n");
		else
			printf("Disabled\n");

		printf("   --- Memory Information for device %d ---\n", i);
		printf("Total global mem:  %ld\n", prop.totalGlobalMem);
		printf("Total constant Mem:  %ld\n", prop.totalConstMem);
		printf("Max mem pitch:  %ld\n", prop.memPitch);
		printf("Texture Alignment:  %ld\n", prop.textureAlignment);

		printf("   --- MP Information for device %d ---\n", i);
		printf("Multiprocessor count:  %d\n",
			prop.multiProcessorCount);
		printf("Shared mem per mp:  %ld\n", prop.sharedMemPerBlock);
		printf("Registers per mp:  %d\n", prop.regsPerBlock);
		printf("Threads in warp:  %d\n", prop.warpSize);
		printf("Max threads per block:  %d\n",
			prop.maxThreadsPerBlock);
		printf("Max thread dimensions:  (%d, %d, %d)\n",
			prop.maxThreadsDim[0], prop.maxThreadsDim[1],
			prop.maxThreadsDim[2]);
		printf("Max grid dimensions:  (%d, %d, %d)\n",
			prop.maxGridSize[0], prop.maxGridSize[1],
			prop.maxGridSize[2]);
		printf("\n");
	}
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess)
	{
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		return false;
	}
	return true;
}

__device__ bool filterByPoint(Windows& wi, float &dist, HalfEdge *halfEdges, int &eid) {
	//return true;
	if (dist < INF) {
		float a, b, c;
		a = halfEdges[eid].length, b = halfEdges[idOfNextHalfEdge(eid)].length, c = halfEdges[idOfPreHalfEdge(eid)].length;
		float ang = (c*c + a*a - b*b) / (2 * c*a);
		float x, y;
		float L, R;
		L = getDistance2D(wi.I, Point2D(wi.a, 0.0)), R = getDistance2D(wi.I, Point2D(wi.b, 0.0));
		x = c*ang, y = c*sqrt(max(float(0.0), 1 - ang*ang));
		ang = wi.d - dist;
		if (wi.flag == 2) {
			if (ang + L < EPS || ang + R < EPS) return true;
			if ((x - wi.a)*(x - wi.a) + y*y < (ang + L)*(ang + L)
				&& (x - wi.b)*(x - wi.b) + y*y < (ang + R)*(ang + R)) return false;
		}
		else if (wi.flag == 1) {
			ang += R;
			if (ang < EPS) return true;
			if ((x - wi.b)*(x - wi.b) + y*y < ang*ang) return false;
		}
		else {
			ang += L;
			if (ang < EPS) return true;
			if ((x - wi.a)*(x - wi.a) + y*y < ang*ang) return false;
		}
	}
	return true;
}

__device__ inline void checkWindows_YH(Windows &wi, float &len) {
	//return;
	if (wi.b - wi.a < EPS || wi.b > len + EPS) wi.isUseful = false;
	//if (wi.b - wi.a < EPS1 || wi.b > len + EPS1) wi.isUseful = false;
}

/*ICHFilter&*/
__device__ bool checkWindows_PCH(Windows& wi, float *dist, HalfEdge *halfEdges, int eid) {
	if (wi.I.y < EPS && (wi.I.x < wi.a + EPS || wi.I.x > wi.b - EPS)) return false;
	if (wi.b - wi.a < EPS) return false;
	//return true;
	int startId, endId;
	startId = halfEdges[eid].idOfStartPoint;
	endId = halfEdges[idOfNextHalfEdge(eid)].idOfStartPoint;
	float L, R, a;
	L = getDistance2D(wi.I, Point2D(wi.a, 0.0)), R = getDistance2D(wi.I, Point2D(wi.b, 0.0));
	a = halfEdges[eid].length;
	if (dist[startId] < INF && dist[startId] + wi.b < wi.d + R+EPS) {
		return false;
	}
	if (dist[endId] < INF && dist[endId] + a - wi.a < wi.d + L+EPS) {
		return false;
	}
	int pid = halfEdges[eid].idOfOppositeVer;
	if (dist[pid] < INF) {
		float b, c;
		b = halfEdges[idOfNextHalfEdge(eid)].length, c = halfEdges[idOfPreHalfEdge(eid)].length;
		float ang = (c*c + a*a - b*b) / (2 * c*a);
		float x, y;
		x = c*ang, y = c*sqrt(max(float(0.0), 1 - ang*ang));
		ang = wi.d - dist[pid];
		if (wi.flag == 2) {
			if (ang + L < EPS || ang + R < EPS) return true;
			if ((x - wi.a)*(x - wi.a) + y*y < (ang + L)*(ang + L)
				&& (x - wi.b)*(x - wi.b) + y*y < (ang + R)*(ang + R)) {
				return false;
			}
		}
		else if (wi.flag == 1) {
			ang += R;
			if (ang < EPS) return true;
			if ((x - wi.b)*(x - wi.b) + y*y < ang*ang) return false;
		}
		else {
			ang += L;
			if (ang < EPS) return true;
			if ((x - wi.a)*(x - wi.a) + y*y < ang*ang) return false;
		}
	}
	return true;
}

__device__ bool getDeleteSonByWindows_PCH(Windows &wi, Point2D& C, HalfEdge *halfEdges, int idOfOppositeEdge) {
	float x = (C.x - wi.I.x)*wi.I.y / (wi.I.y - C.y) + wi.I.x;
	float len = 0.0;
	Point2D A, B;
	if (fabs(wi.I.y) < EPS && fabs(C.y) >= EPS) {
		A = Point2D(wi.a, C.y*(wi.a - len) / (C.x - len));
		len = halfEdges[idOfOppositeEdge].length;
		B = Point2D(wi.b, C.y*(wi.b - len) / (C.x - len));
	}
	else if (fabs(wi.I.y) >= EPS && fabs(C.y) < EPS) {
		A = Point2D(wi.a, C.y);
		B = Point2D(wi.b, C.y);
	}
	else {
		float ty = wi.I.y*C.y*(len - wi.a) / (C.y*(wi.I.x - wi.a) - wi.I.y*(C.x - len));
		A = Point2D(ty*(wi.I.x - wi.a) / wi.I.y + wi.a, ty);
		len = halfEdges[idOfOppositeEdge].length;
		ty = wi.I.y*C.y*(len - wi.b) / (C.y*(wi.I.x - wi.b) - wi.I.y*(C.x - len));
		B = Point2D(ty*(wi.I.x - wi.b) / wi.I.y + wi.b, ty);
	}
	return getDistance2D(wi.I, A) > getDistance2D(wi.I, B);
}

__device__ inline void swap(Windows &a, Windows &b) {
	Windows wi = b;
	b = a;
	a = wi;
}

__device__ inline void QSort(Windows *windowsInEdge, int l, int r) {
	//return;
	//r--;
	//*
	if (r <= l) return;
	for (int i = l; i < r; i++) {
		for (int j = r; j > i; j--) {
			if (windowsInEdge[j] < windowsInEdge[j - 1]) {
				swap(windowsInEdge[j], windowsInEdge[j - 1]);
			}
		}
	}
	return;
}

__device__ inline bool checkCase10(Windows &p, Windows &q, Point2D &C) {
	if (p.b < q.b &&
		p.d + getDistance2D(p.I, Point2D(p.b, 0.0)) + getDistance2D(C, Point2D(p.b, 0.0)) < q.d + getDistance2D(q.I, Point2D(q.b, 0.0)) + getDistance2D(C, Point2D(q.b, 0.0))) {
		q.isUseful = false;
		return true;
	}
	return false;
}

__device__ inline Point2D Line_inter(Point2D A, Point2D B, Point2D C, Point2D D) {
	float k = -((A - C)*(D - C)) / ((B - A)*(D - C));
	return A + (B - A)*k;
}

__device__ inline uchar checkCase123(Windows &w0, Windows &w1) {
	if (w0.isUseful == false) return 0;
	else if (w1.isUseful == false) return 1;
	if (w1.b < w0.a + EPS) {
		Point2D u = Line_inter(w1.I, Point2D(w1.b, 0.0), w0.I, Point2D(w0.a, 0.0));
		if (w0.d + getDistance2D(w0.I, u) < w1.d + getDistance2D(w1.I, u)) {
			w1.isUseful = false;
			return 1;
		}
		else {
			w0.isUseful = false;
			return 0;
		}
	}
	else if (w1.a < w0.a) {
		float pa0 = w0.d + getDistance2D(w0.I, Point2D(w0.a, 0.0)), qa0 = w1.d + getDistance2D(w1.I, Point2D(w0.a, 0.0));
		float pb1 = w0.d + getDistance2D(w0.I, Point2D(w1.b, 0.0)), qb1 = w1.d + getDistance2D(w1.I, Point2D(w1.b, 0.0));
		if (pa0 < qa0) {
			w1.a = w0.a;
			if (w1.b < w0.b + EPS) {
				if (pb1 > qb1) {
					w0.b = w1.b;
					return 2;
				}
				else {
					w1.isUseful = false;
					return 1;
				}
			}
			else if (w0.d + getDistance2D(w0.I, Point2D(w0.b, 0.0)) < w1.d + getDistance2D(w1.I, Point2D(w0.b, 0.0))) {
				w1.a = w0.b;
				return 2;
			}
			return 2;
		}
		else {
			w0.isUseful = false;
			return 0;
		}
	}
	else if (w1.a < w0.b + EPS && w1.a + EPS > w0.a) {
		float pb0 = w0.d + getDistance2D(w0.I, Point2D(w1.a, 0.0)), qb0 = w1.d + getDistance2D(w1.I, Point2D(w1.a, 0.0));
		if (w0.d + getDistance2D(w0.I, Point2D(w1.a, 0.0)) > w1.d + getDistance2D(w1.I, Point2D(w1.a, 0.0))) {
			w0.b = w1.a;
			return 2;
		}
		if (w1.b < w0.b + EPS) {
			if (w0.d + getDistance2D(w0.I, Point2D(w1.b, 0.0)) > w1.d + getDistance2D(w1.I, Point2D(w1.b, 0.0))) {
				return 2;
			}
			else {
				w1.isUseful = false;
				return 1;
			}
		}
		else {
			if (w0.d + getDistance2D(w0.I, Point2D(w0.b, 0.0)) < w1.d + getDistance2D(w1.I, Point2D(w0.b, 0.0))) {
				w1.a = w0.b;
				return 2;
			}
		}
	}
	return 2;
}

__device__ inline bool checkCase10_f(Windows &p, Windows &q, Point2D &C) {
	if (p.b < q.b &&
		p.d + getDistance2D(p.I, Point2D(p.b, 0.0)) + getDistance2D(C, Point2D(p.b, 0.0)) > q.d + getDistance2D(q.I, Point2D(q.b, 0.0)) + getDistance2D(C, Point2D(q.b, 0.0))) {
		p.isUseful = false;
		return true;
	}
	return false;
}

__device__ bool createSonByWindows_VTP(Windows &wi, Point2D &C, bool isRight, HalfEdge *halfEdges, float *dist, int &idOfOpp) {
	if (!wi.isUseful) return false;
	wi.isUseful = false;
	float len = 0.0;
	if (wi.I.y < EPS && (wi.I.x < wi.a + EPS || wi.I.x > wi.b - EPS)) return false;
	if (fabs(wi.a - wi.b) < EPS) return false;
	if (isRight) len = halfEdges[idOfOpp].length;
	Point2D A, B;
	if (fabs(wi.I.y) < EPS && fabs(C.y) >= EPS) {
		A = Point2D(wi.a, C.y*(wi.a - len) / (C.x - len));
		B = Point2D(wi.b, C.y*(wi.b - len) / (C.x - len));
	}
	else if (fabs(wi.I.y) >= EPS && fabs(C.y) < EPS) {
		A = Point2D(wi.a, C.y);
		B = Point2D(wi.b, C.y);
	}
	else {
		float ty = wi.I.y*C.y*(len - wi.a) / (C.y*(wi.I.x - wi.a) - wi.I.y*(C.x - len));
		A = Point2D(ty*(wi.I.x - wi.a) / wi.I.y + wi.a, ty);
		ty = wi.I.y*C.y*(len - wi.b) / (C.y*(wi.I.x - wi.b) - wi.I.y*(C.x - len));
		B = Point2D(ty*(wi.I.x - wi.b) / wi.I.y + wi.b, ty);
	}
	if (isRight) {
		Windows nw = Windows(wi.d, getDistance2D(C, A), getDistance2D(C, B),
			getDistance2D(C, wi.I), wi.rLen, halfEdges[idOfPreHalfEdge(idOfOpp)].length, 1);
		if (checkWindows_PCH(nw, dist, halfEdges, idOfPreHalfEdge(idOfOpp))) {
			wi = nw;
			//if (nw.val > 9999) printf("haha\n");
			return true;
		}
	}
	else {
		Windows nw = Windows(wi.d, getDistance2D(A, Point2D(0.0, 0.0)), getDistance2D(B, Point2D(0.0, 0.0)),
			wi.lLen, getDistance2D(C, wi.I), halfEdges[idOfNextHalfEdge(idOfOpp)].length, 0);
		if (checkWindows_PCH(nw, dist, halfEdges, idOfNextHalfEdge(idOfOpp))) {
			wi = nw;
			//if (nw.val > 9999) printf("haha\n");
			return true;
		}
	}
	return false;
}

__device__ inline void checkCutPoint(Windows &w0, Windows &w1, float l, float r, bool isLeft) {
	float a0, a1, b0, b1;
	a0 = w0.a, a1 = w1.a, b0 = w0.b, b1 = w1.b;
	if (a1 < a0 + EPS) {
		if (b1 < b0 + EPS) {
			//[a0, b1]
			if (l < a0) {
				if (r < a0 + EPS) {
					if (isLeft) {
						w0.a = w1.b;
					}
					else {
						w1.b = r;
					}
				}
				else if (r < b1 + EPS) {
					if (!isLeft) {
						w0.a = w1.b = r;
					}
				}
				else {
					if (isLeft) {
						w1.b = w0.a;
					}
					else {
						w0.a = w1.b;
					}
				}
			}
			else if (l < b1) {
				if (r < b1 + EPS) {

				}
				else {
					if (isLeft) {
						w1.b = w0.a = l;
					}
				}
			}
			else {
				if (isLeft) {
					w0.a = l;
				}
				else {
					w1.b = w0.a;
				}
			}
		}
		else {
			//[a0, b0]
			if (l < a0) {
				if (r < a0 + EPS) {
					if (isLeft) {
						w0.a = w0.b;
					}
				}
				else if (r < b0 + EPS) {
					if (isLeft) {
						w0.b = r;
					}
					else w0.a = r;
				}
				else {
					if (!isLeft) w0.a = w0.b;
				}
			}
			else if (l < b0) {
				if (r < b0 + EPS) {
					if (isLeft) {
						w0.a = l, w0.b = r;
					}
				}
				else {
					if (isLeft) {
						w0.a = l;
					}
					else w0.b = l;
				}
			}
			else {
				if (isLeft) w0.a = w0.b;
			}
		}
	}
	else {
		if (b1 < b0 + EPS) {
			//[a1, b1]
			if (l < a1) {
				if (r < a1 + EPS) {
					if (!isLeft) {
						w1.b = w1.a;
					}
				}
				else if (r < b1 + EPS) {
					if (isLeft) {
						w1.a = r;
					}
					else w1.b = r;
				}
				else {
					if (isLeft) w1.b = w1.a;
				}
			}
			else if (l < b1) {
				if (r < b1 + EPS) {
					if (!isLeft) {
						w1.a = l, w1.b = r;
					}
				}
				else {
					if (isLeft) w1.b = l;
					else w1.a = l;
				}
			}
			else {
				if (!isLeft) w1.a = w1.b;
			}
		}
		else {
			//[a1, b0]
			if (l < a1) {
				if (r < a1 + EPS) {
					if (isLeft) {
						w0.b = w1.a;
					}
					else {
						w1.a = w0.b;
					}
				}
				else if (r < b0 + EPS) {
					if (isLeft) {
						w1.a = w0.b = r;
					}
				}
				else {
					if (isLeft) w1.a = w0.b;
					else w0.b = w1.a;
				}
			}
			else if (l < b0) {
				if (r < b0 + EPS) {
					if (isLeft) w0.b = r;
					else w1.a = l;
				}
				else {
					if (!isLeft) {
						w1.a = w0.b = l;
					}
				}
			}
			else {
				if (isLeft) w0.b = w1.a;
				else w1.a = w0.b;
			}
		}
	}
	w0.isUseful = ((w0.b - w0.a) > EPS);
	w1.isUseful = ((w1.b - w1.a) > EPS);

	//	w0.isUseful = (w0.b > w0.a);
	//	w1.isUseful = (w1.b > w1.a);
}

__device__ inline void checkCutPoint(Windows &w0, Windows &w1, float x) {
	if (w1.a < w0.a + EPS) {
		if (x < w0.a + EPS) {
			w0.a = w1.b;
		}
		else {
			if (w1.b < w0.b + EPS) {
				if (x > w1.b) {
					w1.b = w0.a;
				}
			}
			else {
				w0.b = min(w0.b, x);
			}
		}
	}
	else {
		if (x < w1.a + EPS) {
			if (w0.b < w1.b) {
				w0.b = w1.a;
			}
		}
		else {
			if (w1.b < w0.b) {
				w1.a = x;
			}
			else {
				w1.a = min(x, w0.b);
				w0.b = min(x, w0.b);
			}
		}
	}
	w0.isUseful = ((w0.b - w0.a) > EPS);
	w1.isUseful = ((w1.b - w1.a) > EPS);
	//	w0.isUseful = (w0.b > w0.a);
	//	w1.isUseful = (w1.b > w1.a);
}

__device__ inline int sign(float x) {
	return x < 0 ? -1 : 1;
}
__device__ inline bool checkAns(Windows &w0, Windows &w1, float ans) {
	return fabs(getDistance2D(w0.I, Point2D(ans, 0.0)) - getDistance2D(w1.I, Point2D(ans, 0.0)) - w1.d + w0.d) < EPS;
}

__device__ void new_Cut(Windows &w0, Windows &w1, float &len) {
	if (!w0.isUseful || !w1.isUseful) return;
	if (w1.b < w0.a + EPS || w0.b < w1.a + EPS) return;
	/*
	if (fabs(w0.I.x - w1.I.x) < EPS && fabs(w0.I.y - w1.I.y) < EPS && fabs(w0.d - w1.d) < EPS) {
	w0.a = min(w0.a, w1.a);
	w0.b = max(w0.b, w1.b);
	w1.isUseful = false;
	return;
	}
	*/

	float a = w1.I.x - w0.I.x, b = w1.d - w0.d, c;
	b *= b;
	c = w0.I.x*w0.I.x + w0.I.y*w0.I.y - w1.I.x*w1.I.x - w1.I.y*w1.I.y - b;
	float A = a*a - b, B = c*a + 2 * w1.I.x*b, C = 0.25*c*c - (w1.I.x*w1.I.x + w1.I.y*w1.I.y)*b;
	if (fabs(A) < EPS) {
		c = -C / B;
		if (checkAns(w0, w1, c)) {
			checkCutPoint(w0, w1, c);
			checkWindows_YH(w0, len);
			checkWindows_YH(w1, len);
		}
		return;
	}
	else {
		float V = B*B - 4 * A*C;
		if (V < 0) return;
		c = -B / (2 * A);
		if (fabs(V) < EPS && checkAns(w0, w1, c)) {
			checkCutPoint(w0, w1, c);
			checkWindows_YH(w0, len);
			checkWindows_YH(w1, len);
			return;
		}
		c = (-B - sqrt(V)) / (2 * A), C = (-B + sqrt(V)) / (2 * A);
		bool L = checkAns(w0, w1, c), R = checkAns(w0, w1, C);
		if (L && R) {
			checkCutPoint(w0, w1, c, C, w1.d < w0.d);
		}
		else if (R) {
			checkCutPoint(w0, w1, C);
		}
		else if (L) {
			checkCutPoint(w0, w1, c);
		}
		checkWindows_YH(w0, len);
		checkWindows_YH(w1, len);
	}
}


__device__ inline void cilp(Windows &w1, Windows &w2, bool isLeft) {
	if (w2.a < w1.b + EPS) return;
	if (w2.b > w1.b) {
		if (isLeft) w2.a = w1.b;
		else w1.b = w2.a;
	}
	else {
		if (isLeft) w2.a = w2.b;
	}
	w2.isUseful = (w2.b > w2.a);
	w1.isUseful = (w1.b > w1.a);
}

/*
__device__ void Cutmmp_new(Windows &w1, Windows &w2) {
bool isx = fabs(w1.I.x - w2.I.x) < EPS, isy = fabs(w1.I.y - w2.I.y) < EPS, isd = fabs(w1.d - w2.d) < EPS;
if (isx && isy) {
cilp(w1, w2, w2.d > w1.d - EPS);
return;
}
if (isd && isx && !isy) {
cilp(w1, w2, w2.I.y < w1.I.y);
return;
}
if (isd && isy && !isx) {
if (w1.I.x < w2.I.x) checkCutPoint(w1, w2, 0.5*(w1.I.x + w2.I.x));
else checkCutPoint(w2, w1, 0.5*(w1.I.x + w2.I.x));
return;
}
if (isd && !isy && !isx) {
if (w1.I.x < w2.I.x) checkCutPoint(w1, w2, 0.5*(w1.I.x + w2.I.x) + (w2.I.y*w2.I.y - w1.I.y*w1.I.y) / (2 * (w2.I.x - w1.I.x)));
else checkCutPoint(w2, w1, 0.5*(w1.I.x + w2.I.x) + (w2.I.y*w2.I.y - w1.I.y*w1.I.y) / (2 * (w2.I.x - w1.I.x)));
return;
}
if (w1.I.x < w2.I.x) new_Cut(w1, w2);
else new_Cut(w2, w1);
}
*/
__device__ __host__ struct New_Windows_Save {
	uchar len;
	Windows buf[SIZE_WINDOWS_HE];
};

__global__ void calCntBySource(int sid, HalfEdge *halfEdges, int *outgoing, int *sum) {
	int eid, id = outgoing[sid], now_id;
	eid = id;
	do {
		now_id = idOfNextHalfEdge(eid);
		sum[now_id] = 1;
		eid = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge;
	} while (eid != id);
}

__device__ inline void updateByWindowsInEdge_YH(Windows *wins, int &address, int &size, float &new_occupy, bool isStart, float &len) {
	if (size == 0) return;
	int lim = address + size;
	for (int i = address; i < lim; i++) {
		Windows win = wins[i];
		if (!win.isUseful || win.isLeft) continue;
		if (!isStart && win.a < EPS && win.lLen + win.d < new_occupy) {
			new_occupy = win.d + win.lLen;
		}
		else if (isStart && fabs(win.b - len) < EPS && win.rLen + win.d < new_occupy) {
			new_occupy = win.d + win.rLen;
		}
	}
}

__device__ void updateByWindowsInEdge_YH2(Windows *wins, int &address, int &size, float &new_occupy, bool isStart, float &len) {
	if (size == 0) return;
	int lim = address + size;
	for (int i = address; i < lim; i++) {
		Windows win = wins[i];
		if (!win.isUseful) continue;
		if (!isStart) {
			float tmp = win.d + win.a + getDistance2D(win.I, Point2D(win.a, 0));
			if (new_occupy - EPS > tmp)
				new_occupy = tmp;
		}
		else if (isStart) {
			float tmp = win.d - win.b + getDistance2D(win.I, Point2D(win.b, 0)) + len;
			if (new_occupy - EPS > tmp)
				new_occupy = tmp;
		}
	}
}

__device__ inline bool checkId(int eid, int *sum) {
	if (sum[eid] == 0 || (eid != 0 && sum[eid] - sum[eid - 1] == 0)) return false;
	return true;
}

__global__ void calCntByPoint_YH(HalfEdge *halfEdges, int *outgoing, WindowsInAng *ang, bool *isN, float *dist, int limit, int *read_sum, bool *isU, int *address_w, float *dist_s, float *dist_e) {
	int pid = BID*BMX + TID;
	if (pid >= limit) return;
	if (isU[pid]) {
		int id = outgoing[pid];
		int eid = id;
		float new_occupy = dist[pid];
		bool is = isN[pid];
		do {
			if (checkId(eid, read_sum)) {
				new_occupy = min(new_occupy, dist_s[eid]);
				//updateByWindowsInEdge_YH(windowsInEdge, address[eid], lenOfWindowsInEdge[eid], new_occupy, false, halfEdges[eid].length);
			}
			eid = idOfNextHalfEdge(eid);
			if (is && new_occupy > ang[eid].val) {
				new_occupy = ang[eid].val;
			}

			address_w[eid] = 1;

			eid = idOfNextHalfEdge(eid);

			if (checkId(eid, read_sum)) {
				new_occupy = min(new_occupy, dist_e[eid]);
				//updateByWindowsInEdge_YH(windowsInEdge, address[eid], lenOfWindowsInEdge[eid], new_occupy, true, halfEdges[eid].length);
			}
			eid = halfEdges[eid].idOfOppositeEdge;
		} while (eid != id);
		if (new_occupy < dist[pid] - EPS) {
			dist[pid] = new_occupy;
		}
		else isU[pid] = false;
		//if (!is) isU[pid] = false;
	}
}

__global__ void calCntByPoint_YHMSD(HalfEdge *halfEdges, int *outgoing, int limit, bool *isU, int *address_w) {
	int pid = BID*BMX + TID;
	if (pid >= limit) return;
	if (isU[pid]) {
		int id = outgoing[pid];
		int eid = id;
		//printf("[Hacb] %d\n", pid);
		do {
			eid = idOfNextHalfEdge(eid);

			address_w[eid] = 1;

			eid = idOfNextHalfEdge(eid);

			eid = halfEdges[eid].idOfOppositeEdge;
		} while (eid != id);
	}
}

__device__ inline void updatePoint(float *dist, bool *isU, HalfEdge *halfEdge, int &eid, Windows &wi, float &dist_s, float &dist_e) {
	if (wi.a < EPS) {
		int pid = halfEdge[eid].idOfStartPoint;
		if (wi.lLen + wi.d < dist[pid] - EPS) {
			isU[pid] = true;
			/*if (wi.lLen + wi.d < dist_s[eid]) {
			dist_s[eid] = wi.lLen + wi.d;
			}*/
			dist_s = min(dist_s, wi.lLen + wi.d);
		}
	}
	if (fabs(wi.b - halfEdge[eid].length) < EPS) {
		int pid = halfEdge[idOfNextHalfEdge(eid)].idOfStartPoint;
		if (wi.rLen + wi.d < dist[pid] - EPS) {
			dist_e = min(dist_e, wi.rLen + wi.d);
			isU[pid] = true;
			/*
			if (wi.rLen + wi.d < dist_e[eid]) {
			dist_e[eid] = wi.rLen + wi.d;
			}*/
		}
	}
}

__global__ void createBySource_YH(int sid, HalfEdge *halfEdges, int *outgoing, Windows *windowsInEdge, float *dist, int *address, int *lenOfWindowsInEdge, bool *isU, float *dist_s, float *dist_e, float *angVal) {
	int eid, id = outgoing[sid], now_id;
	eid = id;
	do {
		now_id = idOfNextHalfEdge(eid);
		Windows wi = Windows((float)0, (float)0, halfEdges[now_id].length,
			halfEdges[eid].length, halfEdges[idOfPreHalfEdge(eid)].length, halfEdges[now_id].length);
		lenOfWindowsInEdge[now_id] = 0;
		dist_s[now_id] = INF;
		dist_e[now_id] = INF;
		angVal[now_id] = 0;
		if (checkWindows_PCH(wi, dist, halfEdges, now_id)) {
			updatePoint(dist, isU, halfEdges, now_id, wi, dist_s[now_id], dist_e[now_id]);
			windowsInEdge[address[now_id]] = wi;
			lenOfWindowsInEdge[now_id] = 1;
		}
		eid = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge;
	} while (eid != id);
	return;
}


__global__ void calCntByEdge_YH(int *read_sum, int *sum, HalfEdge *halfEdges, int limits, int *address, int *lenOfWindowsInEdge, int *address1, Windows *wins) {
	int id = BID*BMX + TID;
	if (id >= limits) return;
	if (checkId(id, read_sum)) {
		if (lenOfWindowsInEdge[id] > 0) {
			address1[id] += lenOfWindowsInEdge[id];
			int idO = halfEdges[id].idOfOppositeEdge;
			sum[idOfNextHalfEdge(idO)] = 1;
			sum[idOfPreHalfEdge(idO)] = 1;
			sum[id] = 1;
		}
	}
	int eid = halfEdges[idOfNextHalfEdge(id)].idOfOppositeEdge;
	if (checkId(eid, read_sum)) {
		address1[id] += lenOfWindowsInEdge[eid];
	}
	eid = halfEdges[idOfPreHalfEdge(id)].idOfOppositeEdge;
	if (checkId(eid, read_sum)) {
		address1[id] += lenOfWindowsInEdge[eid];
	}
}

__global__ void calCntByEdge_Init(int *Eid, int *lcnt, int *rcnt, int *address_w, int limit, HalfEdge *halfEdges, Point2D *angOnEdge,
	Windows *windowsInEdge, int *address_r, int *lenOfWindows, float avg) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int size = lenOfWindows[eid];
	if (size == 0) return;
	size += address_r[eid];
	int lc, rc, s, idOfOp = halfEdges[eid].idOfOppositeEdge;
	lc = rc = s = 0;
	Point2D C = angOnEdge[idOfOp];
	for (int i = address_r[eid]; i < size; i++) {
		Windows win = windowsInEdge[i];
		if (!win.isUseful) continue;
		if (win.val > avg) {
			s++;
			continue;
		}
		float x = (C.x - win.I.x)*win.I.y / (win.I.y - C.y) + win.I.x;
		if (x < win.a + EPS) {
			rc++;
		}
		else if (win.b < x + EPS) {
			lc++;
		}
		else {
			rc++;
			lc++;
		}
	}
	address_w[eid] += s;

	lcnt[idOfNextHalfEdge(idOfOp)] += lc;
	rcnt[idOfPreHalfEdge(idOfOp)] += rc;
}

__global__ void calCntByEdge_Init_Notbq(int *Eid, int *lcnt, int *rcnt, int limit, HalfEdge *halfEdges, Point2D *angOnEdge,
	Windows *windowsInEdge, int *address_r, int *lenOfWindows) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int size = lenOfWindows[eid];
	if (size == 0) return;
	size += address_r[eid];
	int lc, rc, idOfOp = halfEdges[eid].idOfOppositeEdge;
	lc = rc = 0;
	Point2D C = angOnEdge[idOfOp];
	for (int i = address_r[eid]; i < size; i++) {
		Windows win = windowsInEdge[i];
		if (!win.isUseful) continue;
		float x = (C.x - win.I.x)*win.I.y / (win.I.y - C.y) + win.I.x;
		if (x < win.a + EPS) {
			rc++;
		}
		else if (win.b < x + EPS) {
			lc++;
		}
		else {
			rc++;
			lc++;
		}
	}
	lcnt[idOfNextHalfEdge(idOfOp)] += lc;
	rcnt[idOfPreHalfEdge(idOfOp)] += rc;
}


__global__ void updateAddress(int *lcnt, int *cuda_sum, int *address_w, int limit) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	address_w[eid] += lcnt[eid] + cuda_sum[eid];
	if (address_w[eid] > 0) cuda_sum[eid] = 1;
	else cuda_sum[eid] = 0;
}

__global__ void pointEven_New_YH(HalfEdge *halfEdges, int *outgoing, Windows *windowsInEdge_r, Windows *windowsInEdge_w, //int *sum_w,
	int *address, int *lenOfWindows, int *address1, int *lenOfWindows1, float *dist, int limit, bool *isU) {
	int pid = BID*BMX + TID;
	if (pid >= limit) return;
	int eid, id = outgoing[pid], now_id;
	eid = id;
	bool is = isU[pid];
	if (is) {
		do {
			now_id = idOfNextHalfEdge(eid);
			//if (checkId(now_id, sum_w)) {
			Windows wi = Windows(dist[pid], (float)0, halfEdges[now_id].length,
				halfEdges[eid].length, halfEdges[idOfPreHalfEdge(eid)].length, halfEdges[now_id].length);
			if (checkWindows_PCH(wi, dist, halfEdges, now_id)) {
				windowsInEdge_w[address1[now_id]] = wi;
				lenOfWindows1[now_id] = 1;
			}
			//}
			eid = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge;
		} while (eid != id);
		isU[pid] = false;
	}
}

__global__ void pointEven_New_YHRH(HalfEdge *halfEdges, int *outgoing, Windows *windowsInEdge_r, Windows *windowsInEdge_w, //int *sum_w,
	int *address, int *lenOfWindows, int *address1, int *lenOfWindows1, float *dist, int limit, bool *isU, int *read_sum, bool *isN, float *dist_s, float *dist_e, float *ang) {
	int pid = BID*BMX + TID;
	if (pid >= limit) return;
	if (isU[pid]) {
		int id = outgoing[pid];
		int eid = id;
		float new_occupy = dist[pid];
		bool is = isN[pid];
		do {
			if (checkId(eid, read_sum)) {
				new_occupy = min(new_occupy, dist_s[eid]);
				//updateByWindowsInEdge_YH(windowsInEdge, address[eid], lenOfWindowsInEdge[eid], new_occupy, false, halfEdges[eid].length);
			}
			eid = idOfNextHalfEdge(eid);
			if (is && new_occupy > ang[eid]) {
				new_occupy = ang[eid];
			}

			eid = idOfNextHalfEdge(eid);

			if (checkId(eid, read_sum)) {
				new_occupy = min(new_occupy, dist_e[eid]);
				//updateByWindowsInEdge_YH(windowsInEdge, address[eid], lenOfWindowsInEdge[eid], new_occupy, true, halfEdges[eid].length);
			}
			eid = halfEdges[eid].idOfOppositeEdge;
		} while (eid != id);
		if (new_occupy < dist[pid] - EPS) {
			dist[pid] = new_occupy;
			eid = id = outgoing[pid];
			do {
				int now_id = idOfNextHalfEdge(eid);
				//if (checkId(now_id, sum_w)) {
				Windows wi = Windows(dist[pid], (float)0, halfEdges[now_id].length,
					halfEdges[eid].length, halfEdges[idOfPreHalfEdge(eid)].length, halfEdges[now_id].length);
				if (checkWindows_PCH(wi, dist, halfEdges, now_id)) {
					windowsInEdge_w[address1[now_id]] = wi;
					lenOfWindows1[now_id] = 1;
				}
				//}
				eid = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge;
			} while (eid != id);
		}
		isU[pid] = false;
	}
}

__global__ void createWindowsByWindows_VTP_New_pq_YH(HalfEdge *halfEdges, float *dist, Windows *windowsInEdge, Windows *creatingInEdgeLeft, Windows *creatingInEdgeRight, int *address, int *lenOfWindows, int *lenOfCreatingLeft, int *lenOfCreatingRight,
	WindowsInAng *ang, Point2D *angOnEdge, int limit, int *Eid, bool *isU, Windows *windows_w, int *address_w, int *lenOfWindows_w, float avg, bool *isN) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int size = lenOfWindows[eid];
	lenOfCreatingLeft[eid] = lenOfCreatingRight[eid] = 0;
	if (size == 0) return;
	size += address[eid];
	int idOfOp = halfEdges[eid].idOfOppositeEdge;
	WindowsInAng now_ang = ang[idOfOp];
	Point2D C = angOnEdge[idOfOp];
	int size_0, size_1, ts = lenOfWindows_w[eid] + address_w[eid], add = address[eid];
	size_0 = size_1 = add;
	for (int i = add; i < size; i++) {
		Windows win = windowsInEdge[i];

		if (!win.isUseful) {
			continue;
		}
		if (!checkWindows_PCH(win, dist, halfEdges, eid)) {
			continue;
			//win.isUseful = false;
}
		if (win.val > avg) {
#if WU
			float len = halfEdges[eid].length;
			for (int j = address_w[eid]; j < ts; j++) {
				if (windows_w[j].isUseful) {
					if (windows_w[j].I.x < win.I.x) {
						new_Cut(windows_w[j], win, len);
					}
					else new_Cut(win, windows_w[j], len);
				}
				if (win.isUseful == false) break;
			}
#endif
			if (win.isUseful) win.isLeft = true, windows_w[ts++] = win;
			continue;
		}

		win.isLeft = false;
		float x = (C.x - win.I.x)*win.I.y / (win.I.y - C.y) + win.I.x;
		if (x < win.a + EPS) {
			creatingInEdgeRight[size_1++] = win;//createSonByWindows(win, C, true, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
		}
		else if (win.b < x + EPS) {
			creatingInEdgeLeft[size_0++] = win;// createSonByWindows(win, C, false, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
		}
		else {
			WindowsInAng tm = WindowsInAng(getDistance2D(win.I, C) + win.d, win.I);
			//float tm = getDistance2D(win.I, C) + win.d;
			if (tm < now_ang) {
				now_ang = tm;
				float a = win.a;
				win.a = x;
				creatingInEdgeRight[size_1++] = win; //createSonByWindows(win, C, true, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
				win.a = a;
				a = win.b;
				win.b = x;
				creatingInEdgeLeft[size_0++] = win; //createSonByWindows(win, C, false, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
				win.b = a;
			}
			else {
				float x1 = (C.x - now_ang.I.x)*now_ang.I.y / (now_ang.I.y - C.y) + now_ang.I.x;
				if (x < x1) {//if (getDeleteSonByWindows_PCH(win, C, halfEdges, idOfOp)) {
					win.a = x;
					creatingInEdgeRight[size_1++] = win;
				}
				else {
					win.b = x;
					creatingInEdgeLeft[size_0++] = win;
				}
			}
		}
	}
	lenOfWindows_w[eid] = ts - address_w[eid];

	//QSort(creatingInEdgeLeft, add, size_0 - 1);
	//QSort(creatingInEdgeRight, add, size_1 - 1);
	/*
	for (int i = add, j = i + 1; j < size_0;) {
	if (checkCase10(creatingInEdgeLeft[i], creatingInEdgeLeft[j], C)) {
	j++;
	}
	else i = j, j++;
	}
	//*/
	//*
	ts = size_0;
	size_0 = add;
	for (int i = size_0; i < ts; i++) {
		Windows wi = creatingInEdgeLeft[i];
		if (createSonByWindows_VTP(wi, C, false, halfEdges, dist, idOfOp)) {
			creatingInEdgeLeft[size_0++] = wi;
		}
	}
	//*/
	for (int i = add, j = i + 1; j < size_0;) {
		uchar res = checkCase123(creatingInEdgeLeft[i], creatingInEdgeLeft[j]);
		if (res == 0) {
			if (i == address[eid]) i = j, j++;
			else --i;
		}
		else if (res == 1) {
			j++;
		}
		else i = j, j++;
	}
	/*
	for (int i = size_1 - 1, j = i - 1; j >= add;) {
	if (checkCase10_f(creatingInEdgeRight[j], creatingInEdgeRight[i], C)) {
	j--;
	}
	else i = j, j--;
	}
	*/
	ts = size_1;
	size_1 = add;
	for (int i = size_1; i < ts; i++) {
		Windows wi = creatingInEdgeRight[i];
		if (createSonByWindows_VTP(wi, C, true, halfEdges, dist, idOfOp)) {
			creatingInEdgeRight[size_1++] = wi;
		}
	}

	for (int i = add, j = i + 1; j < size_1;) {
		uchar res = checkCase123(creatingInEdgeRight[i], creatingInEdgeRight[j]);
		if (res == 0) {
			if (i == address[eid]) i = j, j++;
			else --i;
		}
		else if (res == 1) {
			j++;
		}
		else i = j, j++;
	}
	float len = halfEdges[eid].length;
	//*
	for (int i = add; i < size_0; i++) {
		if (!creatingInEdgeLeft[i].isUseful) continue;
		for (int j = add; j < i; j++) {
			if (!creatingInEdgeLeft[j].isUseful) continue;
			if (creatingInEdgeLeft[i].I.x < creatingInEdgeLeft[j].I.x) {
				new_Cut(creatingInEdgeLeft[i], creatingInEdgeLeft[j], len);
			}
			else {
				new_Cut(creatingInEdgeLeft[j], creatingInEdgeLeft[i], len);
			}
			if (!creatingInEdgeLeft[i].isUseful) break;
		}
	}
	for (int i = add; i < size_1; i++) {
		if (!creatingInEdgeRight[i].isUseful) continue;
		for (int j = add; j < i; j++) {
			if (!creatingInEdgeRight[j].isUseful) continue;
			if (creatingInEdgeRight[i].I.x < creatingInEdgeRight[j].I.x) {
				new_Cut(creatingInEdgeRight[i], creatingInEdgeRight[j], len);
			}
			else {
				new_Cut(creatingInEdgeRight[j], creatingInEdgeRight[i], len);
			}
			if (!creatingInEdgeRight[i].isUseful) break;
		}
	}
	//*/
	lenOfCreatingLeft[eid] = size_0 - add;
	lenOfCreatingRight[eid] = size_1 - add;

	if (now_ang < ang[idOfOp]) {
		ang[idOfOp] = now_ang;
		if (isN[halfEdges[idOfPreHalfEdge(idOfOp)].idOfStartPoint])
			isU[halfEdges[idOfPreHalfEdge(idOfOp)].idOfStartPoint] = true;
	}

}

__global__ void createWindowsByWindows_VTP_New_pq_YH_Notbq(HalfEdge *halfEdges, float *dist, Windows *windowsInEdge, Windows *creatingInEdgeLeft, Windows *creatingInEdgeRight, int *address, int *lenOfWindows, int *lenOfCreatingLeft, int *lenOfCreatingRight,
	WindowsInAng *ang, Point2D *angOnEdge, int limit, int *Eid, bool *isU, Windows *windows_w, int *address_w, int *lenOfWindows_w, bool *isN) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int size = lenOfWindows[eid];
	lenOfCreatingLeft[eid] = lenOfCreatingRight[eid] = 0;
	if (size == 0) return;
	size += address[eid];
	int idOfOp = halfEdges[eid].idOfOppositeEdge;
	WindowsInAng now_ang = ang[idOfOp];
	Point2D C = angOnEdge[idOfOp];
	int size_0, size_1, ts = lenOfWindows_w[eid] + address_w[eid], add = address[eid];
	size_0 = size_1 = add;
	for (int i = add; i < size; i++) {
		Windows win = windowsInEdge[i];
		if (!win.isUseful) {
			continue;
		}
		if (!checkWindows_PCH(win, dist, halfEdges, eid)) continue;
		win.isLeft = false;
		float x = (C.x - win.I.x)*win.I.y / (win.I.y - C.y) + win.I.x;
		if (x < win.a + EPS) {
			creatingInEdgeRight[size_1++] = win;//createSonByWindows(win, C, true, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
		}
		else if (win.b < x + EPS) {
			creatingInEdgeLeft[size_0++] = win;// createSonByWindows(win, C, false, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
		}
		else {
			WindowsInAng tm = WindowsInAng(getDistance2D(win.I, C) + win.d, win.I);
			//float tm = getDistance2D(win.I, C) + win.d;
			if (tm < now_ang) {
				now_ang = tm;
				float a = win.a;
				win.a = x;
				creatingInEdgeRight[size_1++] = win; //createSonByWindows(win, C, true, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
				win.a = a;
				a = win.b;
				win.b = x;
				creatingInEdgeLeft[size_0++] = win; //createSonByWindows(win, C, false, halfEdges, dist, creatingInEdge[idOfOp], idOfOp);
				win.b = a;
			}
			else {
				float x1 = (C.x - now_ang.I.x)*now_ang.I.y / (now_ang.I.y - C.y) + now_ang.I.x;
				if (x < x1) {//if (getDeleteSonByWindows_PCH(win, C, halfEdges, idOfOp)) {
					win.a = x;
					creatingInEdgeRight[size_1++] = win;
				}
				else {
					win.b = x;
					creatingInEdgeLeft[size_0++] = win;
				}
			}
		}
	}
	lenOfWindows_w[eid] = ts - address_w[eid];

	ts = size_0;
	size_0 = add;
	for (int i = size_0; i < ts; i++) {
		Windows wi = creatingInEdgeLeft[i];
		if (createSonByWindows_VTP(wi, C, false, halfEdges, dist, idOfOp)) {
			creatingInEdgeLeft[size_0++] = wi;
		}
	}
	//*/
	for (int i = add, j = i + 1; j < size_0;) {
		uchar res = checkCase123(creatingInEdgeLeft[i], creatingInEdgeLeft[j]);
		if (res == 0) {
			if (i == address[eid]) i = j, j++;
			else --i;
		}
		else if (res == 1) {
			j++;
		}
		else i = j, j++;
	}

	ts = size_1;
	size_1 = add;
	for (int i = size_1; i < ts; i++) {
		Windows wi = creatingInEdgeRight[i];
		if (createSonByWindows_VTP(wi, C, true, halfEdges, dist, idOfOp)) {
			creatingInEdgeRight[size_1++] = wi;
		}
	}

	for (int i = add, j = i + 1; j < size_1;) {
		uchar res = checkCase123(creatingInEdgeRight[i], creatingInEdgeRight[j]);
		if (res == 0) {
			if (i == address[eid]) i = j, j++;
			else --i;
		}
		else if (res == 1) {
			j++;
		}
		else i = j, j++;
	}
	float len = halfEdges[eid].length;
	//*
	for (int i = add; i < size_0; i++) {
		if (!creatingInEdgeLeft[i].isUseful) continue;
		for (int j = add; j < i; j++) {
			if (!creatingInEdgeLeft[j].isUseful) continue;
			if (creatingInEdgeLeft[i].I.x < creatingInEdgeLeft[j].I.x) {
				new_Cut(creatingInEdgeLeft[i], creatingInEdgeLeft[j], len);
			}
			else {
				new_Cut(creatingInEdgeLeft[j], creatingInEdgeLeft[i], len);
			}
			if (!creatingInEdgeLeft[i].isUseful) break;
		}
	}
	for (int i = add; i < size_1; i++) {
		if (!creatingInEdgeRight[i].isUseful) continue;
		for (int j = add; j < i; j++) {
			if (!creatingInEdgeRight[j].isUseful) continue;
			if (creatingInEdgeRight[i].I.x < creatingInEdgeRight[j].I.x) {
				new_Cut(creatingInEdgeRight[i], creatingInEdgeRight[j], len);
			}
			else {
				new_Cut(creatingInEdgeRight[j], creatingInEdgeRight[i], len);
			}
			if (!creatingInEdgeRight[i].isUseful) break;
		}
	}
	//*/
	lenOfCreatingLeft[eid] = size_0 - add;
	lenOfCreatingRight[eid] = size_1 - add;

	if (now_ang < ang[idOfOp]) {
		ang[idOfOp] = now_ang;
		if (isN[halfEdges[idOfPreHalfEdge(idOfOp)].idOfStartPoint])
			isU[halfEdges[idOfPreHalfEdge(idOfOp)].idOfStartPoint] = true;
	}

}

__device__ void createSonByWindows(Windows &wi, Point2D &C, bool isRight, HalfEdge *halfEdges, float *dist, Windows *creatingInEdge, int &idOfOpp, int &address) {
	float len = 0.0;
	if (wi.I.y < EPS && (wi.I.x < wi.a + EPS || wi.I.x > wi.b - EPS)) return;
	if (fabs(wi.a - wi.b) < EPS) return;
	if (isRight) len = halfEdges[idOfOpp].length;
	Point2D A, B;
	if (fabs(wi.I.y) < EPS && fabs(C.y) >= EPS) {
		A = Point2D(wi.a, C.y*(wi.a - len) / (C.x - len));
		B = Point2D(wi.b, C.y*(wi.b - len) / (C.x - len));
	}
	else if (fabs(wi.I.y) >= EPS && fabs(C.y) < EPS) {
		A = Point2D(wi.a, C.y);
		B = Point2D(wi.b, C.y);
	}
	else {
		float ty = wi.I.y*C.y*(len - wi.a) / (C.y*(wi.I.x - wi.a) - wi.I.y*(C.x - len));
		A = Point2D(ty*(wi.I.x - wi.a) / wi.I.y + wi.a, ty);
		ty = wi.I.y*C.y*(len - wi.b) / (C.y*(wi.I.x - wi.b) - wi.I.y*(C.x - len));
		B = Point2D(ty*(wi.I.x - wi.b) / wi.I.y + wi.b, ty);
	}
	if (isRight) {
		Windows nw = Windows(wi.d, getDistance2D(C, A), getDistance2D(C, B),
			getDistance2D(C, wi.I), wi.rLen, halfEdges[idOfPreHalfEdge(idOfOpp)].length, 1);
		if (checkWindows_PCH(nw, dist, halfEdges, idOfPreHalfEdge(idOfOpp))) {
			creatingInEdge[address++] = nw;
		}
	}
	else {
		Windows nw = Windows(wi.d, getDistance2D(A, Point2D(0.0, 0.0)), getDistance2D(B, Point2D(0.0, 0.0)),
			wi.lLen, getDistance2D(C, wi.I), halfEdges[idOfNextHalfEdge(idOfOpp)].length, 0);
		if (checkWindows_PCH(nw, dist, halfEdges, idOfNextHalfEdge(idOfOpp))) {
			creatingInEdge[address++] = nw;
	}
}
	return;
}

__global__ void readWindowsOnEdge_VTP_New_YH(HalfEdge *halfEdges, Windows *creatingInEdgeLeft, Windows *creatingInEdgeRight, Windows *windowsInEdge_w, int *address, int *address_w, int *lenOfWindows_w, int *lenOfWindowsLeft,
	int *lenOfWindowsRight, int *sum_r, int limit, int *Eid, float *dist, bool *isU, float *dist_win, float AVG, float *dist_s, float *dist_e) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
#if TEST_AVG
	//sum_win[eid] = 0;
	dist_win[eid] = 0;
#endif // TEST_AVG

#if TEST_MAX_MIN
	sum_win[eid] = INF;
	dist_win[eid] = INF;
#endif
	eid = Eid[eid];

	int pre_id = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge, next_id = halfEdges[idOfNextHalfEdge(eid)].idOfOppositeEdge;
	int size_0, size_1;
	size_0 = size_1 = 0;
	int add = address_w[eid];
	int size = add + lenOfWindows_w[eid];
	if (checkId(pre_id, sum_r)) {
		size_0 = lenOfWindowsLeft[pre_id];
	}
	if (checkId(next_id, sum_r)) {
		size_1 = lenOfWindowsRight[next_id];
	}

	if (size_0 == 0 && size_1 == 0) {
		//*
		float tm_s = INF;
		float tm_e = INF;
		size_0 = add;
		int tid = BID*BMX + TID;
		for (int i = add; i < size; i++) {
			Windows wi = windowsInEdge_w[i];
			if (!wi.isUseful) {
				continue;
			}
			windowsInEdge_w[size_0++] = wi;
			if (wi.isLeft == false)
				updatePoint(dist, isU, halfEdges, eid, wi, tm_s, tm_e);
#if TEST_AVG
			dist_win[tid] += wi.val;
#endif

#if TEST_MAX_MIN
			dist_win[tid] = min(dist_win[tid], wi.val);
			sum_win[tid] = min(sum_win[tid], -wi.val);
#endif

		}
		//dist_win[tid] = tmp;
#if TEST_AVG
		//sum_win[tid] = size_0 - add;
#endif
		lenOfWindows_w[eid] = size_0 - add;
		dist_s[eid] = tm_s;
		dist_e[eid] = tm_e;
		return;
	}//*
#if CUT
	size_0 += address[pre_id], size_1 += address[next_id];
	//*
	float len = halfEdges[eid].length;
	for (int i = address[pre_id], j = address[next_id]; i < size_0 || j < size_1; i++, j++) {
		if (j < size_1 && creatingInEdgeRight[j].isUseful) {
			//*
			Windows win = creatingInEdgeRight[j];
			for (int k = add; k < size; k++) {
				if (windowsInEdge_w[k].isUseful && !windowsInEdge_w[k].isLeft) {
					if (windowsInEdge_w[k].I.x < win.I.x) {
						new_Cut(windowsInEdge_w[k], win, len);
					}
					else new_Cut(win, windowsInEdge_w[k], len);
				}
				if (win.isUseful == false) break;
			}
			if (win.isUseful) windowsInEdge_w[size++] = win;
			//*/
			//windowsInEdge_w[size++] = creatingInEdgeRight[j];
		}
		if (i < size_0 && creatingInEdgeLeft[i].isUseful) {
			//*
			Windows win = creatingInEdgeLeft[i];
			for (int k = add; k < size; k++) {
				if (windowsInEdge_w[k].isUseful && !windowsInEdge_w[k].isLeft) {
					if (windowsInEdge_w[k].I.x < win.I.x) {
						new_Cut(windowsInEdge_w[k], win, len);
					}
					else new_Cut(win, windowsInEdge_w[k], len);
				}
				if (win.isUseful == false) break;
			}
			if (win.isUseful) windowsInEdge_w[size++] = win;
			//*/
			//windowsInEdge_w[size++] = creatingInEdgeLeft[i];
		}
	}
	/*
	for (int i = add; i < size; i++) {
	if (windowsInEdge_w[i].isLeft) continue;
	for (int j = add; j < i && windowsInEdge_w[i].isUseful; j++) {
	if (windowsInEdge_w[j].isUseful == false || windowsInEdge_w[j].isLeft) continue;
	if (windowsInEdge_w[i].I.x < windowsInEdge_w[j].I.x) {
	new_Cut(windowsInEdge_w[i], windowsInEdge_w[j], len);
	}
	else {
	new_Cut(windowsInEdge_w[j], windowsInEdge_w[i], len);
	}
	}
	}//*/
#endif
	//*
	float tm_s = INF;
	float tm_e = INF;
	size_0 = add;
	int tid = BID*BMX + TID;
	for (int i = add; i < size; i++) {
		Windows wi = windowsInEdge_w[i];
		if (!wi.isUseful) continue;
		windowsInEdge_w[size_0++] = wi;
		if (!wi.isLeft) updatePoint(dist, isU, halfEdges, eid, wi, tm_s, tm_e);
#if TEST_AVG
		dist_win[tid] += wi.val;
#endif

#if TEST_MAX_MIN
		dist_win[tid] = min(dist_win[tid], wi.val);
		sum_win[tid] = min(sum_win[tid], -wi.val);
#endif
	}
	//dist_win[tid] = tmp;
#if TEST_AVG
	//sum_win[tid] = size_0 - add;
#endif
	//*/
	//if (size_0 == 0) printf("sasdfaf\n");
	//sum_win[tid] = 1.0*size_0 - add;
	dist_s[eid] = tm_s;
	dist_e[eid] = tm_e;
	lenOfWindows_w[eid] = size_0 - add;
}

__global__ void readWindowsOnEdge_VTP_New_YH_Notbq(HalfEdge *halfEdges, Windows *creatingInEdgeLeft, Windows *creatingInEdgeRight, Windows *windowsInEdge_w, int *address, int *address_w, int *lenOfWindows_w, int *lenOfWindowsLeft,
	int *lenOfWindowsRight, int *sum_r, int limit, int *Eid, float *dist, bool *isU) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int pre_id = halfEdges[idOfPreHalfEdge(eid)].idOfOppositeEdge, next_id = halfEdges[idOfNextHalfEdge(eid)].idOfOppositeEdge;
	int size_0, size_1;
	size_0 = size_1 = 0;
	int add = address_w[eid];
	int size = add + lenOfWindows_w[eid];
	if (checkId(pre_id, sum_r)) {
		size_0 = lenOfWindowsLeft[pre_id];
	}
	if (checkId(next_id, sum_r)) {
		size_1 = lenOfWindowsRight[next_id];
	}

	if (size_0 == 0 && size_1 == 0) {
		//*
		size_0 = add;
		int tid = BID*BMX + TID;
		for (int i = add; i < size; i++) {
			Windows wi = windowsInEdge_w[i];
			if (!wi.isUseful) {
				continue;
			}
			windowsInEdge_w[size_0++] = wi;
			//	updatePoint(dist, isU, halfEdges, eid, wi);
		}
		lenOfWindows_w[eid] = size_0 - add;
		return;
	}//*

	size_0 += address[pre_id], size_1 += address[next_id];
	//*
	float len = halfEdges[eid].length;
	for (int i = address[pre_id], j = address[next_id]; i < size_0 || j < size_1; i++, j++) {
		if (j < size_1 && creatingInEdgeRight[j].isUseful) {
			//*
			Windows win = creatingInEdgeRight[j];
			for (int k = add; k < size; k++) {
				if (windowsInEdge_w[k].isUseful) {
					if (windowsInEdge_w[k].I.x < win.I.x) {
						new_Cut(windowsInEdge_w[k], win, len);
					}
					else new_Cut(win, windowsInEdge_w[k], len);
				}
				if (win.isUseful == false) break;
			}
			if (win.isUseful) windowsInEdge_w[size++] = win;
			//*/
			//windowsInEdge_w[size++] = creatingInEdgeRight[j];
		}
		if (i < size_0 && creatingInEdgeLeft[i].isUseful) {
			//*
			Windows win = creatingInEdgeLeft[i];
			for (int k = add; k < size; k++) {
				if (windowsInEdge_w[k].isUseful) {
					if (windowsInEdge_w[k].I.x < win.I.x) {
						new_Cut(windowsInEdge_w[k], win, len);
					}
					else new_Cut(win, windowsInEdge_w[k], len);
				}
				if (win.isUseful == false) break;
			}
			if (win.isUseful) windowsInEdge_w[size++] = win;
			//*/
			//windowsInEdge_w[size++] = creatingInEdgeLeft[i];
		}
	}

	//*
	size_0 = add;
	int tid = BID*BMX + TID;
	for (int i = add; i < size; i++) {
		Windows wi = windowsInEdge_w[i];
		if (!wi.isUseful) continue;
		windowsInEdge_w[size_0++] = wi;
		//		updatePoint(dist, isU, halfEdges, eid, wi);
		///		dist_win[tid] += cal(wi);
	}
	//sum_win[tid] = size_0 - add;
	//*/
	//if (size_0 == 0) printf("sasdfaf\n");

	lenOfWindows_w[eid] = size_0 - add;
}


__global__ void Tongji(Windows *windows, int *address, int *len, int *Eid, int limit, float AVG) {
	int cnt = 0;
	int block[102] = { 0 };
	//float mi = 0, mx = 1.6;
	float mi = INF, mx = 0;
	//float gap = (mx - mi) / 100;
	//int bc = (mx - mi) / gap + 1;
	for (int i = 0; i < limit; i++) {
		int eid = Eid[i];
		int lm = address[eid] + len[eid];
		for (int j = address[eid]; j < lm; j++) {
			if (windows[j].isUseful == false || windows[j].val > AVG) continue;
			float tm = windows[j].val;
			if (tm < mi) mi = tm;
			if (tm > mx) mx = tm;
			//int id = (tm - mi) / gap;
			//block[id]++;
		}
	}
	printf("%f\t%f\n", mx, mi);
	//if (cnt == 0) return;
	/*

	for (int i = 0; i < cnt; i++) {
	int id = (dist[i] - mi) / gap;
	block[id]++;
	}
	printf("%lf\t%lf\n", mx, mi);*/
	/*
	printf("\n");
	for (int i = 0; i <= bc; i++) {
	printf("%d\t", block[i]);
	}
	printf("\n");
	//*/
}

__global__ void getSum(int *sum, int lim, int gap, int *hz_sum) {
	int id = (TID + BMX*BID), i = id*gap;
	hz_sum[id] = 0;
	if (i >= lim) return;
	int lm = i + gap;
	if (lm > lim) lm = lim;
	for (i; i + 1 < lm; i++) {
		sum[i + 1] += sum[i];
	}
	hz_sum[id] = sum[i];
}

__global__ void getSum_pq_YH(int *sum, int lim, int gap, int *hz_sum, int *sum_win, int *hz_sum_win, float *sum_dist, float *hz_sum_dist, int *address1, int *hz_sum_address, int *test) {
	int id = (TID + BMX*BID), i = id*gap;
	//test[id] = 0;
	int tsum_win = 0;
	float tsum_dist = 0;
	int tsum = 0, taddress = 0;
	if (i < lim) {
		int lm = i + gap;
		if (lm > lim) lm = lim;
		//sum[i] = sum[i] < 0 ? 1 : 0;
		//int sum_t = 0;
		for (i; i < lm; i++) {
			//test[id]++;
			tsum = tsum + sum[i];
			sum[i] = tsum;

			//sum[i + 1] += sum[i];

			tsum_win = tsum_win + sum_win[i];
			//hz_sum_win[id] += sum_win[i];
			tsum_dist = tsum_dist + sum_dist[i];
			//hz_sum_dist[id] += sum_dist[i];
			taddress = taddress + address1[i];
			address1[i] = taddress;
			//address1[i + 1] += address1[i];
		}
	}

	hz_sum_address[id] = taddress;
	hz_sum[id] = tsum;
	hz_sum_win[id] = tsum_win;
	hz_sum_dist[id] = tsum_dist;
}

__global__ void getSum_hz_YH(int *sum, int lim, int gap, int *hz_sum, int *Eid, int *address) {
	int id = (TID + BMX*BID), i = id*gap;
	if (i >= lim) {
		return;
	}
	int s = id == 0 ? 0 : hz_sum[id - 1], lm = i + gap;
	if (lm > lim) lm = lim;
	//for (i = 0; i < id; i++) s += hz_sum[i];
	for (i; i < lm; i++) {
		sum[i] += s;
		if (sum[i] > s) {
			if (i == id*gap) {
				Eid[sum[i] - 1] = i;
				address[i] = s;
			}
			else if (sum[i] > sum[i - 1]) {
				Eid[sum[i] - 1] = i;
				address[i] = sum[i - 1];
			}
		}
	}
}

__global__ void getSum_hz_YH2(int *sum, int lim, int gap, int *hz_sum, int *Eid, int *address, int *hz_sum_ress) {
	int id = (TID + BMX*BID), tm = id*gap;// i = id*gap;
	if (tm >= lim) {
		return;
	}
	int s = id == 0 ? 0 : hz_sum[id - 1], lm = tm + gap;
	int tsum, pre_sum;
	int s_add = id == 0 ? 0 : hz_sum_ress[id - 1];
	int t_add, pre_add = s_add;
	if (lm > lim) lm = lim;
	//for (int i = tm, j = lm-1; i < lm; i++, j--) {
	for (int i = tm; i < lm; i++) {
		tsum = sum[i] + s;
		sum[i] = tsum;
		//sum[i] += s;
		t_add = s_add + address[i];
		address[i] = pre_add;
		pre_add = t_add;
		//if (j > tm) address[j] = address[j - 1] + s_add;
		//else address[j] = s_add;

		if (tsum > s) {
			if (i == tm) {
				Eid[tsum - 1] = i;
			}
			else if (tsum > pre_sum) {
				Eid[tsum - 1] = i;
			}
		}
		pre_sum = tsum;
	}
}


__global__ void getEidBySource(int * cuda_sum, int * address, int *Eid, int lim) {
	for (int i = 0; i < lim; i++) {
		address[i] = 0;
		if (i > 0) address[i] = cuda_sum[i - 1];
		if (cuda_sum[i] > 0) {
			if (i == 0 || cuda_sum[i] > cuda_sum[i - 1]) {
				Eid[cuda_sum[i] - 1] = i;
				//address[i] = cuda_sum[i] - 1;
			}
		}
	}
}

__global__ void getEid(int *cuda_sum, int *Eid, int lim) {
	int id = BID*BMX + TID;
	if (id < lim) {
		int ts = cuda_sum[id];
		if (ts > 0) {
			if (id == 0 || ts > cuda_sum[id - 1]) {
				Eid[ts - 1] = id;
			}
		}
	}
}

__global__ void getbiaozhuncha(Windows *windows, int *address, int *len, float *test_sum, float avg, int limit, int *Eid, int cnt) {
	int eid = BID*BMX + TID;
	if (eid >= limit) return;
	eid = Eid[eid];
	int size = len[eid] + address[eid];
	float sum = 0;
	for (int i = address[eid]; i < size; i++) {
		float tmp = windows[i].val;
		sum += (tmp - avg)*(tmp - avg);
	}

	test_sum[BID*BMX + TID] = sum;
}

__global__ void reduceAddress(int *Eid2, int *pointer_address1, int *copy_address, int limit) {
	int eid = TID + BID*BMX;
	if (eid >= limit) return;
	copy_address[eid] = pointer_address1[Eid2[eid]];
}

__global__ void copyReturn(int *Eid2, int *pointer_address1, int *copy_address, int limit) {
	int eid = TID + BID*BMX;
	if (eid >= limit) return;
	pointer_address1[Eid2[eid]] = copy_address[eid];
}

//float ZT_biao[10] = {0, -1.28, -0.84, -0.52, -0.35, 0, 0.35, 0.52, 0.84, 1.28};
float ZT_biao[] = { 0.5, 0.5039894, 0.5079783, 0.5119665, 0.5159534, 0.5199388, 0.5239222, 0.5279032, 0.5318814, 0.5358564,
0.5398278, 0.5437953, 0.5477584, 0.5517168, 0.55567, 0.5596177, 0.5635595, 0.5674949, 0.5714237, 0.5753454,
0.5792597, 0.5831662, 0.5870644, 0.5909541, 0.5948349, 0.5987063, 0.6025681, 0.6064199, 0.6102612, 0.6140919,
0.6179114, 0.6217195, 0.6255158, 0.6293, 0.6330717, 0.6368307, 0.6405764, 0.6443088, 0.6480273, 0.6517317,
0.6554217, 0.659097, 0.6627573, 0.6664022, 0.6700314, 0.6736448, 0.6772419, 0.6808225, 0.6843863, 0.6879331,
0.6914625, 0.6949743, 0.6984682, 0.701944, 0.7054015, 0.7088403, 0.7122603, 0.7156612, 0.7190427, 0.7224047,
0.7257469, 0.7290691, 0.7323711, 0.7356527, 0.7389137, 0.7421539, 0.7453731, 0.7485711, 0.7517478, 0.7549029,
0.7580363, 0.7611479, 0.7642375, 0.7673049, 0.77035, 0.7733726, 0.7763727, 0.7793501, 0.7823046, 0.7852361,
0.7881446, 0.7910299, 0.7938919, 0.7967306, 0.7995458, 0.8023375, 0.8051055, 0.8078498, 0.8105703, 0.8132671,
0.8159399, 0.8185887, 0.8212136, 0.8238145, 0.8263912, 0.8289439, 0.8314724, 0.8339768, 0.8364569, 0.8389129,
0.8413447, 0.8437524, 0.8461358, 0.848495, 0.85083, 0.8531409, 0.8554277, 0.8576903, 0.8599289, 0.8621434,
0.8643339, 0.8665005, 0.8686431, 0.8707619, 0.8728568, 0.8749281, 0.8769756, 0.8789995, 0.8809999, 0.8829768,
0.8849303, 0.8868606, 0.8887676, 0.8906514, 0.8925123, 0.8943502, 0.8961653, 0.8979577, 0.8997274, 0.9014747,
0.9031995, 0.9049021, 0.9065825, 0.9082409, 0.9098773, 0.911492, 0.913085, 0.9146565, 0.9162067, 0.9177356,
0.9192433, 0.9207302, 0.9221962, 0.9236415, 0.9250663, 0.9264707, 0.927855, 0.9292191, 0.9305634, 0.9318879,
0.9331928, 0.9344783, 0.9357445, 0.9369916, 0.9382198, 0.9394292, 0.9406201, 0.9417924, 0.9429466, 0.9440826,
0.9452007, 0.9463011, 0.9473839, 0.9484493, 0.9494974, 0.9505285, 0.9515428, 0.9525403, 0.9535213, 0.954486,
0.9554345, 0.9563671, 0.9572838, 0.9581849, 0.9590705, 0.9599408, 0.9607961, 0.9616364, 0.962462, 0.963273,
0.9640697, 0.9648521, 0.9656205, 0.966375, 0.9671159, 0.9678432, 0.9685572, 0.9692581, 0.969946, 0.970621 };

inline void getBlockAndThread(int &block, int &thread, int num) {
	thread = 32;
	if (num > 2000000) thread = 1024;
	block = num / thread;
	if (num % thread) block++;
}

//bool *isU;

void AWP::run(int sid) {
	//puts("haha");
	const int numOfEdges = model.GetNumOfEdges();
	const int numOfVerts = model.GetNumOfVerts();
	const int numOfFaces = model.GetNumOfFaces();
	//	printf("%lf\n", (1.0*numOfEdges * sizeof(HalfEdge) + numOfFaces * sizeof(Face) + numOfVerts * sizeof(Point3D)) / 1024 / 1024);
	//system("pause");
	//	return;
	/*if (!init_cuda(false)) {
		puts("----------------------init cuda error----------------------");
		return;
	}*/
	size_t freee_2;
	size_t total_2;
	cudaMemGetInfo(&freee_2, &total_2);
	//printf("the size of free and total bit is %lf, %u\n", 1.0*freee_2 / 1024 / 1024, total_2);

	puts("-------------------start cuda and init model--------------------");
	puts("Loading......");
	ang = new WindowsInAng[model.GetNumOfEdges()];

	for (int i = 0; i < model.GetNumOfEdges(); i++) {
		ang[i] = WindowsInAng(INF, Point2D(0, 0));
	}
	isU = new bool[numOfVerts];
	memset(isU, false, sizeof(bool)*numOfVerts);
	//freopen("source.txt", "r", stdin);
	//srand((unsigned int)(time(0)));
	//for (int i = 0; i < 5; i++) {
	//sid = (1LL*rand()+rand())% numOfVerts;
	isU[sid] = true;
	//printf("%d\n", sid);
	for (int i = 0; i < numOfVerts; i++) {
		model.dist[i] = INF;
	}
	model.dist[sid] = 0.0;
	//}


	float *dist = 0;
	thrust::device_vector<Point2D> v_angOnEdge(model.angOnEdge.begin(), model.angOnEdge.end());
	thrust::device_vector<HalfEdge> v_halfEdge(model.halfEdges.begin(), model.halfEdges.end());
	thrust::device_vector<bool> v_isN(model.isNieght.begin(), model.isNieght.end());
	thrust::device_vector<int> v_outgoing(model.idOfHalfEdge.begin(), model.idOfHalfEdge.end());
	//v_angOnEdge.clear();
	Point2D * angOnEdge = thrust::raw_pointer_cast(v_angOnEdge.data());
	HalfEdge * halfEdge = thrust::raw_pointer_cast(v_halfEdge.data());
	bool * isN = thrust::raw_pointer_cast(v_isN.data());
	int * outgoing = thrust::raw_pointer_cast(v_outgoing.data());
	WindowsInAng *angVal = 0;

	cudaMalloc((void**)&dist, numOfVerts * sizeof(float));
	cudaMemcpy(dist, model.dist, sizeof(float)*numOfVerts, cudaMemcpyHostToDevice);
	cudaMalloc((void**)&angVal, sizeof(WindowsInAng)*numOfEdges);
	cudaMemcpy(angVal, ang, sizeof(WindowsInAng)*numOfEdges, cudaMemcpyHostToDevice);
	//printf("%d\n", sizeof(Point2D)*numOfEdges + sizeof(float)*numOfVerts + numOfEdges*sizeof(HalfEdge) + numOfVerts*sizeof(bool) +
	//numOfVerts*sizeof(int) + sizeof(WindowsInEdge)*210000 + sizeof(New_Windows_Save)*210000*2 + sizeof(float)*numOfEdges + sizeof(int)*2*numOfEdges);
	//

	/*
	uchar *textlen = 0;
	cudaMalloc((void**)&textlen, sizeof(uchar)*numOfEdges);
	//*/

	int point_block[2], point_thread[2], edge_block[3], edge_thread[3];

	getBlockAndThread(point_block[0], point_thread[0], numOfVerts);
	getBlockAndThread(edge_block[0], edge_thread[0], numOfEdges);

	Windows *windowsInEdge = 0;
	Windows *windowsInEdge1 = 0;
	Windows *creatingInEdgeLeft = 0;
	Windows *creatingInEdgeRight = 0;
	
	int *lenOfWindowsInEdge = 0;
	int *lenOfWindowsInEdge1 = 0;
	//thrust::device_vector<int> lenOfWindowsInEdge(numOfEdges, 0);
	//thrust::device_vector<int> lenOfWindowsInEdge1(numOfEdges, 0);
	//int * pointer_lenOfWindowsInEdge = thrust::raw_pointer_cast(lenOfWindowsInEdge.data());
	//int * pointer_lenOfWindowsInEdge1 = thrust::raw_pointer_cast(lenOfWindowsInEdge1.data());

	int *lenOfCreatingInEdgeLeft = 0;
	int *lenOfCreatingInEdgeRight = 0;

	
	thrust::device_vector<int> address(numOfEdges, 0);
	thrust::device_vector<int> address1(numOfEdges, 0);
	/*
	int * address;
	cudaMalloc((void**)&address, sizeof(int)*numOfEdges);
	int * address1;
	cudaMalloc((void**)&address1, sizeof(int)*numOfEdges);
	*/


	int *lcnt = 0;
	cudaMalloc((void**)&lcnt, sizeof(int)*numOfEdges);
	double mem = (1.0*sizeof(int)*(16 * numOfEdges + numOfVerts) +
		sizeof(float) * 2 * (numOfEdges + numOfVerts) + (sizeof(WindowsInAng) - sizeof(float)) * numOfEdges + sizeof(bool) * 2 * numOfVerts + sizeof(Point2D)*numOfEdges + sizeof(HalfEdge)*numOfEdges) / 1024 / 1024;
	//system("pause");
	int sizeOfBuf = (int)((1.0*freee_2 / 1024 / 1024 - mem - 600) * 1024 * 1024 / (4 * sizeof(Windows)));
	//system("pause");
	//printf("%lf %d\n", mem, sizeOfBuf);
	cudaMalloc((void**)&windowsInEdge, sizeof(Windows)*sizeOfBuf);
	cudaMalloc((void**)&windowsInEdge1, sizeof(Windows)*sizeOfBuf);
	cudaMalloc((void**)&creatingInEdgeLeft, sizeof(Windows)*sizeOfBuf);
	cudaMalloc((void**)&creatingInEdgeRight, sizeof(Windows)*sizeOfBuf);
	cudaMalloc((void**)&lenOfWindowsInEdge, sizeof(int)*numOfEdges);
	cudaMalloc((void**)&lenOfWindowsInEdge1, sizeof(int)*numOfEdges);
	cudaMemset(lenOfWindowsInEdge, 0, sizeof(int)*numOfEdges);
	cudaMalloc((void**)&lenOfCreatingInEdgeLeft, sizeof(int)*numOfEdges);
	cudaMalloc((void**)&lenOfCreatingInEdgeRight, sizeof(int)*numOfEdges);
	thrust::device_ptr<int> ptr_len(lenOfWindowsInEdge);
	thrust::device_ptr<int> ptr_len1(lenOfWindowsInEdge1);

	float *dist_up_s = 0;
	float *dist_up_e = 0;
	cudaMalloc((void**)&dist_up_s, sizeof(float)*numOfEdges);
	cudaMalloc((void**)&dist_up_e, sizeof(float)*numOfEdges);
	/*edge_thread[0] = EB, edge_block[0] = numOfEdges / EB + (numOfEdges%EB == 0 ? 0 : 1);
	point_thread[0] = VB, point_block[0] = numOfVerts / VB + (numOfVerts%VB == 0 ? 0 : 1);
	printf("%d %d %d %d\n", edge_block[0], edge_thread[0], point_block[0], point_thread[0]);
	/*******************************************初始化**************************************************/
	//int *cuda_sum = 0, *cuda_sum1 = 0;
	//int *hz_sum = 0;
	/*
	int * cuda_sum;
	cudaMalloc((void**)&cuda_sum, sizeof(int)*numOfEdges);
	int * cuda_sum1;
	cudaMalloc((void**)&cuda_sum1, sizeof(int)*numOfEdges);
	*/
	int * sum_win;
	cudaMalloc((void**)&sum_win, sizeof(int)*numOfEdges);
	float * dist_win;
	cudaMalloc((void**)&dist_win, sizeof(float)*numOfEdges);
	thrust::device_ptr<int> ptr_sum_win(sum_win);
	thrust::device_ptr<float> ptr_dist_win(dist_win);
	cudaMemset(dist_win, 0, sizeof(float)*numOfEdges);
	cudaMemset(sum_win, 0, sizeof(float)*numOfEdges);
	/*
	thrust::device_ptr<int> ptr_cuda_sum(cuda_sum);
	thrust::device_ptr<int> ptr_cuda_sum1(cuda_sum1);
	thrust::device_ptr<int> ptr_address(address);
	thrust::device_ptr<int> ptr_address1(address1);
	thrust::device_ptr<int> ptr_sum_win(sum_win);
	thrust::device_ptr<float> ptr_dist_win(dist_win);
	//*/
	//*
	thrust::device_vector<int> cuda_sum(numOfEdges, 0);
	thrust::device_vector<int> cuda_sum1(numOfEdges, 0);

	//thrust::device_vector<float> dist_win(numOfEdges, 0);
	//thrust::device_vector<int> sum_win(numOfEdges, 0);

	//thrust::device_vector<float> test_sum(numOfEdges, 0);
	//float * pointer_test_sum = thrust::raw_pointer_cast(test_sum.data());
	int * pointer_address1 = thrust::raw_pointer_cast(address1.data());
	int * pointer_address = thrust::raw_pointer_cast(address.data());
	int * pointer_cuda_sum1 = thrust::raw_pointer_cast(cuda_sum1.data());
	int * pointer_cuda_sum = thrust::raw_pointer_cast(cuda_sum.data());
	//int * pointer_sum_win = thrust::raw_pointer_cast(sum_win.data());
	//float * pointer_dist_win = thrust::raw_pointer_cast(dist_win.data());
	//*/

	bool *isUpdate = 0;
	cudaMalloc((void**)&isUpdate, sizeof(bool)*numOfVerts);
	cudaMemcpy(isUpdate, isU, sizeof(bool)*numOfVerts, cudaMemcpyHostToDevice);

	int *Eid = 0;
	cudaMalloc((void**)&Eid, sizeof(int)*numOfEdges);
	int *Eid2 = 0;
	cudaMalloc((void**)&Eid2, sizeof(int)*numOfEdges);

	//thrust::device_vector<int> copy_address(numOfEdges, 0);

	//int * pointer_copyaddress = thrust::raw_pointer_cast(copy_address.data());
#if SSD
	calCntBySource << <1, 1 >> >(sid, halfEdge, outgoing, pointer_cuda_sum);
	cudaDeviceSynchronize();
	thrust::inclusive_scan(cuda_sum.begin(), cuda_sum.end(), cuda_sum.begin());
	getEidBySource << <1, 1 >> >(pointer_cuda_sum, pointer_address, Eid, numOfEdges);
	cudaDeviceSynchronize();
	createBySource_YH << <1, 1 >> >(sid, halfEdge, outgoing, windowsInEdge, dist, pointer_address, lenOfWindowsInEdge, isUpdate, dist_up_s, dist_up_e, angVal);// pointer_dist_win);
	cudaDeviceSynchronize();
#endif
	float AVG = INF;
	calCntByPoint_YHMSD << <point_block[0], point_thread[0] >> >(halfEdge, outgoing, numOfVerts, isUpdate, pointer_address);
	cudaDeviceSynchronize();

	updateAddress << <edge_block[0], edge_thread[0] >> >(lcnt, pointer_cuda_sum, pointer_address, numOfEdges);
	cudaDeviceSynchronize();

	thrust::inclusive_scan(cuda_sum.begin(), cuda_sum.end(), cuda_sum.begin());
	thrust::exclusive_scan(address.begin(), address.begin() + numOfEdges, address.begin());
	//thrust::exclusive_scan(address., pointer_address1+numOfEdges, pointer_address1);

	int past_edges = cuda_sum[numOfEdges - 1];

	pointEven_New_YH << <point_block[0], point_thread[0] >> >(halfEdge, outgoing, windowsInEdge1, windowsInEdge, pointer_address1,
		lenOfWindowsInEdge1, pointer_address, lenOfWindowsInEdge, dist, numOfVerts, isUpdate);
	cudaDeviceSynchronize();
	getBlockAndThread(edge_block[2], edge_thread[2], past_edges);

	getEid << <edge_block[0], edge_thread[0] >> >(pointer_cuda_sum, Eid2, numOfEdges);
	cudaDeviceSynchronize();
	readWindowsOnEdge_VTP_New_YH << <edge_block[2], edge_thread[2] >> > (halfEdge, creatingInEdgeLeft, creatingInEdgeRight, windowsInEdge, pointer_address1, pointer_address,
		lenOfWindowsInEdge, lenOfCreatingInEdgeLeft, lenOfCreatingInEdgeRight,
		pointer_cuda_sum1, past_edges, Eid2, dist, isUpdate, dist_win, AVG, dist_up_s, dist_up_e);
	cudaDeviceSynchronize();

	bool now = false;
	int cnt = 0;
	int num_active_edge_curr = past_edges;// cuda_sum[numOfEdges - 1];

	double cpoint, cedge, csum, ch, cpu_time;
	double copy = 0.0;
	cpoint = cedge = csum = ch = cpu_time = 0.0;

	long long total_windows = num_active_edge_curr;
	long long sumOfWindows = num_active_edge_curr;
	long long sumOfActiveEdge = num_active_edge_curr;
	int peak_windows = num_active_edge_curr;
	int peak_windows_guji = num_active_edge_curr;
	int peak_active_edge = num_active_edge_curr;
	int getS = 0;
	float avg;
	double ewaiTime = 0;

	int *youhua = 0;
	cudaMalloc((void**)&youhua, sizeof(int) * 257);

	int bqLimit = 500000000;
	if (numOfFaces > 3000000) bqLimit = 1000000;
	bool isbq = false;
	double bq_time = 0.0;
	double init = 0;
	double point_event_time = 0.0;
	double creating_windows_time = 0.0;
	double recive_windows_time = 0.0;
	int num_active_windows = num_active_edge_curr;
	int num_active_windows_processed;
	int num_active_windows_unprocessed;
	long long total_windows_chuli = 0;
	long long baoqian_cnt = 0;
	long long ps_cnt = 0;
	double cal_biaozhuncha_time = 0.0;
	double tongji = 0;
	double bqtiongji = 0;

	puts("-------------------init cuda and model done---------------------\n");
	puts("-------------------------algorithm begin------------------------");
	CTimer time = CTimer();
	do {
		printf("[%d]\n", cnt);
		getBlockAndThread(edge_block[1], edge_thread[1], num_active_edge_curr);

		CTimer initT = CTimer();
		cudaMemset(lcnt, 0, sizeof(int)*numOfEdges);
		cudaMemset(lenOfWindowsInEdge1, 0, sizeof(int)*numOfEdges);
		//cudaMemset(copy_address, 0, sizeof(int)*numOfEdges);
		if (now == false) {
			thrust::fill(cuda_sum1.begin(), cuda_sum1.end(), (int)0);
			thrust::fill(address1.begin(), address1.end(), (int)0);
		}
		else {
			thrust::fill(cuda_sum.begin(), cuda_sum.end(), (int)0);
			thrust::fill(address.begin(), address.end(), (int)0);
		}
		copy += initT.End();

#if MSD

#endif
		CTimer CP = CTimer();
		calCntByPoint_YH << <point_block[0], point_thread[0] >> >(halfEdge, outgoing, angVal, isN, dist, numOfVerts, pointer_cuda_sum,
			isUpdate, pointer_address1, dist_up_s, dist_up_e);
		cudaDeviceSynchronize();
		cpoint += CP.End();
		//puts("haha");
		CTimer CE = CTimer();
		if (isbq) {
			calCntByEdge_Init << <edge_block[1], edge_thread[1] >> > (Eid, lcnt, pointer_address1, pointer_cuda_sum1, num_active_edge_curr, halfEdge,
				angOnEdge, windowsInEdge, pointer_address, lenOfWindowsInEdge, AVG);
			cudaDeviceSynchronize();
			CTimer tongjit;
			if (now == false) {
				num_active_windows_unprocessed = thrust::reduce(cuda_sum1.begin(), cuda_sum1.end());
				num_active_windows_processed = num_active_windows - num_active_windows_unprocessed;
			}
			else {
				num_active_windows_unprocessed = thrust::reduce(cuda_sum.begin(), cuda_sum.end());
				num_active_windows_processed = num_active_windows - num_active_windows_unprocessed;
			}
			tongji += tongjit.End();
#if DEBUG
			//printf("num_active_windows_processed %d\n", num_active_windows_processed);
			//printf("num_active_windows_processed %d, num_active_windows_unprocessed %d num_active_windows %d %lf\n", num_active_windows_processed, num_active_windows_unprocessed, num_active_windows, num_active_windows_processed*1.0/num_active_windows);
			printf("%d\t%d\n", num_active_windows_processed, num_active_windows);
#endif
		}
		else {
			calCntByEdge_Init_Notbq << <edge_block[1], edge_thread[1] >> > (Eid, lcnt, pointer_address1, num_active_edge_curr, halfEdge, angOnEdge,
				windowsInEdge, pointer_address, lenOfWindowsInEdge);
			cudaDeviceSynchronize();
			num_active_windows_processed = num_active_windows;
		}

		updateAddress << <edge_block[0], edge_thread[0] >> >(lcnt, pointer_cuda_sum1, pointer_address1, numOfEdges);
		cudaDeviceSynchronize();
		cedge += CE.End();

		CTimer CS = CTimer();

		if (now == false) {
			thrust::inclusive_scan(cuda_sum1.begin(), cuda_sum1.end(), cuda_sum1.begin());
			thrust::exclusive_scan(address1.begin(), address1.begin() + numOfEdges, address1.begin());
		}
		else {
			thrust::inclusive_scan(cuda_sum.begin(), cuda_sum.end(), cuda_sum.begin());
			thrust::exclusive_scan(address.begin(), address.begin() + numOfEdges, address.begin());
		}
		//thrust::exclusive_scan(address., pointer_address1+numOfEdges, pointer_address1);

		if (now == false) {
			past_edges = cuda_sum1[numOfEdges - 1];
		}
		else {
			past_edges = cuda_sum[numOfEdges - 1];
		}

		getBlockAndThread(edge_block[2], edge_thread[2], past_edges);

		ps_cnt += numOfEdges;
		getEid << <edge_block[0], edge_thread[0] >> >(pointer_cuda_sum1, Eid2, numOfEdges);
		cudaDeviceSynchronize();
		/*
		reduceAddress<<<edge_block[3], edge_thread[3]>>>(Eid2, pointer_address1, pointer_copyaddress, past_edges);
		cudaDeviceSynchronize();//printf("%d %d\n", cnt, past_edges);
		thrust::exclusive_scan(copy_address.begin(), copy_address.begin() + past_edges, copy_address.begin());
		copyReturn << <edge_block[3], edge_thread[3] >> > (Eid2, pointer_address1, pointer_copyaddress, past_edges);
		cudaDeviceSynchronize();
		//*/
		csum += CS.End();
		//puts("hehe");
		CTimer h;

		CTimer point_event_timeC;
		//pointEven_New_YHRH << <point_block[0], point_thread[0] >> >(halfEdge, outgoing, windowsInEdge, windowsInEdge1, pointer_address,
		//lenOfWindowsInEdge, pointer_address1, lenOfWindowsInEdge1, dist, numOfVerts, isUpdate, pointer_cuda_sum, isN, dist_up_s, dist_up_e, angVal);

		pointEven_New_YH << <point_block[0], point_thread[0] >> >(halfEdge, outgoing, windowsInEdge, windowsInEdge1, pointer_address,
			lenOfWindowsInEdge, pointer_address1, lenOfWindowsInEdge1, dist, numOfVerts, isUpdate);
		cudaDeviceSynchronize();
		point_event_time += point_event_timeC.End();
		//puts("xixi");
		CTimer creatingW_C;
		if (isbq) {
			createWindowsByWindows_VTP_New_pq_YH << <edge_block[1], edge_thread[1] >> > (halfEdge, dist, windowsInEdge, creatingInEdgeLeft, creatingInEdgeRight,
				pointer_address, lenOfWindowsInEdge, lenOfCreatingInEdgeLeft, lenOfCreatingInEdgeRight, angVal,
				angOnEdge, num_active_edge_curr, Eid, isUpdate, windowsInEdge1, pointer_address1, lenOfWindowsInEdge1, AVG, isN);
			cudaDeviceSynchronize();
		}
		else {
			createWindowsByWindows_VTP_New_pq_YH_Notbq << <edge_block[1], edge_thread[1] >> > (halfEdge, dist, windowsInEdge, creatingInEdgeLeft, creatingInEdgeRight,
				pointer_address, lenOfWindowsInEdge, lenOfCreatingInEdgeLeft, lenOfCreatingInEdgeRight, angVal,
				angOnEdge, num_active_edge_curr, Eid, isUpdate, windowsInEdge1, pointer_address1, lenOfWindowsInEdge1, isN);
			cudaDeviceSynchronize();
		}
		creating_windows_time += creatingW_C.End();
		//	puts("gugu");
		CTimer shouji;
		readWindowsOnEdge_VTP_New_YH << <edge_block[2], edge_thread[2] >> > (halfEdge, creatingInEdgeLeft, creatingInEdgeRight, windowsInEdge1, pointer_address, pointer_address1,
			lenOfWindowsInEdge1, lenOfCreatingInEdgeLeft, lenOfCreatingInEdgeRight,
			pointer_cuda_sum, past_edges, Eid2, dist, isUpdate, dist_win, AVG, dist_up_s, dist_up_e);
		cudaDeviceSynchronize();
		//	puts("gugu2");
		CTimer bqTongji;
		if (now == false) {
			getS = thrust::reduce(ptr_len1, ptr_len1 + numOfEdges, 0);
		}
		else getS = thrust::reduce(ptr_len, ptr_len + numOfEdges, 0);
		//getS = thrust::reduce(ptr_sum_win, ptr_sum_win + past_edges);
		num_active_windows = getS;
		bqtiongji += bqTongji.End();

		if (num_active_windows > bqLimit) {
			isbq = true;

#if EPCH_AVG
			CTimer ps;
			avg = thrust::reduce(ptr_dist_win, ptr_dist_win + past_edges);
			bq_time += ps.End();
			//int tsss = thrust::reduce(ptr_sum_win, ptr_sum_win + past_edges);
			//float tbb = thrust::reduce(ptr_dist_win, ptr_dist_win + past_edges);
			//thrust::inclusive_scan(ptr_sum_win, ptr_sum_win + past_edges, ptr_sum_win);
			//cudaMemcpy(&getS, sum_win + past_edges - 1, sizeof(int), cudaMemcpyDeviceToHost);
			//thrust::inclusive_scan(ptr_dist_win, ptr_dist_win + past_edges, ptr_dist_win);
			//cudaMemcpy(&avg, dist_win + past_edges - 1, sizeof(float), cudaMemcpyDeviceToHost);

			//printf("%d %f %d %f\n", tsss, tbb, getS, avg);
			//system("pause");
			//printf("%d %f %d\n", cnt, avg, getS);
			AVG = avg / getS;

			baoqian_cnt = max(baoqian_cnt, 1ll * past_edges);
			float bl = 1.0*bqLimit / num_active_windows;
			float sign = 1.0;
			if (bl < 0.5) sign = -1.0, bl = 1.0 - bl;

			int i;
			for (i = 0; i < 179; i++) {
				if (bl > ZT_biao[i] && bl < ZT_biao[i + 1] + EPS) break;
			}

			//copy += ps.End();
#endif

#if TEST_MAX_MIN
			getS = thrust::reduce(ptr_sum_win, ptr_sum_win + past_edges, INF, thrust::minimum<float>());
			avg = thrust::reduce(ptr_dist_win, ptr_dist_win + past_edges, INF, thrust::minimum<float>());
			getS = -getS;
			//float tf = 500000.0 / numOfwindows;
			AVG = (avg + getS)*0.5;
#endif
			//	printf("total active windows: %d\n", (int)getS);
			//printf(" avg %lf: ", AVG);
#if TEST_ZT
			//AVG = avg / getS;
			//if (i < 10 && i != 0 && i != 5) {
			//if (i > 130) {
			float biaozhuncha;
			if (i < 179) {
				if (i > 150 && sign < 0) i = 150;
				thrust::fill(ptr_dist_win, ptr_dist_win + past_edges, (float)0);
				CTimer biaozhuncha_Time;
				getbiaozhuncha << <edge_block[2], edge_thread[2] >> > (windowsInEdge1, pointer_address1, lenOfWindowsInEdge1, dist_win, AVG, past_edges, Eid2, cnt);
				cudaDeviceSynchronize();

				//fprintf(stderr, "dfadfa\n");
				biaozhuncha = thrust::reduce(ptr_dist_win, ptr_dist_win + past_edges); cal_biaozhuncha_time += biaozhuncha_Time.End();
				biaozhuncha = sqrt(biaozhuncha / getS);

				//printf("%f\n", biaozhuncha);
				//if (numOfwindows > 3000000) AVG = AVG - biaozhuncha * 2;
				//else 
				AVG = AVG + sign*i / 100 * biaozhuncha;
			}
			else {
				isbq = false;
				AVG = INF;
			}
#if DEBUG
			//	printf("getS %lf\n", getS);
			//printf("%f %f %f %f %d\t", sign*i*1.0 / 100, avg, biaozhuncha, AVG, cnt);
#endif
#endif

#if DEBUG
			//	printf("%d %f\n", i, ZT_biao[i]);
#endif
			/*
			// +EPS;
			//*/
		}
		else {
			isbq = false;
			AVG = INF;
			/*
			readWindowsOnEdge_VTP_New_YH_Notbq << <edge_block[3], edge_thread[3] >> > (halfEdge, creatingInEdgeLeft, creatingInEdgeRight, windowsInEdge1, pointer_address, pointer_address1,
			lenOfWindowsInEdge1, lenOfCreatingInEdgeLeft, lenOfCreatingInEdgeRight,
			pointer_cuda_sum, past_edges, Eid2, dist, isUpdate);
			cudaDeviceSynchronize();
			*/
		}
#if huatu
		if (cnt % 3 == 0) {
			Tongji << <1, 1 >> > (windowsInEdge1, pointer_address1, lenOfWindowsInEdge1, Eid2, past_edges, AVG);
			cudaDeviceSynchronize();
		}
#endif
	DD:
		recive_windows_time += shouji.End();
		ch += h.End();
		num_active_edge_curr = past_edges;

		CTimer ewai = CTimer();

		/*统计信息*/
		peak_active_edge = max(peak_active_edge, num_active_edge_curr);
		sumOfActiveEdge += num_active_edge_curr;

		if (now == false) {
			peak_windows_guji = max(peak_windows_guji, address1[numOfEdges - 1]);
			//num_active_windows = thrust::reduce(ptr_len1, ptr_len1 + numOfEdges, 0);
			sumOfWindows += num_active_windows_processed;//address[numOfEdges - 1];
			peak_windows = max(peak_windows, num_active_windows);
			total_windows += num_active_windows;
		}
		else {
			peak_windows_guji = max(peak_windows_guji, address[numOfEdges - 1]);
			//num_active_windows = thrust::reduce(ptr_len, ptr_len + numOfEdges, 0);
			sumOfWindows += num_active_windows_processed;// address1[numOfEdges - 1];
			peak_windows = max(peak_windows, num_active_windows);
			total_windows += num_active_windows;
		}
#if DEBUG
		//printf("num_active_windows %d\n", num_active_windows);
#endif
		swap(lenOfWindowsInEdge, lenOfWindowsInEdge1);
		swap(windowsInEdge, windowsInEdge1);
		swap(pointer_address, pointer_address1);
		swap(pointer_cuda_sum, pointer_cuda_sum1);
		swap(Eid, Eid2);
		ewaiTime += ewai.End();
		cnt++;
		now = !now;
	} while (num_active_edge_curr);

	/*
	freopen("out1.txt", "w", stdout);
	printf("total_windows %lld\n", total_windows);
	printf("%lf\t%lf\t%lf\t%lf\t%lf\t%lld\t%d\t%lf\t%lf\t%d\t%lld\n", time.End() - ewaiTime - copy, cpoint, cedge, csum, ch, sumOfWindows, peak_windows,
	(1.0*peak_windows*(sizeof(Windows) * 4) + sizeof(int)*(15 * numOfEdges + numOfVerts) +
	sizeof(float) * 2 * (numOfEdges + numOfVerts) + sizeof(bool) * 2 * numOfVerts + sizeof(Point2D)*numOfEdges + sizeof(HalfEdge)*numOfEdges) / 1024 / 1024,
	(1.0*peak_windows_guji*(sizeof(Windows) * 4) + sizeof(int)*(15 * numOfEdges + numOfVerts) +
	sizeof(float) * 2 * (numOfEdges + numOfVerts) + sizeof(bool) * 2 * numOfVerts + sizeof(Point2D)*numOfEdges + sizeof(HalfEdge)*numOfEdges) / 1024 / 1024,
	peak_active_edge, sumOfActiveEdge / cnt);
	//*/
	//*
	//printf("%lld %lld\n", baoqian_cnt, ps_cnt);
	printf("total_windows %lld\n", total_windows);
	puts("---------------------------Time info----------------------------");
	printf("Running time(s):\t%lf\n", time.End() - ewaiTime - copy - tongji - bqtiongji);
	printf("step1\t%f\n", copy);
	printf("step2\t%f\n", cpoint);
	printf("step3\t%f\n", cedge-tongji);
	printf("step4\t%f\n", point_event_time);

	printf("step5\t%f\n", creating_windows_time);
	printf("step6\t%f\n", recive_windows_time - bqtiongji);
	printf("step6\t%f\n", csum);

	puts("--------------------------Memory info---------------------------");
	printf("The number of Windows:\t%lld\n", sumOfWindows);
	printf("Peck active edge:\t%d\nmean active edge:\t%lld\n", peak_active_edge, sumOfActiveEdge / cnt);
	printf("The number of iteration:\t%d\n", cnt);
	printf("Peak of Windows:\t%d\nPeak memory(M):\t%lf\n", peak_windows, 1.0*peak_windows*sizeof(Windows) * 4 / 1024 / 1024);
	puts("-------------------------algorithm end--------------------------\n");
	//*/
	cudaMemcpy(model.dist, dist, sizeof(float)*numOfVerts, cudaMemcpyDeviceToHost);
	//*
	cudaFree(angVal);
	//cudaFree(angOnEdge);
	cudaFree(dist);
	cudaFree(windowsInEdge);
	cudaFree(windowsInEdge1);
	cudaFree(creatingInEdgeLeft);
	cudaFree(creatingInEdgeRight);
	cudaFree(lenOfCreatingInEdgeLeft);
	cudaFree(lenOfCreatingInEdgeRight);
	cudaFree(lenOfWindowsInEdge);
	cudaFree(lenOfWindowsInEdge1);
	cudaFree(lcnt);
	cudaFree(dist_up_s);
	cudaFree(dist_up_e);
	cudaFree(sum_win);
	cudaFree(dist_win);
	cudaFree(isUpdate);
	cudaFree(Eid);
	cudaFree(Eid2);
	cudaFree(youhua);
	v_angOnEdge.clear();
	v_halfEdge.clear();
	v_isN.clear();
	v_outgoing.clear();
	address.clear();
	address1.clear();
	cuda_sum.clear();
	cuda_sum1.clear(); 
	//thrust::
	//thrust::device_free(thrust::device_ptr<void> (ptr_sum_win));
	//thrust::device_free(ptr_dist_win);
	//thrust::device_free(ptr_len);
	//thrust::device_free(ptr_len1);
	//delete angOnEdge;
	delete isU;
	delete ang;
	//*/
	return;
}
