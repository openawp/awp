#include "stdafx.h"
#include "Model_vector.h"

/*创建半边结构*/
void Model_vector::creatHalfEdge() {
	sizeOfHalf = 0;
	int cntFaces = faces.size();
	map<pair<int, int>, int> halfEdgeByPoint;
	halfEdgeByPoint.clear();
	for (int i = 0; i < vertexs.size(); i++) {
		idOfHalfEdge.push_back(-1);
	}
	angOnEdge.clear();
	halfEdges.clear();
	for (int i = 0; i < cntFaces; i++) {
		for (int j = 0; j < 3; j++) {
			int startPointId = faces[i][j];
			int endPointId = faces[i][(j + 1) % 3];
			HalfEdge newHalfEdge = HalfEdge();
			newHalfEdge.idOfOppositeEdge = -1;
			newHalfEdge.idOfStartPoint = startPointId;
			newHalfEdge.length = getDistance(vertexs[startPointId], vertexs[endPointId]);
			newHalfEdge.idOfOppositeVer = faces[i][(j + 2) % 3];
			sizeOfHalf++;//halfEdges[sizeOfHalf++] = newHalfEdge;
			halfEdges.push_back(newHalfEdge);
												  //occupyOnAng.push_back(-1);
			angOnEdge.push_back(Point2D(0.0, 0.0));
			int nowId = sizeOfHalf - 1;
			idOfHalfEdge[startPointId] = nowId;
			if (halfEdgeByPoint.find(make_pair(endPointId, startPointId)) != halfEdgeByPoint.end()) {
				int otherHalfEdgeId = halfEdgeByPoint[make_pair(endPointId, startPointId)];
				halfEdges[nowId].idOfOppositeEdge = otherHalfEdgeId;
				halfEdges[otherHalfEdgeId].idOfOppositeEdge = nowId;
			}
			else {
				halfEdgeByPoint[make_pair(startPointId, endPointId)] = nowId;
			}
		}
	}
}

void Model_vector::createPoint2dByAng(int eid, float ang, float a, float b, float c) {
	float tang = (b*b + c*c - a*a) / (2 * b*c);
	float len = halfEdges[idOfNextHalfEdge(eid)].length;
	angOnEdge[eid] = Point2D(len*tang, -len*sqrt(max(0.0, 1 - tang*tang)));
}

void Model_vector::findAllInfoOnVers() {
	int ss = 0, i;
	set<int> S;
	for (i = 0; i < vertexs.size(); i++) {
		float ang = 0.0;
		int eid = idOfHalfEdge[i];
		if (eid == -1) continue;
		S.clear();
		do {
			float a = halfEdges[eid].length;
			int teid = idOfNextHalfEdge(eid);
			float b = halfEdges[teid].length;
			eid = idOfPreHalfEdge(eid);
			float c = halfEdges[eid].length;
			eid = halfEdges[eid].idOfOppositeEdge;
			float tmAng = acos((a*a + c*c - b*b) / (2 * a*c));
			ang += tmAng;
			createPoint2dByAng(teid, tmAng, a, b, c);
			if (eid != idOfHalfEdge[i] && S.find(eid) != S.end()) {
				throw "model error";
			}
			S.insert(eid);
		} while (eid != -1 && eid != idOfHalfEdge[i]);
		if (eid == -1) {
			isClose = false;
			break;
			eid = halfEdges[idOfHalfEdge[i]].idOfOppositeEdge;
			while (eid != -1) {
				float a = halfEdges[eid].length;
				int teid = idOfPreHalfEdge(eid);
				float b = halfEdges[teid].length;
				eid = idOfNextHalfEdge(eid);
				float c = halfEdges[eid].length;
				eid = halfEdges[eid].idOfOppositeEdge;
				float tmAng = acos((a*a + c*c - b*b) / (2 * a*c));
				ang += tmAng;
			}
		}
		if (ang > 2 * PI) isNieght.push_back(true);
		else isNieght.push_back(false);
	}
}
/*遍历一个顶点所关联的所有顶点, 参数vid为该点的id， eid为以vid为起点的一条半边id。返回邻接点的id*/
vector<int> Model_vector::findAllAdjVer(int vid, int eid) {
	vector<int> adjVers;
	adjVers.clear();
	int nextId, tid = eid;
	while (1) {
		int idOfOppositeEdge = halfEdges[tid].idOfOppositeEdge;
		adjVers.push_back(halfEdges[idOfOppositeEdge].idOfStartPoint);
		nextId = halfEdges[idOfPreHalfEdge(tid)].idOfOppositeEdge;
		if (nextId == eid) {
			break;
		}
		else {
			tid = nextId;
		}
	}
	while (1) {
		int idOfNextEdge = Model_vector::idOfNextHalfEdge(tid);
		adjVers.push_back(halfEdges[idOfNextEdge].idOfStartPoint);
		nextId = halfEdges[idOfPreHalfEdge(Model_vector::idOfNextHalfEdge(idOfNextEdge))].idOfOppositeEdge;
		if (nextId == -1) {
			int idOfOppositeEdge = halfEdges[eid].idOfOppositeEdge;
			while (idOfOppositeEdge != -1) {
				nextId = Model_vector::idOfNextHalfEdge(idOfOppositeEdge);
				adjVers.push_back(halfEdges[Model_vector::idOfPreHalfEdge(idOfOppositeEdge)].idOfStartPoint);
				idOfOppositeEdge = halfEdges[nextId].idOfOppositeEdge;
			}
			break;
		}
		else if (nextId == eid) {
			break;
		}
		else {
			tid = nextId;
		}
	}
	return adjVers;
}


/*与读写文件有关*/
void Model_vector::LoadModel() {
	ReadFile(inputFileName);
}

void Model_vector::ReadObjFile2(char * filename) {
	std::ifstream file(filename);
	assert(file.is_open());

	char type;
	std::string curLine;
	float coord[3];
	unsigned int vtxIdx[3];
	std::map<string, int> mapForDuplicate;
	int originalVertNum = 0;
	vector<int> realIndex;

	while (getline(file, curLine))
	{
		if (curLine.size() < 2) continue;
		if (curLine[0] == 'v' && curLine[1] != 't')
		{
			std::map<string, int>::iterator pos = mapForDuplicate.find(curLine);
			if (pos == mapForDuplicate.end())
			{
				int oldSize = mapForDuplicate.size();
				realIndex.push_back(oldSize);
				mapForDuplicate[curLine] = oldSize;
				sscanf(curLine.c_str(), "v %f %f %f", &coord[0], &coord[1], &coord[2]);
				vertexs.push_back(Point3D(coord[0], coord[1], coord[2]));
			}
			else
			{
				realIndex.push_back(pos->second);
			}
			++originalVertNum;
		}
		else if (curLine[0] == 'f')
		{
			unsigned tex;
			if (curLine.find('/') != std::string::npos)
				sscanf(curLine.c_str(), "f %d/%d %d/%d %d/%d", &vtxIdx[0], &tex, &vtxIdx[1], &tex, &vtxIdx[2], &tex);
			else
				sscanf(curLine.c_str(), "f %d %d %d", &vtxIdx[0], &vtxIdx[1], &vtxIdx[2]);

			vtxIdx[0] = realIndex[vtxIdx[0] - 1];
			vtxIdx[1] = realIndex[vtxIdx[1] - 1];
			vtxIdx[2] = realIndex[vtxIdx[2] - 1];
			if (vtxIdx[0] == vtxIdx[1] || vtxIdx[0] == vtxIdx[2] || vtxIdx[1] == vtxIdx[2]) continue;
			faces.push_back(Face(vtxIdx[0], vtxIdx[1], vtxIdx[2]));
		}
	}
	file.close();
}

void Model_vector::ReadObjFile(char * filename)
{
	ifstream in(filename, ios_base::in | ios_base::binary);
	if (in.fail())
		throw "Fail to read file!";
	ifstream::pos_type posOfFace;
	map<string, int> uniqueVerts;
	vector<int> vertCorrespondence;
	while (!in.eof())
	{
		string firstWord;
		ReadIntoWord(in, firstWord);
		if (firstWord == "v")
		{
			ostringstream outStr;
			Point3D temp;
			in >> temp.x >> temp.y >> temp.z;
			if (fabs(temp.x) < FLT_EPSILON)
				temp.x = 0;
			if (fabs(temp.y) < FLT_EPSILON)
				temp.y = 0;
			if (fabs(temp.z) < FLT_EPSILON)
				temp.z = 0;
			outStr << setiosflags(ios_base::fixed) << setprecision(5) << temp.x << " " << temp.y << " " << temp.z;
			map<string, int>::iterator pos = uniqueVerts.find(outStr.str());
			if (pos == uniqueVerts.end())
			{
				int oldSize = (int)uniqueVerts.size();
				uniqueVerts[outStr.str()] = oldSize;
				vertCorrespondence.push_back(oldSize);
				vertexs.push_back(temp);
			}
			else
			{
				vertCorrespondence.push_back(pos->second);
			}
		}
		else if (firstWord == "f")
		{
			in.putback('f');
			posOfFace = in.tellg();
			break;
		}
		ReadUntilNextLine(in);
	}
	vertexs.swap(vector<Point3D>(vertexs));
	string temp;
	ReadIntoWord(in, temp);
	ReadIntoWord(in, temp);
	in.seekg(posOfFace);
	if (temp.find('/') == -1)
	{
		while (!in.eof())
		{
			string firstWord;
			ReadIntoWord(in, firstWord);
			if (firstWord == "f")
			{
				faces.push_back(Face());
				Face& lastFace = faces[faces.size() - 1];
				in >> lastFace[0] >> lastFace[1] >> lastFace[2];
				for (int j = 0; j < 3; ++j)
				{
					lastFace[j] = vertCorrespondence[lastFace[j] - 1];
				}
			}
			else
				break;
			ReadUntilNextLine(in);
		}
	}
	else
	{
		while (!in.eof())
		{
			string firstWord;
			ReadIntoWord(in, firstWord);
			if (firstWord == "f")
			{
				faces.push_back(Face());
				Face& lastFace = faces[faces.size() - 1];
				in >> lastFace[0] >> firstWord >> lastFace[1] >> firstWord >> lastFace[2];
				for (int j = 0; j < 3; ++j)
				{
					lastFace[j] = vertCorrespondence[lastFace[j] - 1];
				}
			}
			else
				break;
			ReadUntilNextLine(in);
		}
	}
	in.close();
	faces.swap(vector<Face>(faces));
}

void Model_vector::ReadFile(char * filename)
{
	// system("pause");
	std::string tmpfilename(filename);
	int nDot = (int)tmpfilename.rfind(L'.');
	if (nDot == -1)
	{
		throw "File name doesn't contain a dot!";
	}

	std::string extension = tmpfilename.substr(nDot + 1);
	for (int i = 0; i < extension.size(); ++i)
	{
		extension[i] = tolower(extension[i]);
	}

	if (extension == "obj")
	{
		ReadObjFile2(filename);
	}
	else if (extension == "ply")
	{
		ReadPlyFile(filename);
	}
	else if (extension == "m") {
		ReadMFile(filename);
	}
	else
	{ 
		throw "This format can't be handled!";
	}

}

void Model_vector::ReadMFile(char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		throw "This format can't be handled!";
	}
	char buf[4096];
	int vid, c = 0;
	Point3D p;
	Face f;
	map<int, int>M;
	int x, y, z;
	M.clear();
	while (fgets(buf, 4096, fp) != NULL) {
		if (strncmp(buf, "Vertex", 6) == 0) {
			sscanf(buf, "Vertex %d %f %f %f", &vid, &p.x, &p.y, &p.z);
			//sscanf(buf, "Vertex %d %lf %lf %lf", &vid, &x, &y, &z);
			//p.x = x, p.y = y, p.z = z;
			vertexs.push_back(p);
			M[vid] = c;
			c++;
		}
		else if (strncmp(buf, "Face", 4) == 0) {
			sscanf(buf, "Face %d %d %d %d", &vid, &x, &y, &z);
			f.vertex[0] = M[x], f.vertex[1] = M[y], f.vertex[2] = M[z];
			faces.push_back(f);
		}
	}
	M.clear();
	fclose(fp);
	return;
}

void Model_vector::ReadPlyFile(char * filename)
{
	ifstream in(filename, ios_base::in | ios_base::binary);
	if (in.fail())
	{
		throw "fail to read file";
	}
	while (!in.eof())
	{
		string firstWord;
		ReadIntoWord(in, firstWord);
		if (firstWord == "format")
			break;
		ReadUntilNextLine(in);
	}
	if (in.eof())
	{
		throw "error Model_vector";
	}
	string format;
	ReadIntoWord(in, format);
	in.close();
	transform(format.begin(), format.end(), format.begin(), tolower);
	if (format.find("ascii") != -1)
	{
		ReadAsciiPly(filename);
	}
	else if (format.find("binary") != -1)
	{
		if (format.find("big") != -1)
			ReadBinaryPly(filename, "big");
		else
			ReadBinaryPly(filename, "little");
	}
	else
	{
		throw "error Model_vector format";
	}
}

void Model_vector::ReadAsciiPly(char * filename)
{
	ifstream in(filename, ios_base::in | ios_base::binary);
	if (in.fail())
	{
		throw "fail to read file";
	}
	int vertNum(0);
	int faceNum(0);
	while (!in.eof())
	{
		string word;
		ReadIntoWord(in, word);
		if (word == "element")
		{
			ReadIntoWord(in, word);
			if (word == "vertex")
			{
				in >> vertNum;
			}
			else if (word == "face")
			{
				in >> faceNum;
			}
		}
		else if (word == "end_header")
		{
			ReadUntilNextLine(in);
			break;
		}
	}
	map<string, int> uniqueVerts;
	vertexs.reserve(vertNum);
	faces.reserve(faceNum);
	vector<int> vertCorrespondence(vertNum);
	for (int i = 0; i < vertNum; ++i)
	{
		ostringstream outStr;
		Point3D temp;
		in >> temp.x >> temp.y >> temp.z;
		if (fabs(temp.x) < FLT_EPSILON)
			temp.x = 0;
		if (fabs(temp.y) < FLT_EPSILON)
			temp.y = 0;
		if (fabs(temp.z) < FLT_EPSILON)
			temp.z = 0;
		outStr << setiosflags(ios_base::fixed) << setprecision(5) << temp.x << " " << temp.y << " " << temp.z;
		map<string, int>::iterator pos = uniqueVerts.find(outStr.str());
		if (pos == uniqueVerts.end())
		{
			int oldSize = (int)uniqueVerts.size();
			vertCorrespondence[i] = uniqueVerts[outStr.str()] = oldSize;
			vertexs.push_back(temp);
		}
		else
		{
			vertCorrespondence[i] = pos->second;
		}
		ReadUntilNextLine(in);
	}
	vertexs.swap(vector<Point3D>(vertexs));
	for (int i = 0; i < faceNum; ++i)
	{
		int num;
		in >> num;
		vector<int> indices(num);
		for (int j = 0; j < num; ++j)
		{
			int temp;
			in >> temp;
			indices[j] = vertCorrespondence[temp];
		}
		ReadUntilNextLine(in);
		for (int j = 1; j < num - 1; ++j)
		{
			faces.push_back(Face(indices[0], indices[j], indices[j + 1]));
		}
	}
	in.close();
	faces.swap(vector<Face>(faces));
}

void Model_vector::ReadBinaryPly(char * filename, const char* format)
{
	ifstream in(filename, ios_base::in | ios_base::binary);
	if (in.fail())
	{
		throw "fail to read file";
	}
	map<string, int> types;
	types["char"] = 1;
	types["uchar"] = 1;
	types["short"] = 2;
	types["ushort"] = 2;
	types["int"] = 4;
	types["uint"] = 4;
	types["float"] = 4;
	types["double"] = 8;
	types["int8"] = 1;
	types["int16"] = 2;
	types["int32"] = 4;
	types["uint8"] = 1;
	types["uint16"] = 2;
	types["uint32"] = 4;
	types["float32"] = 4;
	types["float64"] = 8;

	int vertNum(0);
	int faceNum(0);
	while (!in.eof())
	{
		string word;
		ReadIntoWord(in, word);
		if (word == "element")
		{
			ReadIntoWord(in, word);
			if (word == "vertex")
			{
				in >> vertNum;
				break;
			}
		}
	}

	bool isFloatCoordinate(true);
	int sizeOfOneVert(0);
	string word;
	ReadIntoWord(in, word);
	if (!in.eof() && word == "property")
	{
		ReadIntoWord(in, word);
		transform(word.begin(), word.end(), word.begin(), tolower);
		if (types.find(word) != types.end())
		{
			if (types[word] == 4)
			{
				isFloatCoordinate = true;
			}
			else if (types[word] == 8)
			{
				isFloatCoordinate = false;
			}
			else
			{
				throw "read file error";
			}
			sizeOfOneVert += types[word];
		}
		else
		{
			throw "read file error";
		}
		ReadUntilNextLine(in);
	}
	ReadIntoWord(in, word);
	while (word == "property")
	{
		ReadIntoWord(in, word);
		transform(word.begin(), word.end(), word.begin(), tolower);
		sizeOfOneVert += types[word];
		ReadUntilNextLine(in);
		ReadIntoWord(in, word);
	}
	while (word != "element" || (ReadIntoWord(in, word), word != "tristrips" && word != "face"))
	{
		ReadUntilNextLine(in);
		ReadIntoWord(in, word);
	}

	in >> faceNum;
	if (word == "tristrips")
		faceNum = 0;
	ReadUntilNextLine(in);
	ReadIntoWord(in, word);
	while (word != "property" || (ReadIntoWord(in, word), word != "list"))
	{
		ReadUntilNextLine(in);
		ReadIntoWord(in, word);
	}
	ReadIntoWord(in, word);
	transform(word.begin(), word.end(), word.begin(), tolower);
	if (types.find(word) == types.end())
	{
		throw "file read error";
	}
	int szOfNumInFace = types[word];
	ReadIntoWord(in, word);
	transform(word.begin(), word.end(), word.begin(), tolower);
	if (types.find(word) == types.end())
	{
		throw "file read error";
	}
	int szOfVertIndex = types[word];
	do
	{
		ReadIntoWord(in, word);
		ReadUntilNextLine(in);
	} while (word != "end_header");

	map<string, int> uniqueVerts;
	vertexs.reserve(vertNum);
	faces.reserve(faceNum);
	vector<int> vertCorrespondence(vertNum);

	for (int i = 0; i < vertNum; ++i)
	{
		ostringstream outStr;
		int szRemaining = sizeOfOneVert;
		Point3D pt;
		if (isFloatCoordinate)
		{
			float temp[3];
			szRemaining -= 12;
			for (int j = 0; j < 3; ++j)
			{
				float tmp;
				in.read((char *)&tmp, 4);
				if (format == "big")
				{
					SwapOrder((char*)&tmp, 4);
				}
				if (fabs(tmp) < FLT_EPSILON)
					tmp = 0;
				temp[j] = tmp;
			}
			pt.x = temp[0];
			pt.y = temp[1];
			pt.z = temp[2];
		}
		else
		{
			float temp[3];
			szRemaining -= 24;
			for (int j = 0; j < 3; ++j)
			{
				float tmp;
				in.read((char *)&tmp, 8);
				if (format == "big")
				{
					SwapOrder((char*)&tmp, 8);
				}
				if (fabs(tmp) < FLT_EPSILON)
					tmp = 0;
				temp[j] = tmp;
			}
			pt.x = temp[0];
			pt.y = temp[1];
			pt.z = temp[2];
		}
		outStr << setiosflags(ios_base::fixed) << setprecision(5) << pt.x << " " << pt.y << " " << pt.z;
		map<string, int>::iterator pos = uniqueVerts.find(outStr.str());
		if (pos == uniqueVerts.end())
		{
			int oldSize = (int)uniqueVerts.size();
			vertCorrespondence[i] = uniqueVerts[outStr.str()] = oldSize;
			vertexs.push_back(pt);
		}
		else
		{
			vertCorrespondence[i] = pos->second;
		}
		if (szRemaining == 0)
			continue;
		char buf[64];
		in.read((char*)&buf, szRemaining);
	}
	vertexs.swap(vector<Point3D>(vertexs));

	if (faceNum != 0)
	{
		for (int i = 0; i < faceNum; ++i)
		{
			int num(0);
			if (szOfNumInFace == 1)
			{
				char ch;
				in.read(&ch, 1);
				num = ch;
			}
			else if (szOfNumInFace == 2)
			{
				short ch;
				in.read((char*)&ch, 2);
				if (format == "big")
				{
					SwapOrder((char*)&ch, 2);
				}
				num = ch;
			}
			else if (szOfNumInFace == 4)
			{
				int ch;
				in.read((char*)&ch, 4);
				if (format == "big")
				{
					SwapOrder((char*)&ch, 4);
				}
				num = ch;
			}
			vector<int> indices(num);
			for (int j = 0; j < num; ++j)
			{
				int index(0);
				if (szOfVertIndex == 2)
				{
					short ch;
					in.read((char*)&ch, 2);
					if (format == "big")
					{
						SwapOrder((char*)&ch, 2);
					}
					index = ch;
				}
				else if (szOfVertIndex == 4)
				{
					int ch;
					in.read((char*)&ch, 4);
					if (format == "big")
					{
						SwapOrder((char*)&ch, 4);
					}
					index = ch;
				}
				indices[j] = vertCorrespondence[index];
			}

			for (int j = 1; j < num - 1; ++j)
			{
				faces.push_back(Face(indices[0], indices[j], indices[j + 1]));
			}
		}
	}
	else
	{
		if (szOfNumInFace == 1)
		{
			char ch;
			in.read((char*)&ch, 1);
			faceNum = ch;
		}
		else if (szOfNumInFace == 2)
		{
			short ch;
			in.read((char*)&ch, 2);
			if (format == "big")
			{
				SwapOrder((char*)&ch, 2);
			}
			faceNum = ch;
		}
		else if (szOfNumInFace == 4)
		{
			int ch;
			in.read((char*)&ch, 4);
			if (format == "big")
			{
				SwapOrder((char*)&ch, 4);
			}
			faceNum = ch;
		}
		vector<int> magicBox;
		int minPos(0);
		bool fInverse(false);
		for (int i = 0; i < faceNum; ++i)
		{
			int curIndex;
			if (szOfVertIndex == 1)
			{
				char ch;
				in.read((char*)&ch, 1);
				curIndex = ch;
			}
			else if (szOfVertIndex == 2)
			{
				short ch;
				in.read((char*)&ch, 2);
				if (format == "big")
				{
					SwapOrder((char*)&ch, 2);
				}
				curIndex = ch;
			}
			else if (szOfVertIndex == 4)
			{
				int ch;
				in.read((char*)&ch, 4);
				if (format == "big")
				{
					SwapOrder((char*)&ch, 4);
				}
				curIndex = ch;
			}

			if (curIndex == -1)
			{
				magicBox.clear();
				minPos = 0;
				fInverse = false;
				continue;
			}
			else
			{
				curIndex = vertCorrespondence[curIndex];
			}
			if (magicBox.size() < 3)
			{
				magicBox.push_back(curIndex);
			}
			else
			{
				magicBox[minPos] = curIndex;
				fInverse = !fInverse;
			}
			minPos = (minPos + 1) % 3;
			if (magicBox.size() >= 3)
			{
				if (fInverse)
					faces.push_back(Face(magicBox[0], magicBox[2], magicBox[1]));
				else
					faces.push_back(Face(magicBox[0], magicBox[1], magicBox[2]));
			}
		}
	}
	in.close();
	faces.swap(vector<Face>(faces));
}

//eat off the remainder of the current line.
void Model_vector::ReadUntilNextLine(ifstream& in) const
{
	if (in.eof())
		return;
	char ch;
	while (in.read(&ch, 1), !(ch == 0x0A || ch == 0x0D || in.eof()));
	if (in.eof())
		return;
	while (in.read(&ch, 1), (ch == 0x0A || ch == 0x0D) && !in.eof());
	if (in.eof())
		return;
	in.putback(ch);
}

void Model_vector::SwapOrder(char *buf, int sz)
{
	for (int i = 0; i < sz / 2; ++i)
	{
		char temp = buf[i];
		buf[i] = buf[sz - 1 - i];
		buf[sz - 1 - i] = temp;
	}
}

void Model_vector::ReadIntoWord(ifstream &in, string& word) const
{
	word.clear();
	char ch;
	while (in.read(&ch, 1), (ch == 0xA || ch == 0xD || ch == ' ' || ch == '\t' || ch == '\n') && !in.eof());
	while (!(ch == 0xA || ch == 0xD || ch == ' ' || ch == '\t' || ch == '\n') && !in.eof())
	{
		word.push_back(ch);
		in.read(&ch, 1);
	}
	if (in.eof())
		return;
	in.putback(ch);
}