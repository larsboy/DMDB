#include "MapStats64.h"
#include "../LtbUtils.h"

MapStats64::MapStats64(string name, EngineType eng, TaskProgress* wp)
	: MapStats(name, eng, wp)
{
}

MapStats64::~MapStats64()
{
	//dtor
}

void MapStats64::processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs)
{
	int num = lsize / 14;
	wxLogVerbose("Processing THINGS - %i entries", num);
	uint16_t type;
	uint16_t flags;
	ThingDef* td;
	ThingDef* unknown = new ThingDef("");
	ThingDef* friendly = new ThingDef("");
	friendly->cat = THING_FRIENDLY;
	map<int, ThingDef*>::iterator it;
	for (int i = 0; i < num; i++) {
		file->SeekI(8, wxFromCurrent);
		file->Read(&type, 2);
		file->Read(&flags, 2);
		file->SeekI(2, wxFromCurrent);
		if (thingDefs == NULL) {
			td = unknown;
		} else {
			it = thingDefs->find(type);
			if (it == thingDefs->end()) {
				td = unknown;
				wxLogVerbose("Unknown THING %i", type);
			}
			else {
				td = it->second;
			}
		}
		thingCounts[td->cat]++;
		if (flags & 0x0010) {
			//Multiplayer-only things
			multiOnlyThings = true;
			//coopOnlyThings/dmOnlyThings? Original D64 didn't have MP
		} else {
			if (flags & 0x0001) //Easy
				thingStats[0]->addThing(td);
			if (flags & 0x0002) //Medium
				thingStats[1]->addThing(td);
			if (flags & 0x0004) //Hard
				thingStats[2]->addThing(td);
			if (flags & 0x0100)
				secrets++;
		}
	}
	delete unknown;
	delete friendly;
}

void MapStats64::processLinedefs(wxInputStream* file, int32_t lsize)
{
	int num = lsize / 16;
	wxLogVerbose("Processing LINEDEFS - %i entries", num);
	lines = new vector<MapLine>();
	lines->reserve(num);
	lineLength = 0.0;
	uint16_t vMin = 32000;
	uint16_t vMax = 0;
	uint16_t v1, v2;
	uint32_t flags;
	int16_t xd, yd;
	for (int i = 0; i < num; i++) {
		file->Read(&v1, 2);
		file->Read(&v2, 2);
		file->Read(&flags, 4);
		file->SeekI(8, wxFromCurrent);
		lines->push_back(MapLine(v1, v2, (flags & 0x4)));
		//flags & 0x20 secret?
		if (v1 < vMin) vMin = v1;
		if (v2 < vMin) vMin = v2;
		if (v1 > vMax) vMax = v1;
		if (v2 > vMax) vMax = v2;
		xd = vertices->at(v2).x - vertices->at(v1).x;
		yd = vertices->at(v2).y - vertices->at(v1).y;
		lineLength += sqrt(pow(yd, 2) + pow(xd, 2));
	}
	lineVertices = 1 + vMax - vMin;
}

void MapStats64::processSidedefs(wxInputStream* file, int32_t lsize)
{
	int num = lsize / 12;
	wxLogVerbose("Processing SIDEDEFS - %i entries", num);
	textures = new map<string, int>();
	
	//TODO: Textures are 16-bit table indices, not strings
	uint16_t texInd;
	for (int i = 0; i < num; i++) {
		file->SeekI(4, wxFromCurrent);
		for (int j = 0; j < 3; j++)
		{
			file->Read(&texInd, 2);
			string str = LtbUtils::intToString(texInd);
			(*textures)[str] = (*textures)[str] + 1;
		}
		file->SeekI(2, wxFromCurrent);
	}
}

void MapStats64::processVertexes(wxInputStream* file, int32_t lsize)
{
	int num = lsize / 8;
	wxLogVerbose("Processing VERTEXES - %i entries", num);
	vertices = new vector<Vertex>();
	vertices->reserve(num);
	minCorner.x = 32767;
	minCorner.y = 32767;
	maxCorner.x = -32768;
	maxCorner.y = -32768;
	int16_t x, y, frac;
	for (int i = 0; i < num; i++) {
		//Round off fractions to nearest int
		file->Read(&frac, 2);
		file->Read(&x, 2);
		if (frac >= 0x8000) x++;
		file->Read(&frac, 2);
		file->Read(&y, 2);
		if (frac >= 0x8000) y++;
		vertices->push_back(Vertex(x, y));
		if (x < minCorner.x) minCorner.x = x;
		if (y < minCorner.y) minCorner.y = y;
		if (x > maxCorner.x) maxCorner.x = x;
		if (y > maxCorner.y) maxCorner.y = y;
	}
}

void MapStats64::processSectors(wxInputStream* file, int32_t lsize)
{
	sectors = lsize / 24;
	wxLogVerbose("Processing SECTORS - %i entries", sectors);
	lightSum = 0.0;
	uint16_t effect;
	//TODO: Read colortable indices?
	for (int i = 0; i < sectors; i++) {
		file->SeekI(22, wxFromCurrent);
		//file->Read(&light, 2);
		//lightSum += light;
		file->Read(&effect, 2);
		if (effect == 0x20)
			secrets++;
	}
}