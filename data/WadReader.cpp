/*
* WadReader implementation
*/

#include "WadReader.h"

WadStatAspects::WadStatAspects()
: wadFile(true), wadFlags(true),
mapMain(true), gameModes(true), mapStats(true), gameStats(true), mapImages(true)
{}

bool WadStatAspects::mapAspects()
{
	return (mapMain||gameModes||mapStats||gameStats||mapImages);
}


WadReader::WadReader()
: thingType(0), thingDefs(NULL), mainFile(), iwad(IWAD_NONE), engine(DENG_NONE),
archive(NULL), aspects(NULL), wadStatList(NULL), dehacked(NULL), decorate(NULL),
mapinfo(NULL)
{
	fileSystem = new wxFileSystem();
}

WadReader::~WadReader()
{
	delete fileSystem;
	if (thingDefs != NULL)
		delete thingDefs;
	clearState();
	if (aspects != NULL)
		delete aspects;
}

void WadReader::setThingFile(int type, wxString file)
{
	thingFiles[type] = file;
	if (thingDefs!=NULL && thingType==type) {
		delete thingDefs;
		thingDefs = NULL;
		thingType = 0;
	}
}

void WadReader::setAspects(WadStatAspects* asp)
{
	if (aspects != NULL)
		delete aspects;
	aspects = asp;
}

void WadReader::setTempFolder(wxString folder)
{
	tempFolder = folder;
}

void WadReader::clearState()
{
	iwad = IWAD_NONE;
	engine = DENG_NONE;
	if (archive != NULL) {
		archive->deleteExtracted();
		delete archive;
		archive = NULL;
	}
	if (wadStatList != NULL) {
		for (int i=0; i<wadStatList->size(); i++) {
			wadStatList->at(i)->cleanup();
			delete wadStatList->at(i);
		}
		delete wadStatList;
		wadStatList = NULL;
	}
	if (dehacked != NULL) {
		delete dehacked;
		dehacked = NULL;
	}
	if (decorate != NULL) {
		delete decorate;
		decorate = NULL;
	}
	if (mapinfo != NULL) {
		delete mapinfo;
		mapinfo = NULL;
	}
	if (thingDefs!=NULL && thingDefs->hasModifiedDefs()) {
		//ThingDefs are modified, need to reload from file
		delete thingDefs;
		thingDefs = NULL;
		thingType = 0;
	}
	//Delete dirs in tempFolder
	wxDir dir(tempFolder);
	wxString subdir;
	bool cont = dir.GetFirst(&subdir, wxEmptyString, wxDIR_DIRS);
	while (cont) {
		//wxLogVerbose("Delete folder %s", subdir);
		wxFileName::Rmdir(tempFolder+wxFILE_SEP_PATH+subdir, wxPATH_RMDIR_FULL);
		cont = dir.GetNext(&subdir);
	}
}

void WadReader::initReader(wxString file)
{
	clearState();
	mainFile = wxFileName(file);
	wadStatList = new vector<WadStats*>();
	if (mainFile.GetExt().CmpNoCase("zip") == 0) {
		archive = new WadArchive(mainFile.GetFullPath(), tempFolder);
		archive->readArchiveFiles();
	}
}

void WadReader::processWads(TaskProgress* tp)
{
	WadStats* ws;
	if (archive == NULL) {
		ws = processWad(mainFile.GetFullPath(), !mainFile.GetExt().IsSameAs("wad", false), tp);
		if (ws != NULL) wadStatList->push_back(ws);
		if (tp->hasFailed()) return;
	} else {
		wxString mainName = mainFile.GetName();
		int maxPri = 0;
		int maxPriIndex = 0;
		for (int i=0; i<archive->numberOfWads(); i++) {
			try {
				wxString fileName = archive->extractWad(i);
				if (fileName.Length() > 0) {
					wxFileName fn = wxFileName(fileName);
					wxLogVerbose("Processing file of type %s", fn.GetExt());
					TaskProgress* sub = new TaskProgress(wxString::Format("Analysing contents of %s",fn.GetFullName()), tp);
					ws = processWad(fileName, !fn.GetExt().IsSameAs("wad", false), sub);
					if (ws != NULL) wadStatList->push_back(ws);
					bool failed = sub->hasFailed(true);
					delete sub;
					if (failed) return;
					ws->year = archive->getWadYear();
					if (mainName.IsSameAs(fn.GetName(), false))
						ws->priority++;
					if (ws->priority > maxPri) {
						maxPri = ws->priority;
						maxPriIndex = i;
					}
				}
			} catch (GuiError e) {
				//archive->extractWad failed
				tp->fatalError("Failed to extract file from archive");
				return;
			}
		}
		//Main wad must be first in vector
		if (maxPriIndex > 0) {
			ws = wadStatList->at(0);
			(*wadStatList)[0] = (*wadStatList)[maxPriIndex];
			(*wadStatList)[maxPriIndex] = ws;
		}
	}
	//Find iwad and engine
	for (int i=0; i<wadStatList->size(); i++) {
		ws = wadStatList->at(i);
		if (ws->iwad > iwad) iwad=ws->iwad;
		if (ws->engine > engine) engine=ws->engine;
	}
}

void WadReader::findThingDefs(TaskProgress* tp)
{
	int type = 0;
	//TODO: Engine-specifc (ZDoom) thingDefs, combined with iwad-specific
	if (iwad>=IWAD_DOOM && iwad<=IWAD_PLUT) {
		if (engine==DENG_SKULLTAG)
			type = 3;
		else if (engine==DENG_ZDOOM || engine==DENG_GZDOOM)
			type = 2;
		else
			type = 1;
	} else if (iwad == IWAD_HERETIC) {
		type = 4;
	} else if (iwad == IWAD_SQUARE) {
		type = 2;
	}
	if (thingType != type) {
		if (thingDefs != NULL)
			delete thingDefs;
		thingDefs = new ThingDefList();
		if (type > 0) {
			try { thingDefs->loadDefs(thingFiles[type]); }
			catch (GuiError e) { tp->fatalError("Error loading ThingDef file"); }
		}
		thingType=type;
	} else if (thingDefs == NULL) {
		thingDefs = new ThingDefList();
	}
	if (tp->hasFailed())
		return;

	//Check for DECORATE in wad
	for (int i=0; i<wadStatList->size(); i++) {
		processDecorate(wadStatList->at(i), tp);
	}
	if (decorate != NULL)
		thingDefs = decorate->extractThingDefs();

	//Check for Dehacked in wad
	TaskProgress* sub = new TaskProgress("", tp);
	for (int i=0; i<wadStatList->size(); i++) {
		dehacked = processDeh(wadStatList->at(i), sub);
		if (dehacked != NULL) {
			delete sub;
			return;
		}
	}

	//Check for Dehacked file
	if (dehacked==NULL && archive!=NULL) {
		wxString dehFile = archive->extractDehacked();
		if (dehFile.Length()>0)
			dehacked = processDeh(dehFile, sub);
	}
	delete sub;
}

void WadReader::printReport(TextReport* reportView)
{
	if (archive != NULL)
		archive->printReport(reportView);
	for (int i=0; i<wadStatList->size(); i++)
		wadStatList->at(i)->printReport(reportView);
	if (dehacked != NULL)
		dehacked->printReport(reportView);
	if (decorate != NULL)
		decorate->printReport(reportView);
	if (mapinfo != NULL)
		mapinfo->printReport(reportView);
	//TODO: text files
}

void WadReader::addWad(wxString file, bool pk3, TextReport* reportView, TaskProgress* tp)
{
	WadStats* ws = processWad(file, pk3, tp);
	if (tp->hasFailed() && (ws != NULL)) {
		delete ws;
		ws = NULL;
	}
	if (ws != NULL) {
		wadStatList->push_back(ws);
		if (reportView != NULL) {
			ws->printReport(reportView);
		}
		if (decorate == NULL) {
			//So far don't support adding decorate here
			//Use zip with all wads if have multiple wads with decorate
			if (processDecorate(ws, tp)) {
				thingDefs = decorate->extractThingDefs();
				decorate->printReport(reportView);
				//TODO: Replace existing report
			}
		}
		if (dehacked == NULL) {
			TaskProgress* sub = new TaskProgress("", tp);
			dehacked = processDeh(ws, sub);
			delete sub; //Any error is transferred to tp
			if (dehacked != NULL)
				dehacked->printReport(reportView);
		}
	}
}

void WadReader::addDeh(wxString file, TextReport* reportView, TaskProgress* tp)
{
	if (dehacked != NULL) {
		tp->fatalError("Only one Dehacked patch supported per wad entry.");
	} else {
		dehacked = processDeh(file, tp);
	}
	if (!tp->hasFailed())
		dehacked->printReport(reportView);
}

vector<wxString>* WadReader::getWadNames()
{
	vector<wxString>* result = new vector<wxString>();
	for (int i=0; i<wadStatList->size(); i++) {
		result->push_back(wadStatList->at(i)->fileName);
	}
	return result;
}

void WadReader::removeWad(int index)
{
	wadStatList->at(index)->cleanup();
	wadStatList->erase(wadStatList->begin()+index);
}

int WadReader::numberOfMaps()
{
	int maps = 0;
	for (int i=0; i<wadStatList->size(); i++) {
		WadStats* wadStats = wadStatList->at(i);
		WadContentX* wcx = wadStats->getMapContent();
		while (wcx != NULL) {
			maps++;
			wcx = wcx->next;
		}
	}
	return maps;
}

vector<wxString>* WadReader::getMapNames()
{
	//Map name is name of first lump
	vector<wxString>* result = new vector<wxString>();
	WadStats* wadStats = NULL;
	WadContentX* wcx = NULL;
	int mapWads = 0;
	for (int i=0; i<wadStatList->size(); i++) {
		wadStats = wadStatList->at(i);
		if (wadStats->getMapContent() != NULL)
			mapWads++;
	}
	for (int i=0; i<wadStatList->size(); i++) {
		wadStats = wadStatList->at(i);
		wcx = wadStats->getMapContent();
		while (wcx != NULL) {
			if (mapWads>1) {
				wxString str = wadStats->fileName + ":" + wcx->lumps->at(0)->name;
				result->push_back(str);
			} else {
				wxString str = wcx->lumps->at(0)->name;
				result->push_back(str);
			}
			wcx = wcx->next;
		}
	}
	return result;
}

int WadReader::getMapLumpSize(string fileName, string mapName)
{
	WadStats* wadStats = NULL;
	WadContentX* wcx = NULL;
	bool found = false;
	for (int i=0; i<wadStatList->size(); i++) {
		wadStats = wadStatList->at(i);
		if (fileName.length()==0 || fileName.compare(wadStats->fileName)==0)
			wcx = wadStats->getMapContent();
		else
			wcx = NULL;
		while (wcx != NULL) {
			if (wcx->lumps->at(0)->name.compare(mapName)==0) {
				found = true;
				break;
			}
			wcx = wcx->next;
		}
		if (found) break;
	}
	if (found) {
		return wcx->bytes;
	} else {
		return 0;
	}
}

MapStats* WadReader::processMap(string fileName, string mapName, TaskProgress* tp)
{
	WadStats* wadStats = NULL;
	WadContentX* wcx = NULL;
	bool found = false;
	for (int i=0; i<wadStatList->size(); i++) {
		wadStats = wadStatList->at(i);
		if (fileName.length()==0 || fileName.compare(wadStats->fileName)==0)
			wcx = wadStats->getMapContent();
		else
			wcx = NULL;
		while (wcx != NULL) {
			if (wcx->lumps->at(0)->name.compare(mapName)==0) {
				found = true;
				break;
			}
			wcx = wcx->next;
		}
		if (found) break;
	}
	if (found) {
		return loadMap(wcx, tp);
	} else {
		return NULL;
	}
}

WadEntry* WadReader::createEntries(wxString& imgFileFolder, TaskProgress* tp, MapEntry* mapTempl)
{
	int mapCount = 0;
	for (int i=0; i<wadStatList->size(); i++) {
		WadContentX* wcx = wadStatList->at(i)->getMapContent();
		while (wcx != NULL) {
			mapCount++;
			wcx = wcx->next;
		}
	}
	WadEntry* result = new WadEntry(0, mapCount);
	result->ownFlags = OF_MAINNEW|OF_OWNNEW;
	updateEntries(result, imgFileFolder, tp, true);
	if (mapTempl != NULL)
		result->setCommonFields(mapTempl);
	return result;
}

bool WadReader::updateEntries(WadEntry* wadEntry, wxString& imgFileFolder, TaskProgress* progress, bool newEntry)
{
	WadStats* wadStats = wadStatList->at(0); // First WadStats is primary file
	if (newEntry || aspects->wadFile) {
		wadEntry->fileName = wadStats->fileName;
		wadEntry->fileSize = wadStats->fileSize;
		for (int i=0; i<16; i++)
			wadEntry->md5Digest[i] = wadStats->md5Digest[i];
	}
	if (aspects->wadFile) {
		wadEntry->year = wadStats->year;
		if ((wadStats->wadType.compare("IWAD")==0) || (wadStats->wadType.compare("iwad")==0))
			wadEntry->flags |= WF_IWAD;
		wadEntry->ownFlags |= OF_HAVEFILE;
		wadEntry->iwad = iwad;
		wadEntry->engine = engine;
		// Add contribution from any additional WadStats and other files:
		bool exFiles=false;
		for (int i=1; i<wadStatList->size(); i++) {
			if (i==1)
				wadEntry->extraFiles = wadStatList->at(i)->fileName;
			else
				wadEntry->extraFiles = wadEntry->extraFiles+";"+wadStatList->at(i)->fileName;
			exFiles=true;
		}
		if ((dehacked!=NULL) && (dehacked->getFileName().Len()>0)) {
			if (exFiles)
				wadEntry->extraFiles = wadEntry->extraFiles+";"+dehacked->getFileName();
			else
				wadEntry->extraFiles = dehacked->getFileName();
		}
		if (!(wadEntry->ownFlags&OF_MAINNEW))
			wadEntry->ownFlags |= OF_MAINMOD;
	}

	//Content flags
	if (aspects->wadFlags) {
		setWadFlags(wadEntry, wadStats);
		for (int i=1; i<wadStatList->size(); i++)
			setWadFlags(wadEntry, wadStatList->at(i));
		if (dehacked != NULL) {
			wadEntry->flags |= WF_DEHBEX;
			if (dehacked->thingChanges() > 0)
				wadEntry->flags |= WF_THINGS;
		}
		if (!(wadEntry->ownFlags&OF_MAINNEW))
			wadEntry->ownFlags |= OF_MAINMOD;
	}

	wxLogVerbose("Processed WadEntry for %s, will process map entries",
		wadEntry->fileName);
	if (!aspects->mapAspects() && (!newEntry))
		return false;

	//Find number of maps
	int mapCount = 0;
	for (int i=0; i<wadStatList->size(); i++) {
		WadContentX* wcx = wadStatList->at(i)->getMapContent();
		while (wcx != NULL) {
			mapCount++;
			wcx = wcx->next;
		}
	}
	if (mapCount>1)
		progress->startCount(mapCount);
	//If we update an old WadEntry, we move maps from oldMaps to newMaps
	//when they are updated. Maps left in oldMaps will need to be deleted.
	list<MapEntry*> oldMaps;
	list<MapEntry*> newMaps;
	if (wadEntry->numberOfMaps > 0) {
		for (int i=0; i<wadEntry->numberOfMaps; i++)
			oldMaps.push_back(wadEntry->mapPointers[i]);
	}

	//Need to keep track of repeating use of same map name
	//Add letter postfix to make unique: "MAP01a", "MAP01b", ...
	map<string, int>* mapNames = new map<string, int>();
	bool repeatMapName;
	char postfix = 'a';
	for (int i=0; i<wadStatList->size(); i++) {
		repeatMapName = false;
		WadContentX* wcx = wadStatList->at(i)->getMapContent();
		bool glNodes = false;
		while (wcx != NULL) {
			TaskProgress* sub = new TaskProgress(wxString::Format("Processing map %s",wcx->lumps->at(0)->name), progress);
			MapStats* ms = loadMap(wcx, sub);
			bool failed = sub->hasFailed(true);
			delete sub;
			if (failed) {
				if (ms != NULL) delete ms;
				break;
			}
			string mapName = ms->mapName;
			(*mapNames)[mapName] = (*mapNames)[mapName]+1;
			if ((*mapNames)[mapName] > 1)
				repeatMapName = true;
			if (repeatMapName)
				mapName += postfix;
			if (ms->hasGLNodes())
				glNodes = true;
			MapEntry* me = wadEntry->getMap(mapName);
			if (me == NULL) {
				//No existing entry to update - make new
				me = new MapEntry(0);
				me->name = mapName;
				me->ownFlags = OF_MAINNEW|OF_OWNNEW;
				me->wadPointer = wadEntry;
			} else {
				oldMaps.remove(me);
			}
			updateMapEntry(me, ms);
			newMaps.push_back(me);
			wxLogVerbose("Processed MapEntry for %s", me->name);
			if (aspects->mapImages) {
				wxString imgFile = imgFileFolder+wxFILE_SEP_PATH+me->fileName()+".png";
				storeMapImage(ms, imgFile);
			}
			wcx = wcx->next;
			delete ms;
			if (mapCount>1)
				progress->incrCount();
		}
		if (glNodes && aspects->wadFlags) {
			wadEntry->flags |= WF_GLNODES;
		}
		if (repeatMapName)
			postfix++;
		if (postfix > 'z')
			progress->fatalError("Too many duplicate map names");
		if (progress->hasFailed()) break;
	}
	if (mapCount>1)
		progress->completeCount();
	delete mapNames;

	//Put newMaps into WadEntry. The WadEntry is either new, with empty vector of
	//correct size, or we are updating an existing WadEntry.
	int totalCount = newMaps.size() + oldMaps.size();
	if (totalCount != wadEntry->mapPointers.size())
		wadEntry->mapPointers.resize(totalCount);
	wadEntry->numberOfMaps = 0;
	for (list<MapEntry*>::iterator it=newMaps.begin(); it != newMaps.end(); ++it)
		wadEntry->addMap(*it);

	if (wadEntry->numberOfMaps==1 && aspects->mapMain) {
		wadEntry->title = wadEntry->mapPointers.at(0)->title;
	}

	//Any entries left in oldMaps need to be deleted. They are included in the WadEntry,
	//but flagged for deletion, and the function returns true.
	if (oldMaps.size() > 0) {
		wxLogVerbose("%i old map entries were not found in new version, will be deleted", oldMaps.size());
		for (list<MapEntry*>::iterator it=oldMaps.begin(); it != oldMaps.end(); ++it) {
			(*it)->ownFlags |= OFLG_DELETE;
			wadEntry->addMap(*it);
		}
		return true;
	} else {
		//No maps to delete
		return false;
	}
}

void WadReader::updateMapEntry(MapEntry* mapEntry, MapStats* mapStats)
{
	if (aspects->mapMain) {
		if (mapinfo != NULL)
			mapEntry->title = mapinfo->getMapTitle(mapEntry->name);
		else if (dehacked != NULL)
			mapEntry->title = dehacked->getMapTitle(mapEntry->name);
	}
	if (aspects->gameModes) {
		if (mapStats->getThingCount(THING_PLAYER1) > 0) {
			if (mapStats->getThingCount(THING_MONSTER) > 0)
				mapEntry->singlePlayer = 3;
			else
				mapEntry->singlePlayer = 1;
		} else {
			mapEntry->singlePlayer = 0;
		}
		if (mapStats->getThingCount(THING_COOP) > 0) {
			if (mapStats->coopOnlyThings) {
				mapEntry->cooperative = 3;
			} else if (mapStats->getThingCount(THING_MONSTER) == 0) {
				mapEntry->cooperative = 1;
			} else if (mapStats->multiOnlyThings) {
				mapEntry->cooperative = 3;
			} else {
				mapEntry->cooperative = 2;
			}
		} else {
			mapEntry->cooperative = 0;
		}
		if (mapStats->getThingCount(THING_DM) > 0) {
			if (mapStats->dmOnlyThings) {
				mapEntry->deathmatch = 3;
			} else if (mapStats->getThingCount(THING_MONSTER) == 0) {
				if (mapStats->multiOnlyThings || mapEntry->singlePlayer <= 1)
					mapEntry->deathmatch = 3;
				else
					mapEntry->deathmatch = 2;
			} else {
				mapEntry->deathmatch = 1;
			}
		} else {
			mapEntry->deathmatch = 0;
		}
		//result->otherMode
		if (!(mapEntry->ownFlags&OF_MAINNEW))
			mapEntry->ownFlags |= OF_MAINMOD;
	}

	if (aspects->mapStats) {
		mapEntry->linedefs = mapStats->getLineCount();
		mapEntry->sectors = mapStats->getSectorCount();
		mapEntry->things = mapStats->getTotalThings();
		mapEntry->secrets = mapStats->getSecretCount();
		mapEntry->area = mapStats->getMapArea() / AREA_FACTOR;
		if (mapStats->hasDifficultySetting())
			mapEntry->flags |= MF_DIFFSET;
		if (mapStats->getThingCount(THING_PLAYER1)>1)
			mapEntry->flags |= MF_VOODOO;
		if (mapStats->getThingCount(THING_UNKNOWN)>0)
			mapEntry->flags |= MF_UNKNOWN;
		if (!(mapEntry->ownFlags&OF_MAINNEW))
			mapEntry->ownFlags |= OF_MAINMOD;
	}

	if (aspects->gameStats) {
		ThingStats* thingStats = mapStats->getThingStats(2); //UV
		mapEntry->enemies = thingStats->monsters;
		mapEntry->totalHP = thingStats->monsterHP;
		mapEntry->healthRatio = thingStats->getHealthRate();
		mapEntry->armorRatio = thingStats->getArmorRate();
		mapEntry->ammoRatio = thingStats->getAmmoRate();
		if (thingStats->spawners)
			mapEntry->flags |= MF_SPAWN;
		if (!(mapEntry->ownFlags&OF_MAINNEW))
			mapEntry->ownFlags |= OF_MAINMOD;
	}
}

WadStats* WadReader::processWad(wxString fileName, bool pk3, TaskProgress* tp)
{
	wxFSFile* fsFile = fileSystem->OpenFile(fileName);
	if (fsFile == NULL) {
		tp->fatalError(wxString::Format("Couldn't open file %s",fileName));
		return NULL;
	}
	wxDateTime date = fsFile->GetModificationTime();
	int year = (date.IsValid())? date.GetYear(): 0;
	delete fsFile;

	WadStats* result;
	if (pk3) result = new Pk3Stats(fileName, tempFolder);
	else result = new WadStats(fileName);
	result->readFile(tp);
	result->year = year;

	if ((mapinfo==NULL) && !tp->hasFailed())
		mapinfo = result->getMapinfo(tp);

	return result;
}

DehackedParser* WadReader::processDeh(wxString fileName, TaskProgress* tp)
{
	wxFileName path = wxFileName(fileName);
	DehackedParser* parser = new DehackedParser(thingDefs, path.GetFullName());
	try {
		parser->parseFile(fileName, 0, -1);
		return parser;
	} catch (GuiError e) {
		tp->fatalError("Failed parsing Dehacked file");
		return NULL;
	}
}

DehackedParser* WadReader::processDeh(WadStats* wadStats, TaskProgress* tp)
{
	WadContentX* wcx = wadStats->getContent(WDEHACKED);
	if (wcx!=NULL && wcx->lumps->size()>0) {
		DirEntry* de = wcx->lumps->at(0);
		DehackedParser* parser = new DehackedParser(thingDefs, "");
		try {
			parser->parseFile(wcx->name, de->offset, de->size);
			return parser;
		} catch (GuiError e) {
			tp->fatalError("Failed parsing Dehacked lump");
		}
	}
	return NULL;
}

bool WadReader::processDecorate(WadStats* wadStats, TaskProgress* tp)
{
	bool result = false;
	WadContentX* wcx = wadStats->getContent(WDECORATE);
	while (wcx != NULL) {
		if (wcx->lumps->size() > 0) {
			DirEntry* de = wcx->lumps->at(0);
			if (de->offset > -1) {
				if (decorate == NULL) {
					decorate = new DecorateParser();
					decorate->setInitialDefs(thingDefs);
					if (mapinfo!=NULL)
						decorate->setActorNums(mapinfo->exportDoomEdNums());
				}
				try {
					decorate->parseFile(wcx->name, de->offset, de->size);
					result = true;
				} catch (GuiError e) {
					tp->warnError("Failed parsing decorate");
				}
			}
		}
		wcx = wcx->next;
	}
	return result;
}

MapStats* WadReader::loadMap(WadContentX* mapEntry, TaskProgress* tp)
{
	wxFileInputStream file(mapEntry->name);
	if (!file.IsOk()) {
		tp->fatalError("Couldn't open file.");
		return NULL;
	}
	wxBufferedInputStream* buf = new wxBufferedInputStream(file, 2048);

	bool udmfFormat = mapEntry->containsLump("TEXTMAP");
	bool hexenFormat = mapEntry->containsLump("BEHAVIOR");
	MapStats* result;
	if (udmfFormat)
		result = new UdmfMapStats(mapEntry->lumps->at(0)->name, engine, tp);
	else if (hexenFormat)
		result = new HexenMapStats(mapEntry->lumps->at(0)->name, engine, tp);
	else
		result = new MapStats(mapEntry->lumps->at(0)->name, engine, tp);
	map<int, ThingDef*>* tm = (thingDefs==NULL)? NULL: thingDefs->getMapPointer();
	result->readFile(buf, mapEntry->lumps, tm);
	delete buf;
	if (tp->hasFailed()) {
		delete result;
		result = NULL;
	}
	return result;
}

void WadReader::setWadFlags(WadEntry* wadEntry, WadStats* wadStats)
{
	if ((wadStats->content[WSPRITE]!=NULL) && (wadStats->content[WSPRITE]->count>0))
		wadEntry->flags |= WF_SPRITES;
	if ((wadStats->content[WPATCH]!=NULL) && (wadStats->content[WPATCH]->count>0))
		wadEntry->flags |= WF_TEX;
	if ((wadStats->content[WFLAT]!=NULL) && (wadStats->content[WFLAT]->count>0))
		wadEntry->flags |= WF_TEX;
	if ((wadStats->content[WTX]!=NULL) && (wadStats->content[WTX]->count>0))
		wadEntry->flags |= WF_TEX;
	if ((wadStats->content[WFONT]!=NULL) && (wadStats->content[WFONT]->count>0))
		wadEntry->flags |= WF_GFX;
	if ((wadStats->content[WGFX]!=NULL) && (wadStats->content[WGFX]->count>0))
		wadEntry->flags |= WF_GFX;
	if ((wadStats->content[WBITMAP]!=NULL) && (wadStats->content[WBITMAP]->count>0))
		wadEntry->flags |= WF_GFX;
	if (wadStats->content[WPLAYPAL]!=NULL)
		wadEntry->flags |= WF_COLOR;
	if ((wadStats->content[WSFX]!=NULL) && (wadStats->content[WSFX]->count>0))
		wadEntry->flags |= WF_SOUND;
	if ((wadStats->content[WWAV]!=NULL) && (wadStats->content[WWAV]->count>0))
		wadEntry->flags |= WF_SOUND;
	if ((wadStats->content[WOTHERSND]!=NULL) && (wadStats->content[WOTHERSND]->count>0))
		wadEntry->flags |= WF_SOUND;
	if ((wadStats->content[WMUS]!=NULL) && (wadStats->content[WMUS]->count>0))
		wadEntry->flags |= WF_MUSIC;
	if ((wadStats->content[WMIDI]!=NULL) && (wadStats->content[WMIDI]->count>0))
		wadEntry->flags |= WF_MUSIC;
	if ((wadStats->content[WOTHERMUS]!=NULL) && (wadStats->content[WOTHERMUS]->count>0))
		wadEntry->flags |= WF_MUSIC;
	if ((wadStats->content[WDEHACKED]!=NULL) && (wadStats->content[WDEHACKED]->count>0))
		wadEntry->flags |= WF_DEHBEX;
	if ((wadStats->content[WDECORATE]!=NULL) && (wadStats->content[WDECORATE]->count>0))
		wadEntry->flags |= WF_THINGS;
	if ((wadStats->content[WBEHAVIOR]!=NULL) && (wadStats->content[WBEHAVIOR]->count>0))
		wadEntry->flags |= WF_SCRIPT;
	if ((wadStats->content[WACS]!=NULL) && (wadStats->content[WACS]->count>0))
		wadEntry->flags |= WF_SCRIPT;
	//WF_GLNODES set from MapStats
}

void WadReader::storeMapImage(MapStats* mapStats, wxString& fileName)
{
	int width = (mapStats->getEndCorner().x - mapStats->getStartCorner().x)/DRAW_SCALE;
	width+=3;
	int height = (mapStats->getEndCorner().y - mapStats->getStartCorner().y)/DRAW_SCALE;
	height+=3;
	wxLogVerbose("Drawing map PNG of %i x %i pixels", width, height);
	wxBitmap bm(width, height, wxBITMAP_SCREEN_DEPTH);
	wxMemoryDC temp_dc(bm);
	drawMap(mapStats, temp_dc);
	temp_dc.SelectObject(wxNullBitmap);
	bool ok = bm.SaveFile(fileName, wxBITMAP_TYPE_PNG);
	if (ok)
		wxLogVerbose("Map PNG saved to file");
	else
		wxLogVerbose("Failed saving map PNG to file");
}

void WadReader::drawMap(MapStats* mapStats, wxDC& dc)
{
	wxBrush bckgrnd(MAP_BACKGROUNDCOLOR);
	dc.SetBackground(bckgrnd);
	dc.Clear();

	wxPen wall1Pen(MAP_WALL1, 1, wxSOLID);
	wxPen wall2Pen(MAP_WALL2, 1, wxSOLID);

	vector<Vertex>* vertices = mapStats->getVertices();
	vector<MapLine>* lines = mapStats->getLines();
	int xTrans = mapStats->getStartCorner().x;
	int yTrans = mapStats->getEndCorner().y;

	MapLine l;
	int x1, x2, y1, y2;
	for (int i=0; i<lines->size(); i++) {
		l = lines->at(i);
		if (l.twoSided)
			dc.SetPen(wall2Pen);
		else
			dc.SetPen(wall1Pen);
        x1 = ((vertices->at(l.v1).x - xTrans)/DRAW_SCALE) + 1;
        y1 = (( (vertices->at(l.v1).y * -1) + yTrans)/DRAW_SCALE) + 1;
        x2 = ((vertices->at(l.v2).x - xTrans)/DRAW_SCALE) + 1;
        y2 = (( (vertices->at(l.v2).y * -1) + yTrans)/DRAW_SCALE) + 1;
		dc.DrawLine(x1, y1, x2, y2);
	}
}
