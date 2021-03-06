#include "StatisticSet.h"

//*************************************************************
//************************ Comparators ************************
//*************************************************************

StatFields statisticSetSortField;

bool stat_comp_int(DBStatistics* first, DBStatistics* second)
{
	return (first->intStats[statisticSetSortField] > second->intStats[statisticSetSortField]);
}

bool stat_comp_float(DBStatistics* first, DBStatistics* second)
{
	return (first->floatStats[statisticSetSortField] > second->floatStats[statisticSetSortField]);
}

bool stat_comp_name(DBStatistics* first, DBStatistics* second)
{
	return (first->heading.CmpNoCase(second->heading) < 0);
}


//**************************************************************
//************************ StatisticSet ************************
//**************************************************************

StatisticSet::StatisticSet(wxString category, wxString name)
: fieldCount(0), categ(category), setName(name), statList(NULL)
{
	for (int i=0; i<30; i++) {
		fields[i] = STS_END;
		width[i] = 0;
	}
}

StatisticSet::~StatisticSet()
{
	if (statList != NULL) {
		for (list<DBStatistics*>::iterator it=statList->begin(); it!=statList->end(); ++it)
			delete *it;
		delete statList;
	}
}

void StatisticSet::processWad(WadEntry* wadEntry)
{
	//For map statistics - override for wad statistics
	for (int i=0; i<wadEntry->numberOfMaps; i++)
		processMap(wadEntry->mapPointers.at(i));
}

void StatisticSet::sort(int field)
{
	if (field==0) {
		comp = stat_comp_name;
	} else {
		statisticSetSortField = fields[field];
		if (statisticSetSortField<STS_LINEDEFS_AVG)
			comp = stat_comp_int;
		else
			comp = stat_comp_float;
	}
	statList->sort(*comp);
}

void StatisticSet::addField(StatFields field, uint16_t w)
{
	if (fieldCount < 30) {
		fields[fieldCount] = field;
		width[fieldCount] = w;
		fieldCount++;
	}
}


//*************************************************************
//************************ YearStatSet ************************
//*************************************************************

YearStatSet::YearStatSet(wxString setName) : StatisticSet("Year", setName)
{
	statMap = new map<int, MapStatistics*>();
	addField(STS_YEAR_MIN, 60);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

YearStatSet::~YearStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void YearStatSet::processMap(MapEntry* mapEntry)
{
	map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->wadPointer->year);
	if (it == statMap->end()) {
		wxString name = (mapEntry->wadPointer->year==0)? "Unknown" :
				wxString::Format("%i", mapEntry->wadPointer->year);
		MapStatistics* ms = new MapStatistics(name);
		ms->processMap(mapEntry);
		(*statMap)[mapEntry->wadPointer->year] = ms;
	} else {
		it->second->processMap(mapEntry);
	}
}

void YearStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//*************************************************************
//************************ IwadStatSet ************************
//*************************************************************

IwadStatSet::IwadStatSet(wxString setName) : StatisticSet("Iwad", setName)
{
	statMap = new map<int, MapStatistics*>();
	addField(STS_END, 100);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

IwadStatSet::~IwadStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void IwadStatSet::processMap(MapEntry* mapEntry)
{
	map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->wadPointer->iwad);
	if (it == statMap->end()) {
		wxString name = (mapEntry->wadPointer->iwad==0)? "Unknown" :
				iwadNames[mapEntry->wadPointer->iwad];
		MapStatistics* ms = new MapStatistics(name);
		ms->processMap(mapEntry);
		(*statMap)[mapEntry->wadPointer->iwad] = ms;
	} else {
		it->second->processMap(mapEntry);
	}
}

void IwadStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//***************************************************************
//************************ EngineStatSet ************************
//***************************************************************

EngineStatSet::EngineStatSet(wxString setName) : StatisticSet("Engine", setName)
{
	statMap = new map<int, MapStatistics*>();
	addField(STS_END, 100);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

EngineStatSet::~EngineStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void EngineStatSet::processMap(MapEntry* mapEntry)
{
	map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->wadPointer->engine);
	if (it == statMap->end()) {
		wxString name = (mapEntry->wadPointer->engine==0)? "Unknown" :
				engineNames[mapEntry->wadPointer->engine];
		MapStatistics* ms = new MapStatistics(name);
		ms->processMap(mapEntry);
		(*statMap)[mapEntry->wadPointer->engine] = ms;
	} else {
		it->second->processMap(mapEntry);
	}
}

void EngineStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//***************************************************************
//************************ RatingStatSet ************************
//***************************************************************

RatingStatSet::RatingStatSet(wxString setName) : StatisticSet("Own rating", setName)
{
	statMap = new map<int, MapStatistics*>();
	for (int i=0; i<=10; i++)
		(*statMap)[i] = new MapStatistics(wxString::Format("%i",i));
	(*statMap)[11] = new MapStatistics("No rating");

	addField(STS_END, 80);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
}

RatingStatSet::~RatingStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void RatingStatSet::processMap(MapEntry* mapEntry)
{
	int rat = 11;
	if (mapEntry->ownRating<=100)
		rat = mapEntry->ownRating/10;
	map<int,MapStatistics*>::iterator it = statMap->find(rat);
	it->second->processMap(mapEntry);
}

void RatingStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//*******************************************************************
//************************ DifficultyStatSet ************************
//*******************************************************************

DifficultyStatSet::DifficultyStatSet(wxString setName) : StatisticSet("Difficulty", setName)
{
	statMap = new map<int, MapStatistics*>();
	(*statMap)[0] = new MapStatistics("Not rated");
	for (int i=1; i<5; i++)
		(*statMap)[i] = new MapStatistics(difficultyRatings[i]);

	addField(STS_END, 120);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

DifficultyStatSet::~DifficultyStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void DifficultyStatSet::processMap(MapEntry* mapEntry)
{
	map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->difficulty);
	it->second->processMap(mapEntry);
}

void DifficultyStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//******************************************************************
//************************ PlaystyleStatSet ************************
//******************************************************************

PlaystyleStatSet::PlaystyleStatSet(wxString setName) : StatisticSet("Playstyle", setName)
{
	statMap = new map<int, MapStatistics*>();
	for (int i=0; i<3; i++)
		(*statMap)[i] = new MapStatistics(playStyles[i]);

	addField(STS_END, 120);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

PlaystyleStatSet::~PlaystyleStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void PlaystyleStatSet::processMap(MapEntry* mapEntry)
{
	map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->wadPointer->playStyle);
	it->second->processMap(mapEntry);
}

void PlaystyleStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//***************************************************************
//************************ AuthorStatSet ************************
//***************************************************************

AuthorStatSet::AuthorStatSet(wxString setName, map<int, MapStatistics*>* authorStats)
: StatisticSet("Author", setName)
{
	unknown = new MapStatistics("Unknown");
	statMap = authorStats;
	addField(STS_END, 200);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

AuthorStatSet::~AuthorStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void AuthorStatSet::processMap(MapEntry* mapEntry)
{
	if (mapEntry->author1 != NULL) {
		map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->author1->dbid);
		it->second->processMap(mapEntry);
	} else if (mapEntry->author2 == NULL) {
		unknown->processMap(mapEntry);
	}
	if (mapEntry->author2 != NULL) {
		map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->author2->dbid);
		it->second->processMap(mapEntry);
	}
}

void AuthorStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
	if (unknown->intStats[STS_MAPS] > 0) {
		statList->push_back(unknown);
	} else {
		delete unknown;
		unknown = NULL;
	}
}


//************************************************************
//************************ TagStatSet ************************
//************************************************************

TagStatSet::TagStatSet(wxString setName, map<int, MapStatistics*>* tagStats)
: StatisticSet("Style tag", setName)
{
	none = new MapStatistics("None");
	statMap = tagStats;
	addField(STS_END, 200);
	addField(STS_MAPS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_SINGLE, 50);
	addField(STS_COOP, 50);
	addField(STS_DM, 50);
	addField(STS_LINEDEFS, 80);
	addField(STS_LINEDEFS_AVG, 80);
	addField(STS_SECTORS, 80);
	addField(STS_SECTORS_AVG, 80);
	addField(STS_THINGS, 80);
	addField(STS_THINGS_AVG, 80);
	addField(STS_SECRETS_AVG, 80);
	addField(STS_ENEMIES_AVG, 80);
	addField(STS_TOTALHP_AVG, 80);
	addField(STS_AMMORAT_AVG, 80);
	addField(STS_HEALTHRAT_AVG, 80);
	addField(STS_ARMORRAT_AVG, 80);
	addField(STS_AREA_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

TagStatSet::~TagStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void TagStatSet::processMap(MapEntry* mapEntry)
{
	bool bnone = true;
	for (int i=0; i<MAXTAGS; i++) {
		if (mapEntry->tags[i] != 0) {
			map<int,MapStatistics*>::iterator it = statMap->find(mapEntry->tags[i]);
			it->second->processMap(mapEntry);
			bnone=false;
		}
	}
	if (bnone)
		none->processMap(mapEntry);
}

void TagStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,MapStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
	if (none->intStats[STS_MAPS] > 0) {
		statList->push_back(none);
	} else {
		delete none;
		none = NULL;
	}
}


//****************************************************************
//************************ YearWadStatSet ************************
//****************************************************************

YearWadStatSet::YearWadStatSet(wxString setName) : StatisticSet("Year", setName)
{
	statMap = new map<int, WadStatistics*>();
	addField(STS_YEAR_MIN, 60);
	addField(STS_WADS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_MAPS, 80);
	addField(STS_MAPS_AVG, 80);
	addField(STS_MAPS_MIN, 80);
	addField(STS_MAPS_MAX, 80);
	addField(STS_SIZE, 100);
	addField(STS_SIZE_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

YearWadStatSet::~YearWadStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void YearWadStatSet::processWad(WadEntry* wadEntry)
{
	map<int,WadStatistics*>::iterator it = statMap->find(wadEntry->year);
	if (it == statMap->end()) {
		wxString name = (wadEntry->year==0)? "Unknown" :
				wxString::Format("%i", wadEntry->year);
		WadStatistics* ms = new WadStatistics(name);
		ms->processWad(wadEntry);
		(*statMap)[wadEntry->year] = ms;
	} else {
		it->second->processWad(wadEntry);
	}
}

void YearWadStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,WadStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//****************************************************************
//************************ IwadWadStatSet ************************
//****************************************************************

IwadWadStatSet::IwadWadStatSet(wxString setName) : StatisticSet("Iwad", setName)
{
	statMap = new map<int, WadStatistics*>();
	addField(STS_END, 100);
	addField(STS_WADS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_MAPS, 80);
	addField(STS_MAPS_AVG, 80);
	addField(STS_MAPS_MIN, 80);
	addField(STS_MAPS_MAX, 80);
	addField(STS_SIZE, 100);
	addField(STS_SIZE_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

IwadWadStatSet::~IwadWadStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void IwadWadStatSet::processWad(WadEntry* wadEntry)
{
	map<int,WadStatistics*>::iterator it = statMap->find(wadEntry->iwad);
	if (it == statMap->end()) {
		wxString name = (wadEntry->iwad==0)? "Unknown" :
				iwadNames[wadEntry->iwad];
		WadStatistics* ms = new WadStatistics(name);
		ms->processWad(wadEntry);
		(*statMap)[wadEntry->iwad] = ms;
	} else {
		it->second->processWad(wadEntry);
	}
}

void IwadWadStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,WadStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//******************************************************************
//************************ EngineWadStatSet ************************
//******************************************************************

EngineWadStatSet::EngineWadStatSet(wxString setName) : StatisticSet("Engine", setName)
{
	statMap = new map<int, WadStatistics*>();
	addField(STS_END, 100);
	addField(STS_WADS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_MAPS, 80);
	addField(STS_MAPS_AVG, 80);
	addField(STS_MAPS_MIN, 80);
	addField(STS_MAPS_MAX, 80);
	addField(STS_SIZE, 100);
	addField(STS_SIZE_AVG, 80);
	addField(STS_OWNRATING_AVG, 60);
}

EngineWadStatSet::~EngineWadStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void EngineWadStatSet::processWad(WadEntry* wadEntry)
{
	map<int,WadStatistics*>::iterator it = statMap->find(wadEntry->engine);
	if (it == statMap->end()) {
		wxString name = (wadEntry->engine==0)? "Unknown" :
				engineNames[wadEntry->engine];
		WadStatistics* ms = new WadStatistics(name);
		ms->processWad(wadEntry);
		(*statMap)[wadEntry->engine] = ms;
	} else {
		it->second->processWad(wadEntry);
	}
}

void EngineWadStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,WadStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}


//******************************************************************
//************************ RatingWadStatSet ************************
//******************************************************************

RatingWadStatSet::RatingWadStatSet(wxString setName) : StatisticSet("Own rating", setName)
{
	statMap = new map<int, WadStatistics*>();
	for (int i=0; i<=10; i++)
		(*statMap)[i] = new WadStatistics(wxString::Format("%i",i));
	(*statMap)[11] = new WadStatistics("No rating");

	addField(STS_END, 100);
	addField(STS_WADS, 50);
	addField(STS_WF_IWAD, 50);
	addField(STS_WF_THINGS, 50);
	addField(STS_MAPS, 80);
	addField(STS_MAPS_AVG, 80);
	addField(STS_MAPS_MIN, 80);
	addField(STS_MAPS_MAX, 80);
	addField(STS_SIZE, 100);
	addField(STS_SIZE_AVG, 80);
}

RatingWadStatSet::~RatingWadStatSet()
{
	if (statMap != NULL)
		delete statMap;
}

void RatingWadStatSet::processWad(WadEntry* wadEntry)
{
	int rat = 11;
	if (wadEntry->ownRating<=100)
		rat = wadEntry->ownRating/10;
	map<int,WadStatistics*>::iterator it = statMap->find(rat);
	it->second->processWad(wadEntry);
}

void RatingWadStatSet::computeResults()
{
	statList = new list<DBStatistics*>();
	for (map<int,WadStatistics*>::iterator it=statMap->begin(); it!=statMap->end(); ++it) {
		it->second->computeResults();
		statList->push_back(it->second);
	}
	delete statMap;
	statMap = NULL;
}
