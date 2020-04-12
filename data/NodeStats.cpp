#include "NodeStats.h"


//*************************************************************
//************************ MapNode(32) ************************
//*************************************************************

bool MapNode::rightSubsector() { return (rightIndex & 0x8000) != 0; }

bool MapNode::leftSubsector() { return (leftIndex & 0x8000) != 0; }

int32_t MapNode::rightChild() { return rightIndex & 0x7FFF; }

int32_t MapNode::leftChild() { return leftIndex & 0x7FFF; }


bool MapNode32::rightSubsector() { return (rightIndex & 0x80000000) != 0; }

bool MapNode32::leftSubsector() { return (leftIndex & 0x80000000) != 0; }

int32_t MapNode32::rightChild() { return rightIndex & 0x7FFFFFFF; }

int32_t MapNode32::leftChild() { return leftIndex & 0x7FFFFFFF; }


//**********************************************************
//************************ NodeDraw ************************
//**********************************************************

NodeDraw::NodeDraw(int scale)
: bm(NULL), dc(NULL), drawScale(scale)
{
}

NodeDraw::~NodeDraw()
{
	if (dc != NULL) {
		delete dc;
		delete bm;
	}
}

void NodeDraw::setup(Vector2D& minXY, Vector2D& maxXY)
{
	xTrans = (int)minXY.x;
	yTrans = (int)maxXY.y;
	int width = (maxXY.x - minXY.x)/drawScale;
	width+=3;
	int height = (maxXY.y - minXY.y)/drawScale;
	height+=3;
	bm = new wxBitmap(width, height, wxBITMAP_SCREEN_DEPTH);
	dc = new wxMemoryDC(*bm);

	wxBrush bckgrnd(DRAW_BACKGROUND);
	dc->SetBackground(bckgrnd);
	dc->Clear();
	wxPen pen(DRAW_FOREGROUND, 1, wxSOLID);
	dc->SetPen(pen);
}

void NodeDraw::drawPoly(vector<Vector2D>* poly)
{
	int fend = poly->size() - 1;
	int x1, y1, x2, y2;
	for (int i=0; i<fend; i++) {
		x1 = ((poly->at(i).x - xTrans)/drawScale) + 1;
        y1 = (( (poly->at(i).y * -1) + yTrans)/drawScale) + 1;
        x2 = ((poly->at(i+1).x - xTrans)/drawScale) + 1;
        y2 = (( (poly->at(i+1).y * -1) + yTrans)/drawScale) + 1;
		dc->DrawLine(x1, y1, x2, y2);
	}
	x1 = ((poly->at(fend).x - xTrans)/drawScale) + 1;
	y1 = (( (poly->at(fend).y * -1) + yTrans)/drawScale) + 1;
	x2 = ((poly->at(0).x - xTrans)/drawScale) + 1;
	y2 = (( (poly->at(0).y * -1) + yTrans)/drawScale) + 1;
	dc->DrawLine(x1, y1, x2, y2);
}

void NodeDraw::saveImage(wxString fileName)
{
	dc->SelectObject(wxNullBitmap);
	bool ok = bm->SaveFile(fileName, wxBITMAP_TYPE_PNG);
}


//***********************************************************
//************************ NodeStats ************************
//***********************************************************

NodeStats::NodeStats(vector<Vertex>* vert, vector<MapLine>* lin)
: progress(NULL), nodes(0), segs(NULL), ssectors(NULL), nodeList(NULL), area(0.0), invalids(0)
{
	vertices = vert;
	lines = lin;
	draw = NULL; //new NodeDraw(16); //For debug: new NodeDraw(8);
}

NodeStats::~NodeStats()
{
	if (draw != NULL)
		delete draw;
	if (segs != NULL)
		delete segs;
	if (ssectors != NULL)
		delete ssectors;
	if (nodeList != NULL) {
		for (int i=0; i<nodes; i++)
			delete nodeList->at(i);
		delete nodeList;
	}
}

wxString NodeStats::getTypeLabel()
{
	return "Basic";
}

bool NodeStats::isGL()
{
	return false;
}

void NodeStats::readFile(wxInputStream* file, vector<DirEntry*>* lumps)
{
	wxLogVerbose("Processing nodes of type %s", getTypeLabel());
	DirEntry* lump;
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		wxString lname(lump->name);
		if (lname.CmpNoCase("SEGS")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processSegs(file, lump->size);
		} else if (lname.CmpNoCase("SSECTORS")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processSSectors(file, lump->size);
		} else if (lname.CmpNoCase("NODES")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processNodes(file, lump->size);
		}
	}
}

bool NodeStats::checkNodes()
{
	if (segs==NULL || segs->size()<=1)
		return false;
	if (ssectors==NULL || ssectors->size()<=1)
		return false;
	if (nodeList==NULL || nodeList->size()<=1)
		return false;
	return true;
}

/*
bool NodeStats::checkNodes()
{
	int32_t index;
	int32_t minIndex = 1000000;
	int32_t maxIndex = -1000000;
	int ncount = 0;
	int scount = 0;
	int32_t ssect = ssectors->size();
	for (int i=0; i<nodes; i++) {
		index = nodeList->at(i)->rightChild();
		if (nodeList->at(i)->rightSubsector()) {
			scount++;
			if (index < minIndex) minIndex=index;
			if (index > maxIndex) maxIndex=index;
			if (index >= ssect)
				wxLogVerbose("Node Subsector index out of bounds!");
		} else {
			ncount++;
			if (index >= nodes)
				wxLogVerbose("Node node index out of bounds, is %i in node %i!", index, i);
		}
		index = nodeList->at(i)->leftChild();
		if (nodeList->at(i)->leftSubsector()) {
			scount++;
			if (index < minIndex) minIndex=index;
			if (index > maxIndex) maxIndex=index;
			if (index >= ssect)
				wxLogVerbose("Node Subsector index out of bounds!");
		} else {
			ncount++;
			if (index >= nodes)
				wxLogVerbose("Node node index out of bounds, is %i in node %i!", index, i);
		}
	}
	wxLogVerbose("Subsector index range %i to %i", minIndex, maxIndex);
	wxLogVerbose("Found %i subsector refs", scount);
	wxLogVerbose("Found %i node refs", ncount);
	if ((scount>ssect) || (ncount>nodes)) return false;
	else return true;
}*/

double NodeStats::computeArea(Vector2D minXY, Vector2D maxXY)
{
	wxLogVerbose("Calculate area");
	minx = minXY.x;
	maxx = maxXY.x;
	miny = minXY.y;
	maxy = maxXY.y;

	if (draw != NULL)
		draw->setup(minXY, maxXY);

	vector<Split*>* splitStack = new vector<Split*>();
	recursiveBuildSubsectorPoly(nodeList->size()-1, splitStack);
	for (int i=0; i<splitStack->size(); i++)
		delete splitStack->at(i);
	delete splitStack;

	if (draw != NULL)
		draw->saveImage("F:\\Doom\\DMDB\\temp\\nodes.png");
	return area;
}

void NodeStats::printReport(TextReport* reportView)
{
	reportView->writeLine("Node type:\t" + getTypeLabel());
	reportView->writeText("Total vertices (incl. nodes):\t");
	reportView->writeLine(wxString::Format("%i", vertices->size()));
	reportView->writeText("Segs:\t");
	if (segs==NULL)
		reportView->writeLine("No");
	else
		reportView->writeLine(wxString::Format("%i", segs->size()));
	reportView->writeText("SubSectors:\t");
	if (ssectors==NULL)
		reportView->writeLine("No");
	else
		reportView->writeLine(wxString::Format("%i", ssectors->size()));
	reportView->writeText("Nodes:\t");
	reportView->writeLine(wxString::Format("%i", nodes));
	reportView->writeText("SubSector area (sq.units):\t");
	reportView->writeLine(wxString::Format("%.0f", area));
	reportView->writeText("Invalid SubSectors:\t");
	reportView->writeLine(wxString::Format("%i", invalids));
}

void NodeStats::processSegs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/12;
	wxLogVerbose("Processing SEGS - %i entries", num);
	//Need startVertex and endVertex for creating subsector polygons
	segs = new vector<PairUint>();
	segs->reserve(num);
	int vertCount = vertices->size();
	uint16_t ld, dir;
	for (int i=0; i<num; i++) {
		PairUint seg;
		file->Read(&seg.first, 2);
		file->Read(&seg.second, 2);
		if ((seg.first>vertCount) || (seg.second>vertCount)) {
			//Vertex index out of bounds, use linedef
			file->SeekI(2, wxFromCurrent);
			file->Read(&ld, 2);
			file->Read(&dir, 2);
			if (dir == 0) {
				seg.first = lines->at(ld).v1;
				seg.second = lines->at(ld).v2;
			} else {
				seg.first = lines->at(ld).v2;
				seg.second = lines->at(ld).v1;
			}
			file->SeekI(2, wxFromCurrent);
		} else {
			file->SeekI(8, wxFromCurrent);
		}
		segs->push_back(seg);
	}
	progress->incrCount(10);
}

void NodeStats::processSSectors(wxInputStream* file, int32_t lsize)
{
	int num = lsize/4;
	wxLogVerbose("Processing SSECTORS - %i entries", num);
	ssectors = new vector<PairUint>();
	ssectors->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint ss;
		file->Read(&ss.first, 2);
		file->Read(&ss.second, 2);
		ssectors->push_back(ss);
	}
	progress->incrCount(10);
}

void NodeStats::processNodes(wxInputStream* file, int32_t lsize)
{
	nodes = lsize/28;
	wxLogVerbose("Processing NODES - %i entries", nodes);
	nodeList = new vector<MapNode*>();
	nodeList->reserve(nodes);
	for (int i=0; i<nodes; i++) {
		MapNode* node = new MapNode();
		file->Read(&(node->lineStart.x), 2);
		file->Read(&(node->lineStart.y), 2);
		file->Read(&(node->lineDelta.x), 2);
		file->Read(&(node->lineDelta.y), 2);
		file->SeekI(16, wxFromCurrent);
		file->Read(&(node->rightIndex), 2);
		file->Read(&(node->leftIndex), 2);
		nodeList->push_back(node);
	}
	progress->incrCount(10);
}

void NodeStats::recursiveBuildSubsectorPoly(int nodeIndex, vector<Split*>* splits)
{
	/*if (nodeIndex >= nodes) {
		wxLogVerbose("Node index %i is out of bounds!", nodeIndex);
		return;
	}*/
	MapNode* n = nodeList->at(nodeIndex);
	//Left side
	Split* s = new Split(n->lineStart, n->lineDelta.inverse()); //"-": Need to reverse
	splits->push_back(s);
	if (n->leftSubsector())
		buildSubsectorPoly(n->leftChild(), splits);
	else
		recursiveBuildSubsectorPoly(n->leftChild(), splits);

	s = splits->back();
	splits->pop_back();
	delete s;

	//Right side
	s = new Split(n->lineStart, n->lineDelta);
	splits->push_back(s);
	if (n->rightSubsector())
		buildSubsectorPoly(n->rightChild(), splits);
	else
		recursiveBuildSubsectorPoly(n->rightChild(), splits);

	s = splits->back();
	splits->pop_back();
	delete s;
}

void NodeStats::buildSubsectorPoly(int ss, vector<Split*>* nodesplits)
{
	int16_t MIN = -32768;
	int16_t MAX = 32767;

	//Begin with a giant square polygon that covers the entire map
	vector<Vector2D>* poly = new vector<Vector2D>(); //16
	poly->push_back(Vector2D((double)MIN,(double)MAX));
	poly->push_back(Vector2D((double)MAX,(double)MAX));
	poly->push_back(Vector2D((double)MAX,(double)MIN));
	poly->push_back(Vector2D((double)MIN,(double)MIN));

	//Crop the polygon by the node tree splits
	for (int i=0; i<nodesplits->size(); i++)
		poly = cropPolygon(poly, nodesplits->at(i));

	//Crop the polygon by the subsector segs
	PairUint& ssect = ssectors->at(ss);
	int vertCount = vertices->size();
	for (int i=0; i<ssect.first; i++) {
		PairUint sg = segs->at(ssect.second + i);
		if (sg.first<vertCount && sg.second<vertCount) {
			Vertex lStart = vertices->at(sg.first);
			int16_t xDelta = vertices->at(sg.second).x - vertices->at(sg.first).x;
			int16_t yDelta = vertices->at(sg.second).y - vertices->at(sg.first).y;
			Vertex lDelta(xDelta, yDelta);
			Split* s = new Split(lStart, lDelta);
			poly = cropPolygon(poly, s);
			delete s;
		} else {
			//Vertex index out of bounds
			invalids++;
			delete poly;
			return;
		}
	}

	//Check for bad subsector - outside map area
	bool outside = false;
	for (int i=0; i<poly->size(); i++) {
		if ((poly->at(i).x < minx) || (poly->at(i).x > maxx))
			outside = true;
		if ((poly->at(i).y < miny) || (poly->at(i).y > maxy))
			outside = true;
		if (outside) {
			wxLogVerbose("Erroneous subSector %i - outside map area", ss);
			break;
		}
	}

	//Ready to calculate area of ssector
	//Some polygons can end up with size 0
	if (poly->size()>=3 && !outside) {
		double ar = polygonArea(poly);
		/*if (ss<3) {
			for (int j=0; j<poly->size(); j++)
				wxLogVerbose("[%.0f, %.0f]", poly->at(j).x, poly->at(j).y);
		}*/
		//wxLogVerbose("Area of subSector %i (%i points) is %f", ss, poly->size(), ar);
		area += ar;
	} else {
		invalids++;
		/*if (!outside) {
			wxLogVerbose("subSector %i polygon with only %i points", ss, poly->size());
			Split* spl;
			for (int i=0; i<nodesplits->size(); i++) {
				spl = nodesplits->at(i);
				wxLogVerbose("Split point %i %i  delta %i %i",
						spl->lineStart.x, spl->lineStart.y, spl->lineDelta.x, spl->lineDelta.y);
			}
			for (int i=0; i<ssect.first; i++) {
				PairUint sg = segs->at(ssect.second + i);
				Vertex v1 = vertices->at(sg.first);
				Vertex v2 = vertices->at(sg.second);
				wxLogVerbose("Segment %i %i  to  %i %i", v1.x, v1.y, v2.x, v2.y);
			}
		}*/
	}
	delete poly;
}

vector<Vector2D>* NodeStats::cropPolygon(vector<Vector2D>* poly, Split* split)
{
	if (poly->size() == 0) return poly;
	Vector2D prev = poly->back();
	float side1 = (prev.y - split->lineStart.y) * split->lineDelta.x - (prev.x - split->lineStart.x) * split->lineDelta.y;

	vector<Vector2D>* newp = new vector<Vector2D>();
	for (int i=0; i<poly->size(); i++) {
		Vector2D cur = poly->at(i);
		float side2 = (cur.y - split->lineStart.y) * split->lineDelta.x - (cur.x - split->lineStart.x) * split->lineDelta.y;

		// Front?
		if (side2 < -EPSILON) {
			if (side1 > EPSILON) {
				// Split line with plane and insert the vertex
				Vertex splitSum(split->lineStart.x + split->lineDelta.x,
					split->lineStart.y + split->lineDelta.y);
				float u = getIntersection(split->lineStart, splitSum,
					prev.x, prev.y, cur.x, cur.y);
				float xx = prev.x + (cur.x - prev.x) * u;
				float yy = prev.y + (cur.y - prev.y) * u;
				Vector2D newv(xx,yy);
				newp->push_back(newv);
			}
			newp->push_back(cur);

		// Back?
		} else if (side2 > EPSILON) {
			if (side1 < -EPSILON) {
				// Split line with plane and insert the vertex
				Vertex splitSum(split->lineStart.x + split->lineDelta.x,
					split->lineStart.y + split->lineDelta.y);
				float u = getIntersection(split->lineStart, splitSum,
					prev.x, prev.y, cur.x, cur.y);
				float xx = prev.x + (cur.x - prev.x) * u;
				float yy = prev.y + (cur.y - prev.y) * u;
				Vector2D newv(xx,yy);
				newp->push_back(newv);
			}
		} else {
			// On the plane
			newp->push_back(cur);
		}

		// Next
		prev = cur;
		side1 = side2;
	}
	//newp replaces poly
	delete poly;
	return newp;
}

float NodeStats::getIntersection(Vertex v1, Vertex v2, float x3, float y3, float x4, float y4)
{
	// Calculate divider
	float div = (y4 - y3) * (float)(v2.x - v1.x) - (x4 - x3) * (float)(v2.y - v1.y);
	if (div != 0.0f) {
		// Calculate the intersection distance from the line
		//float u_line = ((x4 - x3) * (v1.y - y3) - (y4 - y3) * (v1.x - x3)) / div;

		// Calculate the intersection distance from the ray
		float u_ray = ((float)(v2.x - v1.x) * ((float)v1.y - y3) - (float)(v2.y - v1.y) * ((float)v1.x - x3)) / div;

		// Check if intersecting
		//return (u_ray >= 0.0f) && (u_ray <= 1.0f) && (u_line >= 0.0f) && (u_line <= 1.0f);

		return u_ray;
	} else {
		// Unable to detect intersection
		return 0.0;
	}
}

double NodeStats::polygonArea(vector<Vector2D>* poly)
{
	if (draw != NULL)
		draw->drawPoly(poly);

	double area = 0.0;
	int fend = poly->size() - 1;
	for (int i=0; i<fend; i++)
		area += (poly->at(i).x * poly->at(i+1).y) - (poly->at(i).y * poly->at(i+1).x);
	area += (poly->at(fend).x * poly->at(0).y) - (poly->at(fend).y * poly->at(0).x);
	if (area < 0.0)
		return area/-2.0;
	else
		return area/2.0;
}


//***************************************************************
//************************ DeepNodeStats ************************
//***************************************************************

DeepNodeStats::DeepNodeStats(vector<Vertex>* vert, vector<MapLine>* lin)
: NodeStats(vert,lin)
{
}

wxString DeepNodeStats::getTypeLabel()
{
	return "Deep";
}

void DeepNodeStats::processSegs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/16;
	wxLogVerbose("Processing SEGS - %i entries", num);
	//Need startVertex and endVertex for creating subsector polygons
	segs = new vector<PairUint>();
	segs->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint seg;
		file->Read(&seg.first, 4);
		file->Read(&seg.second, 4);
		segs->push_back(seg);
		file->SeekI(8, wxFromCurrent);
	}
	progress->incrCount(10);
}

void DeepNodeStats::processSSectors(wxInputStream* file, int32_t lsize)
{
	int num = lsize/6;
	wxLogVerbose("Processing SSECTORS - %i entries", num);
	ssectors = new vector<PairUint>();
	ssectors->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint ss;
		file->Read(&ss.first, 2);
		file->Read(&ss.second, 4);
		ssectors->push_back(ss);
	}
	progress->incrCount(10);
}

void DeepNodeStats::processNodes(wxInputStream* file, int32_t lsize)
{
	file->SeekI(8, wxFromCurrent);
	nodes = (lsize-8)/32;
	wxLogVerbose("Processing NODES - %i entries", nodes);
	nodeList = new vector<MapNode*>();
	nodeList->reserve(nodes);
	for (int i=0; i<nodes; i++) {
		MapNode32* node = new MapNode32();
		file->Read(&(node->lineStart.x), 2);
		file->Read(&(node->lineStart.y), 2);
		file->Read(&(node->lineDelta.x), 2);
		file->Read(&(node->lineDelta.y), 2);
		file->SeekI(16, wxFromCurrent);
		file->Read(&(node->rightIndex), 4);
		file->Read(&(node->leftIndex), 4);
		nodeList->push_back(node);
	}
	progress->incrCount(10);
}


//****************************************************************
//************************ ZDoomNodeStats ************************
//****************************************************************

ZDoomNodeStats::ZDoomNodeStats(vector<Vertex>* vert, vector<MapLine>* lin)
: NodeStats(vert,lin)
{
	//draw = new NodeDraw(16); //For debug
}

wxString ZDoomNodeStats::getTypeLabel()
{
	return "ZDoom (XNOD)";
}

void ZDoomNodeStats::processSegs(wxInputStream* file, int32_t lsize)
{
}

void ZDoomNodeStats::processSSectors(wxInputStream* file, int32_t lsize)
{
}

void ZDoomNodeStats::processNodes(wxInputStream* file, int32_t lsize)
{
	long lumpLimit = file->TellI() + lsize + 1;
	file->SeekI(4, wxFromCurrent); //"XNOD"

	//Vertices
	zVertexes(file);
	progress->incrCount(10);
	uint32_t totalVertices = vertices->size();

	//Subsectors
	uint32_t seg = zSubsectors(file);
	progress->incrCount(10);

	//Segs
	zSegs(file, seg);
	progress->incrCount(10);

	//Nodes
	zNodes(file);
	progress->incrCount(10);

	//Sanity check:
	if (file->TellI() > lumpLimit)
		wxLogVerbose("Error: Read beyond lump");
}

void ZDoomNodeStats::zVertexes(wxInputStream* file)
{
	uint32_t vert;
	file->Read(&vert, 4); //Number of original vertices used
	if (vert != vertices->size())
		wxLogVerbose("Mismatch between VERTEXES size %i and node OrgVerts %i",
				vertices->size(), vert);
		//What to do with this mismatch?
	file->Read(&vert, 4); //Number of new vertices
	wxLogVerbose("Reading %i node vertices", vert);
	int16_t x, y, frac;
	for (int i=0; i<vert; i++) {
		//How to handle fractions?
		//Round off to nearest int
		file->Read(&frac, 2);
		file->Read(&x, 2);
		if (frac>=0x8000) x++;
		file->Read(&frac, 2);
		file->Read(&y, 2);
		if (frac>=0x8000) y++;
		vertices->push_back(Vertex(x,y));
	}
}

uint32_t ZDoomNodeStats::zSubsectors(wxInputStream* file)
{
	uint32_t ssect;
	file->Read(&ssect, 4);
	wxLogVerbose("Reading %i node SSECTORS", ssect);
	ssectors = new vector<PairUint>();
	ssectors->reserve(ssect);
	uint32_t seg = 0;
	for (int i=0; i<ssect; i++) {
		//Traditional ssector entry is (numberOfSegments, firstSegment)
		//Here it is just numberOfSegments
		PairUint ss;
		file->Read(&ss.first, 4);
		ss.second = seg;
		ssectors->push_back(ss);
		seg += ss.first;
	}
	return seg;
}

void ZDoomNodeStats::zSegs(wxInputStream* file, uint32_t seg)
{
	//Need startVertex and endVertex for creating subsector polygons
	uint32_t segCount;
	file->Read(&segCount, 4);
	wxLogVerbose("Reading %i node SEGS", segCount);
	if (segCount != seg)
		wxLogVerbose("Mismatch between %i SEGS and %i total segments in SSECTORS", segCount, seg);
	segs = new vector<PairUint>();
	segs->reserve(segCount);
	for (int i=0; i<segCount; i++) {
		PairUint seg;
		file->Read(&seg.first, 4);
		file->Read(&seg.second, 4);
		segs->push_back(seg);
		file->SeekI(3, wxFromCurrent);
		//if ((seg.first>=totalVertices) || (seg.second>=totalVertices))
		//	wxLogVerbose("Segment vertex index out of bounds!");
	}
}

void ZDoomNodeStats::zNodes(wxInputStream* file)
{
	file->Read(&nodes, 4);
	wxLogVerbose("Reading %i NODES", nodes);
	nodeList = new vector<MapNode*>();
	nodeList->reserve(nodes);
	for (int i=0; i<nodes; i++) {
		MapNode32* node = new MapNode32();
		file->Read(&(node->lineStart.x), 2);
		file->Read(&(node->lineStart.y), 2);
		file->Read(&(node->lineDelta.x), 2);
		file->Read(&(node->lineDelta.y), 2);
		file->SeekI(16, wxFromCurrent);
		file->Read(&(node->rightIndex), 4);
		file->Read(&(node->leftIndex), 4);
		nodeList->push_back(node);
	}
}


//*********************************************************************
//************************ ZDoomComprNodeStats ************************
//*********************************************************************

ZDoomComprNodeStats::ZDoomComprNodeStats(vector<Vertex>* vert, vector<MapLine>* lin)
: ZDoomNodeStats(vert,lin)
{
}

wxString ZDoomComprNodeStats::getTypeLabel()
{
	return "ZDoom compressed (ZNOD)";
}

double ZDoomComprNodeStats::computeArea(Vector2D minXY, Vector2D maxXY)
{
	return 0.0;
}

void ZDoomComprNodeStats::processNodes(wxInputStream* file, int32_t lsize)
{
}


//***************************************************************
//************************ GLv2NodeStats ************************
//***************************************************************

GLv2NodeStats::GLv2NodeStats(vector<Vertex>* vert)
: NodeStats(vert, NULL)
{
	glVertStart = vert->size();
	//draw = new NodeDraw(16); //For debug
}

wxString GLv2NodeStats::getTypeLabel()
{
	return "GL v2";
}

bool GLv2NodeStats::isGL()
{
	return true;
}

void GLv2NodeStats::readFile(wxInputStream* file, vector<DirEntry*>* lumps)
{
	wxLogVerbose("Processing nodes of type %s", getTypeLabel());
	DirEntry* lump;
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		wxString lname(lump->name);
		if (lname.CmpNoCase("GL_VERT")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processVert(file, lump->size);
		} else if (lname.CmpNoCase("GL_SEGS")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processSegs(file, lump->size);
		} else if (lname.CmpNoCase("GL_SSECT")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processSSectors(file, lump->size);
		} else if (lname.CmpNoCase("GL_NODES")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processNodes(file, lump->size);
			//Not actually needed for GL nodes
		}
	}
}

void GLv2NodeStats::processVert(wxInputStream* file, int32_t lsize)
{
	file->SeekI(4, wxFromCurrent); //"gNd2"
	int vert = (lsize-4)/8;
	wxLogVerbose("Processing GL_VERT - %i entries", vert);
	int16_t x, y, frac;
	for (int i=0; i<vert; i++) {
		//How to handle fractions?
		//Round off to nearest int
		file->Read(&frac, 2);
		file->Read(&x, 2);
		if (frac>=0x8000) x++;
		file->Read(&frac, 2);
		file->Read(&y, 2);
		if (frac>=0x8000) y++;
		vertices->push_back(Vertex(x,y));
	}
	progress->incrCount(10);
}

void GLv2NodeStats::processSegs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/10;
	wxLogVerbose("Processing GL_SEGS - %i entries", num);
	//Need startVertex and endVertex for creating subsector polygons
	segs = new vector<PairUint>();
	segs->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint seg;
		file->Read(&seg.first, 2);
		file->Read(&seg.second, 2);
		if ((seg.first&0x8000) != 0) {
			seg.first &= 0x7FFF;
			seg.first += glVertStart;
		}
		if ((seg.second&0x8000) != 0) {
			seg.second &= 0x7FFF;
			seg.second += glVertStart;
		}
		segs->push_back(seg);
		file->SeekI(6, wxFromCurrent);
	}
	progress->incrCount(10);
}

double GLv2NodeStats::computeArea(Vector2D minXY, Vector2D maxXY)
{
	wxLogVerbose("Calculate area");
	if (draw != NULL)
		draw->setup(minXY, maxXY);

	//Each subsector should be complete and closed, so we don't
	//need the splits of the node tree.
	int ss = ssectors->size();
	int verts = vertices->size();
	vector<Vector2D>* poly;
	for (int i=0; i<ss; i++) {
		PairUint& ssect = ssectors->at(i);
		poly = new vector<Vector2D>();
		for (int j=0; j<ssect.first; j++) {
			PairUint sg = segs->at(ssect.second + j);
			poly->push_back(Vector2D((double)vertices->at(sg.first).x,
					(double)vertices->at(sg.first).y));
		}
		area += polygonArea(poly);
		delete poly;
	}

	if (draw != NULL)
		draw->saveImage("F:\\Doom\\DMDB\\temp\\nodes.png");
	return area;
}


//***************************************************************
//************************ GLv5NodeStats ************************
//***************************************************************

GLv5NodeStats::GLv5NodeStats(vector<Vertex>* vert)
: GLv2NodeStats(vert)
{
}

wxString GLv5NodeStats::getTypeLabel()
{
	return "GL v5";
}

void GLv5NodeStats::processSegs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/16;
	wxLogVerbose("Processing GL_SEGS - %i entries", num);
	//Need startVertex and endVertex for creating subsector polygons
	segs = new vector<PairUint>();
	segs->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint seg;
		file->Read(&seg.first, 4);
		file->Read(&seg.second, 4);
		if ((seg.first&0x80000000) != 0) {
			seg.first &= 0x7FFFFFFF;
			seg.first += glVertStart;
		}
		if ((seg.second&0x80000000) != 0) {
			seg.second &= 0x7FFFFFFF;
			seg.second += glVertStart;
		}
		segs->push_back(seg);
		file->SeekI(8, wxFromCurrent);
	}
	progress->incrCount(10);
}

void GLv5NodeStats::processSSectors(wxInputStream* file, int32_t lsize)
{
	int num = lsize/8;
	wxLogVerbose("Processing SSECTORS - %i entries", num);
	ssectors = new vector<PairUint>();
	ssectors->reserve(num);
	for (int i=0; i<num; i++) {
		PairUint ss;
		file->Read(&ss.first, 4);
		file->Read(&ss.second, 4);
		ssectors->push_back(ss);
	}
	progress->incrCount(10);
}

void GLv5NodeStats::processNodes(wxInputStream* file, int32_t lsize)
{
	nodes = lsize/32;
	wxLogVerbose("Processing NODES - %i entries", nodes);
	nodeList = new vector<MapNode*>();
	nodeList->reserve(nodes);
	for (int i=0; i<nodes; i++) {
		MapNode32* node = new MapNode32();
		file->Read(&(node->lineStart.x), 2);
		file->Read(&(node->lineStart.y), 2);
		file->Read(&(node->lineDelta.x), 2);
		file->Read(&(node->lineDelta.y), 2);
		file->SeekI(16, wxFromCurrent);
		file->Read(&(node->rightIndex), 4);
		file->Read(&(node->leftIndex), 4);
		nodeList->push_back(node);
	}
	progress->incrCount(10);
}


//******************************************************************
//************************ ZDoomGLNodeStats ************************
//******************************************************************

ZDoomGLNodeStats::ZDoomGLNodeStats(vector<Vertex>* vert)
: ZDoomNodeStats(vert, NULL)
{
	//draw = new NodeDraw(16); //For debug
}

wxString ZDoomGLNodeStats::getTypeLabel()
{
	return "ZDoom GL (XGLN)";
}

bool ZDoomGLNodeStats::isGL()
{
	return true;
}

void ZDoomGLNodeStats::readFile(wxInputStream* file, vector<DirEntry*>* lumps)
{
	wxLogVerbose("Processing nodes of type %s", getTypeLabel());
	DirEntry* lump;
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		wxString lname(lump->name);
		if (lname.CmpNoCase("SSECTORS")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processNodes(file, lump->size);
		} else if (lname.CmpNoCase("ZNODES")==0) {
			file->SeekI(lump->offset, wxFromStart);
			processNodes(file, lump->size);
		}
	}
}

void ZDoomGLNodeStats::zSegs(wxInputStream* file, uint32_t seg)
{
	uint32_t segCount;
	file->Read(&segCount, 4);
	wxLogVerbose("Reading %i node SEGS", segCount);
	if (segCount != seg)
		wxLogVerbose("Mismatch between %i SEGS and %i total segments in SSECTORS", segCount, seg);
	segs = new vector<PairUint>();
	segs->reserve(segCount);
	for (int i=0; i<segCount; i++) {
		PairUint seg;
		file->Read(&seg.first, 4);
		//Not storing second vertex, as it is same as first of next seg
		segs->push_back(seg);
		file->SeekI(7, wxFromCurrent);
		//if ((seg.first>=totalVertices) || (seg.second>=totalVertices))
		//	wxLogVerbose("Segment vertex index out of bounds!");
	}
}

//Same as GLv2NodeStats::computeArea
double ZDoomGLNodeStats::computeArea(Vector2D minXY, Vector2D maxXY)
{
	wxLogVerbose("Calculate area");
	if (draw != NULL)
		draw->setup(minXY, maxXY);

	//Each subsector should be complete and closed, so we don't
	//need the splits of the node tree.
	int ss = ssectors->size();
	int verts = vertices->size();
	vector<Vector2D>* poly;
	for (int i=0; i<ss; i++) {
		PairUint& ssect = ssectors->at(i);
		poly = new vector<Vector2D>();
		for (int j=0; j<ssect.first; j++) {
			PairUint sg = segs->at(ssect.second + j);
			poly->push_back(Vector2D((double)vertices->at(sg.first).x,
					(double)vertices->at(sg.first).y));
		}
		area += polygonArea(poly);
		delete poly;
	}

	if (draw != NULL)
		draw->saveImage("F:\\Doom\\DMDB\\temp\\nodes.png");
	return area;
}

//***********************************************************************
//************************ ZDoomGLComprNodeStats ************************
//***********************************************************************

ZDoomGLComprNodeStats::ZDoomGLComprNodeStats(vector<Vertex>* vert, wxString typeLabel)
: ZDoomComprNodeStats(vert,NULL), typeStr(typeLabel)
{
}

wxString ZDoomGLComprNodeStats::getTypeLabel()
{
	return typeStr;
}

bool ZDoomGLComprNodeStats::isGL()
{
	return true;
}


//*******************************************************************
//************************ ZDoomGL2NodeStats ************************
//*******************************************************************

ZDoomGL2NodeStats::ZDoomGL2NodeStats(vector<Vertex>* vert)
: ZDoomGLNodeStats(vert)
{
}

wxString ZDoomGL2NodeStats::getTypeLabel()
{
	return "ZDoom GL (XGL2)";
}

void ZDoomGL2NodeStats::zSegs(wxInputStream* file, uint32_t seg)
{
	uint32_t segCount;
	file->Read(&segCount, 4);
	wxLogVerbose("Reading %i node SEGS", segCount);
	if (segCount != seg)
		wxLogVerbose("Mismatch between %i SEGS and %i total segments in SSECTORS", segCount, seg);
	segs = new vector<PairUint>();
	segs->reserve(segCount);
	for (int i=0; i<segCount; i++) {
		PairUint seg;
		file->Read(&seg.first, 4);
		//Not storing second vertex, as it is same as first of next seg
		segs->push_back(seg);
		file->SeekI(9, wxFromCurrent);
		//if ((seg.first>=totalVertices) || (seg.second>=totalVertices))
		//	wxLogVerbose("Segment vertex index out of bounds!");
	}
}

//*******************************************************************
//************************ ZDoomGL3NodeStats ************************
//*******************************************************************

ZDoomGL3NodeStats::ZDoomGL3NodeStats(vector<Vertex>* vert)
: ZDoomGL2NodeStats(vert)
{
}

wxString ZDoomGL3NodeStats::getTypeLabel()
{
	return "ZDoom GL (XGL3)";
}

void ZDoomGL3NodeStats::zNodes(wxInputStream* file)
{
	file->Read(&nodes, 4);
	wxLogVerbose("Reading %i NODES", nodes);
	nodeList = new vector<MapNode*>();
	nodeList->reserve(nodes);
	int16_t frac;
	for (int i=0; i<nodes; i++) {
		MapNode32* node = new MapNode32();
		file->Read(&frac, 2);
		file->Read(&(node->lineStart.x), 2);
		if (frac>=0x8000) node->lineStart.x++;
		//TODO: what if negative number?
		file->Read(&frac, 2);
		file->Read(&(node->lineStart.y), 2);
		if (frac>=0x8000) node->lineStart.y++;
		file->Read(&frac, 2);
		file->Read(&(node->lineDelta.x), 2);
		if (frac>=0x8000) node->lineDelta.x++;
		file->Read(&frac, 2);
		file->Read(&(node->lineDelta.y), 2);
		if (frac>=0x8000) node->lineDelta.y++;

		file->SeekI(16, wxFromCurrent);
		file->Read(&(node->rightIndex), 4);
		file->Read(&(node->leftIndex), 4);
		nodeList->push_back(node);
	}
}
