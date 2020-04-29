#include "MapStats.h"


//************************************************************
//************************ ThingStats ************************
//************************************************************

ThingStats::ThingStats()
: monsters(0), monsterHP(0), spawners(false), health(0), ammo(0), armor(0)
{
}

void ThingStats::addThing(ThingDef* thing)
{
	if (thing->cat == THING_WEAPON) {
		//Add name to vector
		bool found = false;
        for (int i=0; i<weapons.size(); i++) {
			if (weapons[i].compare(thing->name) == 0)
				found = true;
        }
        if (!found)
			weapons.push_back(thing->name);
	} else if (thing->cat == THING_MONSTER) {
		monsters++;
	}
	if (thing->spawner)
		spawners = true;
	if ((thing->cat!=THING_FRIENDLY) && (thing->hp<9999))
		monsterHP += thing->hp;
	health += thing->health;
	ammo += thing->ammo;
	armor += thing->armor;
}

float ThingStats::getHealthRate()
{
	if (monsterHP == 0)
		return 0.0;
	else
		return float( double(health) / double(monsterHP) );
}

float ThingStats::getAmmoRate()
{
	if (monsterHP == 0)
		return 0.0;
	else
		return float( double(ammo) / double(monsterHP) );
}

float ThingStats::getArmorRate()
{
	if (monsterHP == 0)
		return 0.0;
	else
		return float( double(armor) / double(monsterHP) );
}


bool texture_comp(const pair<string,int> &first, const pair<string,int> &second)
{
	return (first.second > second.second);
}


//**********************************************************
//************************ MapStats ************************
//**********************************************************

MapStats::MapStats(string name, EngineType eng, TaskProgress* tp)
: mapName(name), engine(eng), progress(tp),
multiOnlyThings(false), dmOnlyThings(false), coopOnlyThings(false),
vertices(NULL), lineVertices(0), minCorner(), maxCorner(), lines(NULL),
lineLength(0), sectors(0), secrets(0), lightSum(0.0), area(0.0), textures(NULL),
nodeStats(NULL), reject(false), blockmap(false)
{
	for (int i=0; i<THING_END; i++)
		thingCounts[i] = 0;
	for (int i=0; i<3; i++)
		thingStats[i] = new ThingStats();
}

MapStats::~MapStats()
{
	for (int i=0; i<3; i++) {
		if (thingStats[i] != NULL)
			delete thingStats[i];
	}
	if (nodeStats != NULL)
		delete nodeStats;
	if (vertices != NULL)
		delete vertices;
	if (lines != NULL)
		delete lines;
	if (textures != NULL)
		delete textures;
}

DirEntry* MapStats::findLump(const string& name, vector<DirEntry*>* lumps)
{
	for (int i=0; i<lumps->size(); i++) {
		if (lumps->at(i)->name.compare(name)==0)
			return lumps->at(i);
	}
	return NULL;
}

void MapStats::readFile(wxInputStream* file, vector<DirEntry*>* lumps, map<int, ThingDef*>* thingDefs)
{
	wxLogVerbose("Processing map %s with %i lumps", mapName, lumps->size());
	progress->startCount(MAP_PROGRESS_STEPS);
	//progress has MAP_PROGRESS_STEPS = 100
	DirEntry* lump;
	//Must process VERTEXES first, and check for GL nodes
	lump = findLump("VERTEXES", lumps);
	if (lump != NULL) {
		file->SeekI(lump->offset, wxFromStart);
		processVertexes(file, lump->size);
		progress->incrCount(10);
	}
	lump = findLump("SSECTORS", lumps);
	if (lump != NULL) {
		//Can contain ZDoom GL nodes
		file->SeekI(lump->offset, wxFromStart);
		findGLNodeType(file, lump->size); //Can create nodeStats
	}
	lump = findLump("GL_VERT", lumps);
	if (lump != NULL) {
		file->SeekI(lump->offset, wxFromStart);
		findGLNodeType(file, lump->size); //Creates nodeStats
	}

	//Rest of lumps
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		file->SeekI(lump->offset, wxFromStart);
		wxString lname(lump->name);
		if (lname.CmpNoCase("THINGS")==0) {
			processThings(file, lump->size, thingDefs);
			progress->incrCount(10);
		} else if (lname.CmpNoCase("LINEDEFS")==0) {
			processLinedefs(file, lump->size);
			progress->incrCount(10);
		} else if (lname.CmpNoCase("SIDEDEFS")==0) {
			processSidedefs(file, lump->size);
			progress->incrCount(10);
		} else if (lname.CmpNoCase("SECTORS")==0) {
			processSectors(file, lump->size);
			progress->incrCount(10);
		} else if ((lname.CmpNoCase("NODES")==0) && nodeStats==NULL) {
			findNodeType(file, lump->size); //Creates nodeStats
		} else if (lname.CmpNoCase("REJECT")==0) {
			processReject(file, lump->size);
		} else if (lname.CmpNoCase("BLOCKMAP")==0) {
			processBlockmap(file, lump->size);
		}
	}

	if (nodeStats != NULL) {
		nodeStats->progress = progress;
		nodeStats->readFile(file,lumps);
		Vector2D minXY((double)minCorner.x - 1.0, (double)minCorner.y - 1.0);
		Vector2D maxXY((double)maxCorner.x + 1.0, (double)maxCorner.y + 1.0);
		bool ok = nodeStats->checkNodes();
		if (ok) area = nodeStats->computeArea(minXY, maxXY);
	} else {
		wxLogVerbose("No nodes found, no area calculation");
	}
	progress->completeCount();
}

void MapStats::printReport(TextReport* reportView)
{
	reportView->writeHeading(mapName);
	reportView->writeSubHeading("Map geometry");
	wxArrayInt tabs;
	tabs.Add(450);tabs.Add(650);tabs.Add(850);tabs.Add(1050);
	reportView->setTabs(tabs);
	reportView->writeText("Vertices (for linedefs):\t");
	reportView->writeLine(wxString::Format("%i", lineVertices));
	reportView->writeText("X range:\t");
	reportView->writeLine(wxString::Format("%i to %i", minCorner.x, maxCorner.x));
	reportView->writeText("Y range:\t");
	reportView->writeLine(wxString::Format("%i to %i", minCorner.y, maxCorner.y));
	reportView->writeText("Linedefs:\t");
	reportView->writeLine(wxString::Format("%i", lines->size()));
	reportView->writeText("Total line length:\t");
	reportView->writeLine(wxString::Format("%.0f",lineLength));
	reportView->writeText("Avg. line length:\t");
	reportView->writeLine(wxString::Format("%.0f", avgLineLength()));
	reportView->writeText("Sectors:\t");
	reportView->writeLine(wxString::Format("%i", sectors));
	reportView->writeText("Avg. light level (0-255):\t");
	reportView->writeLine(wxString::Format("%.0f", avgLightLevel()));
	reportView->writeText("Secrets:\t");
	reportView->writeLine(wxString::Format("%i", secrets));

	reportView->writeSubHeading("Compiled structures");
	if (nodeStats == NULL)
		reportView->writeLine("Node type:\tNone");
	else
		nodeStats->printReport(reportView);
	reportView->writeText("Non-null Reject entries:\t");
	reportView->writeLine((reject)? "Yes": "No");
	reportView->writeText("Blockmap lump:\t");
	reportView->writeLine((blockmap)? "Yes": "No");

	reportView->writeSubHeading("Thing counts");
	for (int i=1; i<THING_END; i++) {
		reportView->writeText(thingCatStr[i]+":\t");
		reportView->writeLine(wxString::Format("%i", thingCounts[i]));
	}
	reportView->writeText(thingCatStr[0]+":\t");
	reportView->writeLine(wxString::Format("%i", thingCounts[THING_UNKNOWN]));
	reportView->writeText("Total:\t"); //writeBold()?
	reportView->writeLine(wxString::Format("%i", getTotalThings()));
	reportView->writeText("Skill levels used: ");
	reportView->writeLine((hasDifficultySetting())? "Yes": "No");
	reportView->writeText("Multiplayer-exlusive things: ");
	reportView->writeLine((multiOnlyThings)? "Yes": "No");
	reportView->writeText("Deathmatch-exlusive things: ");
	reportView->writeLine((dmOnlyThings)? "Yes": "No");
	reportView->writeText("Coop-exlusive things: ");
	reportView->writeLine((coopOnlyThings)? "Yes": "No");

	reportView->writeSubHeading("Gameplay stats");
	reportView->writeLine("\tSkill 1\tSkill 2\tSkill 3");
	reportView->writeLine("\t(HNTR)\t(HMP)\t(UV)");
	reportView->writeText("Enemies:");
	for (int i=0; i<3; i++)
		reportView->writeText("\t" + wxString::Format("%i", thingStats[i]->monsters));
	reportView->writeLine("");
	reportView->writeText("Total hitpoints:");
	for (int i=0; i<3; i++)
		reportView->writeText("\t" + wxString::Format("%i", thingStats[i]->monsterHP));
	reportView->writeLine("");
	reportView->writeText("Enemy spawning:");
	for (int i=0; i<3; i++)
		reportView->writeText((thingStats[i]->spawners)? "\tYes": "\tNo");
	reportView->writeLine("");
	reportView->writeText("Ammo ratio:");
	for (int i=0; i<3; i++)
		reportView->writeText("\t" + wxString::Format("%.2f", thingStats[i]->getAmmoRate()));
	reportView->writeLine("");
	reportView->writeText("Health ratio:");
	for (int i=0; i<3; i++)
		reportView->writeText("\t" + wxString::Format("%.2f", thingStats[i]->getHealthRate()));
	reportView->writeLine("");
	reportView->writeText("Armor ratio:");
	for (int i=0; i<3; i++)
		reportView->writeText("\t" + wxString::Format("%.2f", thingStats[i]->getArmorRate()));
	reportView->writeLine("");

	reportView->writeLine("");
	reportView->writeText("Weapons Skill 1:\t");
	for (int i=0; i<thingStats[0]->weapons.size(); i++) {
		reportView->writeText(thingStats[0]->weapons.at(i));
		reportView->writeText("  ");
	}
	reportView->writeLine("");
	reportView->writeText("Weapons Skill 2:\t");
	for (int i=0; i<thingStats[1]->weapons.size(); i++) {
		reportView->writeText(thingStats[1]->weapons.at(i));
		reportView->writeText("  ");
	}
	reportView->writeLine("");
	reportView->writeText("Weapons Skill 3:\t");
	for (int i=0; i<thingStats[2]->weapons.size(); i++) {
		reportView->writeText(thingStats[2]->weapons.at(i));
		reportView->writeText("  ");
	}
	reportView->writeLine("");

	reportView->writeSubHeading("Textures");
	reportView->writeText("Different textures used: ");
	reportView->writeLine(wxString::Format("%i", textures->size()));
	reportView->writeLine("Textures in order of use:");
	list< pair<string,int> >* texCount = getTexturesCount();
	for (list< pair<string,int> >::iterator it=texCount->begin(); it!=texCount->end(); ++it) {
		pair<string,int> pr = *it;
		reportView->writeText(pr.first + "\t");
		reportView->writeLine(wxString::Format("%i", pr.second));
	}
	delete texCount;
}

uint16_t MapStats::getTotalThings()
{
	uint16_t result = 0;
	for (int i=0; i<THING_END; i++)
		result += thingCounts[i];
	return result;
}

bool MapStats::hasDifficultySetting()
{
	uint32_t stuff = thingStats[0]->monsterHP;
	if (stuff!=thingStats[1]->monsterHP || stuff!=thingStats[2]->monsterHP)
		return true;
	stuff = thingStats[0]->health;
	if (stuff!=thingStats[1]->health || stuff!=thingStats[2]->health)
		return true;
	stuff = thingStats[0]->ammo;
	if (stuff!=thingStats[1]->ammo || stuff!=thingStats[2]->ammo)
		return true;
	stuff = thingStats[0]->armor;
	if (stuff!=thingStats[1]->armor || stuff!=thingStats[2]->armor)
		return true;
	return false;
}

list< pair<string,int> >* MapStats::getTexturesCount()
{
	if (textures == NULL) return NULL;
	list< pair<string,int> >* texList = new list< pair<string,int> >();
	for (map<string,int>::iterator it=textures->begin(); it!=textures->end(); ++it) {
		texList->push_back(*it);
	}
	bool (*textComp)(const pair<string,int> &first, const pair<string,int> &second) =
		texture_comp;
	texList->sort(textComp);
	return texList;
}

bool MapStats::hasGLNodes()
{
	return (nodeStats!=NULL && nodeStats->isGL());
}

void MapStats::processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs)
{
	int num = lsize/10;
	wxLogVerbose("Processing THINGS - %i entries", num);
	uint16_t type;
	uint16_t flags;
	ThingDef* td;
	ThingDef* unknown = new ThingDef("");
	ThingDef* friendly = new ThingDef("");
	friendly->cat = THING_FRIENDLY;
	map<int,ThingDef*>::iterator it;
	for (int i=0; i<num; i++) {
		file->SeekI(6, wxFromCurrent);
		file->Read(&type, 2);
		file->Read(&flags, 2);
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
		if ((flags&0x0080) && (td->cat==THING_MONSTER) && (engine>=DENG_BOOM)) {
			//MBF flag 0x0080	Friendly monster
			td = friendly;
		}
		thingCounts[td->cat]++;
		if (flags&0x0010) {//Multiplayer only (not in single-player)
			if (flags&0x0020) //Boom flag: Thing is not in deathmatch
				coopOnlyThings = true;
			else if (flags&0x0040) //Boom flag: Thing is not in coop
				dmOnlyThings = true;
			else
				multiOnlyThings = true;
		} else {
			if (flags&0x0001) //Easy
				thingStats[0]->addThing(td);
			if (flags&0x0002) //Medium
				thingStats[1]->addThing(td);
			if (flags&0x0004) //Hard
				thingStats[2]->addThing(td);
		}
	}
	delete unknown;
	delete friendly;
}

void MapStats::processVertexes(wxInputStream* file, int32_t lsize)
{
	int num = lsize/4;
	wxLogVerbose("Processing VERTEXES - %i entries", num);
	vertices = new vector<Vertex>();
	vertices->reserve(num);
	minCorner.x = 32767;
	minCorner.y = 32767;
	maxCorner.x = -32768;
	maxCorner.y = -32768;
	int16_t x, y;
	for (int i=0; i<num; i++) {
		file->Read(&x, 2);
		file->Read(&y, 2);
		vertices->push_back(Vertex(x,y));
		if (x<minCorner.x) minCorner.x=x;
		if (y<minCorner.y) minCorner.y=y;
		if (x>maxCorner.x) maxCorner.x=x;
		if (y>maxCorner.y) maxCorner.y=y;
	}
}

void MapStats::processLinedefs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/14;
	wxLogVerbose("Processing LINEDEFS - %i entries", num);
	lines = new vector<MapLine>();
	lines->reserve(num);
	lineLength = 0.0;
	uint16_t vMin = 30000;
	uint16_t vMax = 0;
	uint16_t v1, v2;
	uint16_t flags;
	int16_t xd, yd;
	//Boom linedef flag: 0x0200	pass-thru
	for (int i=0; i<num; i++) {
		file->Read(&v1, 2);
		file->Read(&v2, 2);
		file->Read(&flags, 2);
		file->SeekI(8, wxFromCurrent);
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

void MapStats::processSidedefs(wxInputStream* file, int32_t lsize)
{
	int num = lsize/30;
	wxLogVerbose("Processing SIDEDEFS - %i entries", num);
	textures = new map<string, int>();
	char name[] = {0,0,0,0,0,0,0,0,0};

	for (int i=0; i<num; i++) {
		file->SeekI(4, wxFromCurrent);
		for (int j=0; j<3; j++)
		{
			file->Read(name, 8);
			if (name[0] == '-')
				continue;
			string str(name);
			(*textures)[str] = (*textures)[str]+1;
		}
		file->SeekI(2, wxFromCurrent);
	}
}

void MapStats::processSectors(wxInputStream* file, int32_t lsize)
{
	sectors = lsize/26;
	wxLogVerbose("Processing SECTORS - %i entries", sectors);
	secrets = 0;
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
		else if (effect&128) //Generalized Boom flags
			secrets++;
		file->SeekI(2, wxFromCurrent);
	}
}

void MapStats::processReject(wxInputStream* file, int32_t lsize)
{
	wxLogVerbose("Processing REJECT - %i bytes", lsize);
	reject = false;
	char b;
	for (int i=0; i<lsize; i++) {
		file->Read(&b, 1);
		if (b != 0) {
			reject = true;
			break;
		}
		if (i > 20000)
			break; //Give up
	}
}

void MapStats::processBlockmap(wxInputStream* file, int32_t lsize)
{
	wxLogVerbose("Processing BLOCKMAP - %i bytes", lsize);
	blockmap = (lsize > 0);
}

void MapStats::findNodeType(wxInputStream* file, int32_t lsize)
{
	if (lsize < 8) {
		nodeStats = NULL;
		return;
	}
	char ch[8];
	file->Read(ch, 8);
	if (ch[0]=='x' && ch[1]=='N' && ch[2]=='d' && ch[3]=='4' && ch[4]==0 && ch[5]==0 && ch[6]==0 && ch[7]==0)
		nodeStats = new DeepNodeStats(vertices, lines);
	else if (ch[0]=='X' && ch[1]=='N' && ch[2]=='O' && ch[3]=='D')
		nodeStats = new ZDoomNodeStats(vertices, lines);
	else if (ch[0]=='Z' && ch[1]=='N' && ch[2]=='O' && ch[3]=='D')
		nodeStats = new ZDoomComprNodeStats(vertices, lines);
	else //Basic
		nodeStats = new NodeStats(vertices, lines);
}

void MapStats::findGLNodeType(wxInputStream* file, int32_t lsize)
{
	if (lsize < 4) return;
	char ch[4];
	file->Read(ch, 4);
	if (ch[0]=='g' && ch[1]=='N' && ch[2]=='d' && ch[3]=='2')
		nodeStats = new GLv2NodeStats(vertices);
	else if (ch[0]=='g' && ch[1]=='N' && ch[2]=='d' && ch[3]=='5')
		nodeStats = new GLv5NodeStats(vertices);
	else if (ch[0]=='X' && ch[1]=='G' && ch[2]=='L' && ch[3]=='N')
		nodeStats = new ZDoomGLNodeStats(vertices);
	else if (ch[0]=='Z' && ch[1]=='G' && ch[2]=='L' && ch[3]=='N')
		nodeStats = new ZDoomGLComprNodeStats(vertices, "ZDoom GL compressed (ZGLN)");
	else if (ch[0]=='X' && ch[1]=='G' && ch[2]=='L' && ch[3]=='2')
		nodeStats = new ZDoomGL2NodeStats(vertices);
	else if (ch[0]=='Z' && ch[1]=='G' && ch[2]=='L' && ch[3]=='2')
		nodeStats = new ZDoomGLComprNodeStats(vertices, "ZDoom GL compressed (ZGL2)");
	else if (ch[0]=='X' && ch[1]=='G' && ch[2]=='L' && ch[3]=='3')
		nodeStats = new ZDoomGL3NodeStats(vertices);
	else if (ch[0]=='Z' && ch[1]=='G' && ch[2]=='L' && ch[3]=='3')
		nodeStats = new ZDoomGLComprNodeStats(vertices, "ZDoom GL compressed (ZGL3)");
}
