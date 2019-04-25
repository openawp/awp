// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

//#include "targetver.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cstdio>
#include <tchar.h>

#include <iomanip>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <limits>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <Windows.h>
#include <thrust/scan.h>
#include <thrust/sequence.h> 
#include <thrust/host_vector.h>  
#include <thrust/fill.h>
#include <thrust/device_vector.h>   
const float PI = acos(-1.0);
#define INF 9999.0
/*torus
#define NUMOFVERS 4801
#define NUMOFFACE 9601
#define NUMOFEDGE 28801
*/
/*bunny144*/
#define NUMOFVERS 72021
#define NUMOFFACE 144037
#define NUMOFEDGE 432109
/*bunny
#define NUMOFVERS 34836
#define NUMOFFACE 69667
#define NUMOFEDGE 208999
*/
/*botijo
#define NUMOFVERS 11790
#define NUMOFFACE 23595
#define NUMOFEDGE 70783*/

/*dragon
#define NUMOFVERS 50003
#define NUMOFFACE 100001
#define NUMOFEDGE 300001
*/

/*Beet
#define NUMOFVERS 10407
#define NUMOFFACE 20809
#define NUMOFEDGE 62425
*/
/*golf
#define NUMOFFACE 100001
#define NUMOFEDGE 300001
#define NUMOFVERS 50003
*/
/*girl
#define NUMOFFACE 1005383
#define NUMOFEDGE 3016147
#define NUMOFVERS 502694
*/
using namespace std;
#define EPS (1e-6)

// TODO:  在此处引用程序需要的其他头文件
