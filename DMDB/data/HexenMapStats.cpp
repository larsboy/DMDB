#include "HexenMapStats.h"

HexenMapStats::HexenMapStats(string name, EngineType eng, TaskProgress* wp)
: MapStats(name, eng, wp)
{
}

HexenMapStats::~HexenMapStats()
{
	//dtor
}

void HexenMapStats::processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs)
{
	int num = lsize/20;
	wxLogVerbose("Processing THINGS - %i entries", num);
	uint16_t type;
	uint16_t flags;
	ThingDef* td;
	ThingDef* unknown = new ThingDef("");
	ThingDef* friendly = new ThingDef("");
	friendly->cat = THING_FRIENDLY;
	map<int,ThingDef*>::iterator it;
	for (int i=0; i<num; i++) {
		file->SeekI(10, wxFromCurrent);
		file->Read(&type, 2);
		file->Read(&flags, 2);
		file->SeekI(6, wxFromCurrent);
		if (thingDefs == NULL) {
			td = unknown;
		} else {
			it = thingDefs->find(type);
			if (it == thingDefs->end()) {
				td = unknown;
				wxLogVerbose("Unknown THING %i", type);
			} else {
				td = it->second;
			}
		}
		if ((flags&0x2000) && (td->cat==THING_MONSTER) && (engine>=DENG_ZDOOM)) {
			//Friendly monster
			td = friendly;
		}
		thingCounts[td->cat]++;
		if (flags&0x0100) { //Thing appears in single-player games
			if (flags&0x0001) //Easy
				thingStats[0]->addThing(td);
			if (flags&0x0002) //Medium
				thingStats[1]->addThing(td);
			if (flags&0x0004) //Hard
				thingStats[2]->addThing(td);
		} else {
			//Multiplayer-only things
			if ((flags&0x0200) && (flags&0x0400))
				multiOnlyThings = true;
			else if (flags&0x0200) //Thing appears in cooperative games
				coopOnlyThings = true;
			else if (flags&0x0400) //Thing appears in deathmatch games
				dmOnlyThings = true;
		}
	}
	delete unknown;
	delete friendly;
}

void HexenMapStats::processLinedefs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/16;
	wxLogVerbose("Processing LINEDEFS - %i entries", num);
	lines = new vector<MapLine>();
	lines->reserve(num);
	lineLength = 0.0;
	uint16_t vMin = 32000;
	uint16_t vMax = 0;
	uint16_t v1, v2;
	uint16_t flags;
	int16_t xd, yd;
	for (int i=0; i<num; i++) {
		file->Read(&v1, 2);
		file->Read(&v2, 2);
		file->Read(&flags, 2);
		file->SeekI(10, wxFromCurrent);
		lines->push_back(MapLine(v1,v2,(flags&0x0004)));
		if (v1<vMin) vMin = v1;
		if (v2<vMin) vMin = v2;
		if (v1>vMax) vMax = v1;
		if (v2>vMax) vMax = v2;
		xd = vertices->at(v2).x - vertices->at(v1).x;
		yd = vertices->at(v2).y - vertices->at(v1).y;
		lineLength += sqrt(pow(yd, 2) + pow(xd, 2));
	}
	lineVertices = 1 + vMax - vMin;
}

void HexenMapStats::processSectors(wxInputStream* file, int32_t lsize)
{
	sectors = lsize/26;
	wxLogVerbose("Processing SECTORS - %i entries", sectors);
	lightSum = 0.0;
	int16_t light;
	uint16_t effect;
	for (int i=0; i<sectors; i++) {
		file->SeekI(20, wxFromCurrent);
		file->Read(&light, 2);
		lightSum += light;
		file->Read(&effect, 2);
		if (effect == 0x09)
			secrets++;
		else if (effect&1024)
			secrets++;
		file->SeekI(2, wxFromCurrent);
	}
}
