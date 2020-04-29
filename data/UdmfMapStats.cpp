#include "UdmfMapStats.h"

UdmfMapStats::UdmfMapStats(string name, EngineType eng, TaskProgress* wp)
: MapStats(name, eng, wp),
current(0), currentThing(NULL), flags(0)
{
	unknownThing = new ThingDef("");
	friendlyThing = new ThingDef("");
	friendlyThing->cat = THING_FRIENDLY;
}

UdmfMapStats::~UdmfMapStats()
{
	delete unknownThing;
	delete friendlyThing;
}

void UdmfMapStats::readFile(wxInputStream* file, vector<DirEntry*>* lumps, map<int, ThingDef*>* things)
{
	thingDefs = things;
	wxLogVerbose("Processing map %s with %i lumps", mapName, lumps->size());
	progress->startCount(MAP_PROGRESS_STEPS);
	DirEntry* lump;
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		file->SeekI(lump->offset, wxFromStart);
		wxString lname(lump->name);
		if (lname.CmpNoCase("TEXTMAP")==0)
			processTextmap(file, lump->size);
		else if (lname.CmpNoCase("ZNODES")==0)
			findGLNodeType(file, lump->size); //Creates nodeStats
		else if (lname.CmpNoCase("REJECT")==0)
			processReject(file, lump->size);
	}

	if (nodeStats != NULL) {
		nodeStats->progress = progress;
		nodeStats->readFile(file, lumps);
		Vector2D minXY((double)minCorner.x - 1.0, (double)minCorner.y - 1.0);
		Vector2D maxXY((double)maxCorner.x + 1.0, (double)maxCorner.y + 1.0);
		bool ok = nodeStats->checkNodes();
		if (ok) area = nodeStats->computeArea(minXY, maxXY);
	} else {
		wxLogVerbose("No nodes found, no area calculation");
	}
	progress->completeCount();
}

void UdmfMapStats::processTextmap(wxInputStream* file, int32_t lsize)
{
	vertices = new vector<Vertex>();
	minCorner.x = 32767;
	minCorner.y = 32767;
	maxCorner.x = -32768;
	maxCorner.y = -32768;
	lines = new vector<MapLine>();
	textures = new map<string, int>();


	char ch = 0;
	//'\r' is carriage return, and '\n' is line feed.
	//Newline is normally \r + \n
	int32_t bytesLeft = lsize;
	bool lineDone;
	//Split progress into 60 units
	int prog = lsize/60;
	wxString line("");
	line.Alloc(100);
	while (bytesLeft!=0) {
		lineDone = false;
		while (!lineDone) {
			ch = file->GetC();
			if (file->Eof()) {
				bytesLeft=0;
			} else {
				bytesLeft--;
				prog--;
				if (ch == '\n') {
					lineDone=true;
				} else if (ch == '\r') {
					ch = file->Peek();
					if (ch == '\n') {
						file->GetC();
						bytesLeft--;
						prog--;
					}
					lineDone=true;
				} else {
					line << ch;
				}
			}
			if (bytesLeft==0) lineDone=true;
		}
		processLine(line);
		line.Empty();
		//wxLogVerbose("Offset: %i", file->Tell());
		if (prog < 1) {
			progress->incrCount();
			prog = lsize/60;
			if (progress->getCurrentCount()%10 == 0)
				wxLogVerbose("TEXTMAP: %i bytes left", bytesLeft);
		}
	}
	lineVertices = vertices->size();
}

void UdmfMapStats::processLine(wxString& line)
{
	line.Trim(false);
	//line.MakeLower();
	if (current == 1) { //thing
		//type always before flags?
		if (line.StartsWith("type")) {
			if (thingDefs != NULL) {
				int type = processInteger(line);
				map<int,ThingDef*>::iterator it = thingDefs->find(type);
				if (it == thingDefs->end()) {
					currentThing = unknownThing;
					wxLogVerbose("Unknown THING %i", type);
				} else {
					currentThing = it->second;
				}
			}
		} else if (line.StartsWith("skill2")) {
			if (processFlag(line))
				flags |= 0x0001;
		} else if (line.StartsWith("skill3")) {
			if (processFlag(line))
				flags |= 0x0002;
		} else if (line.StartsWith("skill4")) {
			if (processFlag(line))
				flags |= 0x0004;
		} else if (line.StartsWith("single")) {
			if (processFlag(line))
				flags |= 0x0100;
		} else if (line.StartsWith("dm")) {
			if (processFlag(line))
				flags |= 0x0400;
		} else if (line.StartsWith("coop")) {
			if (processFlag(line))
				flags |= 0x0200;
		} else if (line.StartsWith("friend")) {
			if (processFlag(line) && (currentThing->cat==THING_MONSTER))
				currentThing = friendlyThing;
		} else if (line.StartsWith("countsecret")) {
			if (processFlag(line))
				secrets++;
		} else if (line.Contains("}")) {
			//End of object, registered here
			thingCounts[currentThing->cat]++;
			if (flags&0x0100) { //Thing appears in single-player games
				//wxLogVerbose("Single-player thing");
				if (flags&0x0001) //Easy
					thingStats[0]->addThing(currentThing);
				if (flags&0x0002) //Medium
					thingStats[1]->addThing(currentThing);
				if (flags&0x0004) //Hard
					thingStats[2]->addThing(currentThing);
			} else {
				//wxLogVerbose("Multiplayer-only thing");
				//Multiplayer-only things
				if ((flags&0x0200) && (flags&0x0400))
					multiOnlyThings = true;
				else if (flags&0x0200) //Thing appears in cooperative games
					coopOnlyThings = true;
				else if (flags&0x0400) //Thing appears in deathmatch games
					dmOnlyThings = true;
			}
			currentThing = NULL;
			flags = 0;
			current = 0;
		}
		return;

	} else if (current == 2) { //vertex
		if (line.StartsWith("x")) {
			vertX = processFloat(line);
			if (vertX<minCorner.x) minCorner.x=vertX;
			if (vertX>maxCorner.x) maxCorner.x=vertX;
		} else if (line.StartsWith("y")) {
			vertY = processFloat(line);
			if (vertY<minCorner.y) minCorner.y=vertY;
			if (vertY>maxCorner.y) maxCorner.y=vertY;
		} else if (line.Contains("}")) {
			vertices->push_back(Vertex(vertX,vertY));
			current = 0;
		}
		return;

	} else if (current == 3) { //linedef
		//Must come after all vertices
		if (line.StartsWith("v1")) {
			v1 = processInteger(line);
		} else if (line.StartsWith("v2")) {
			v2 = processInteger(line);
		} else if (line.StartsWith("sideback")) {
			twoSided = true;
		} else if (line.Contains("}")) {
			lines->push_back(MapLine(v1,v2,twoSided));
			int16_t xd = vertices->at(v2).x - vertices->at(v1).x;
			int16_t yd = vertices->at(v2).y - vertices->at(v1).y;
			lineLength += sqrt(pow(yd, 2) + pow(xd, 2));
			current = 0;
		}
		return;

	} else if (current == 4) { //sidedef
		if (line.StartsWith("texturetop")) {
			string str = processQuoted(line);
			if (str.length() >0)
				(*textures)[str] = (*textures)[str]+1;
		} else if (line.StartsWith("texturebottom")) {
			string str = processQuoted(line);
			if (str.length() >0)
				(*textures)[str] = (*textures)[str]+1;
		} else if (line.StartsWith("texturemiddle")) {
			string str = processQuoted(line);
			if (str.length() >0)
				(*textures)[str] = (*textures)[str]+1;
		} else if (line.Contains("}")) {
			current = 0;
		}
		return;

	} else if (current == 5) { //sector
		if (line.StartsWith("lightlevel")) {
			long light = processInteger(line);
			lightSum += light;
		} else if (line.StartsWith("special")) {
			uint16_t flags = processInteger(line);
			if (flags&1024)
				secrets++;
		} else if (line.Contains("}")) {
			current = 0;
		}
		return;
	}

	if (line.StartsWith("thing")) {
		current = 1;
		currentThing = unknownThing;
	} else if (line.StartsWith("vertex")) {
		current = 2;
	} else if (line.StartsWith("linedef")) {
		current = 3;
		twoSided = false;
	} else if (line.StartsWith("sidedef")) {
		current = 4;
	} else if (line.StartsWith("sector")) {
		sectors++;
		current = 5;
	}
}

bool UdmfMapStats::processFlag(wxString& line)
{
	int i1 = line.find('=', 0);
	int i2 = line.find(';', i1+1);
	if (i1==string::npos || i2==string::npos)
		return false;
	wxString str = line.SubString(i1+1, i2-1);
	str.Trim();
	str.Trim(false);
	return (str.CmpNoCase("true")==0);
}

long UdmfMapStats::processInteger(wxString& line)
{
	int i1 = line.find('=', 0);
	int i2 = line.find(';', i1+1);
	if (i1==string::npos || i2==string::npos)
		return 0;
	wxString str = line.SubString(i1+1, i2-1);
	str.Trim();
	str.Trim(false);
	long result;
	if (str.ToCLong(&result))
		return result;
	else
		return 0;
}

int UdmfMapStats::processFloat(wxString& line)
{
	int i1 = line.find('=', 0);
	int i2 = line.find(';', i1+1);
	if (i1==string::npos || i2==string::npos)
		return 0;
	wxString str = line.SubString(i1+1, i2-1);
	str.Trim();
	str.Trim(false);
	double result;
	if (str.ToCDouble(&result)) {
		return (int)round(result);
	}
	return 0;
}

string UdmfMapStats::processQuoted(wxString& line)
{
	int i1 = line.find('\"', 0);
	int i2 = line.find('\"', i1+1);
	if (i1==string::npos || i2==string::npos)
		return "";
	return line.SubString(i1+1, i2-1).ToStdString();
}
