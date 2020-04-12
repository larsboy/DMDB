#include "MapStatistics.h"

MapStatistics::MapStatistics(wxString name)
: heading(name)
{
	for (int i=0; i<STS_LINEDEFS_AVG; i++)
		intStats[i] = 0;
	setIntMins(INVALID_MIN_INT);
	
	for (int i=0; i<STS_END; i++)
		floatStats[i] = 0.0;
	setFloatMins(INVALID_MIN_FLOAT);
}

MapStatistics::~MapStatistics()
{
}

void MapStatistics::setIntMins(unsigned long minValue)
{
	intStats[STS_YEAR_MIN] = minValue;
	intStats[STS_LINEDEFS_MIN] = minValue;
	intStats[STS_SECTORS_MIN] = minValue;
	intStats[STS_THINGS_MIN] = minValue;
	intStats[STS_SECRETS_MIN] = minValue;
	intStats[STS_ENEMIES_MIN] = minValue;
	intStats[STS_TOTALHP_MIN] = minValue;
}
		
void MapStatistics::setFloatMins(double minValue)
{
	floatStats[STS_HEALTHRAT_MIN] = minValue;
	floatStats[STS_ARMORRAT_MIN] = minValue;
	floatStats[STS_AMMORAT_MIN] = minValue;
	floatStats[STS_AREA_MIN] = minValue;
}

void MapStatistics::processMap(MapEntry* mapEntry)
{
	intStats[STS_COUNT]++;
	processWad(mapEntry->wadPointer, 1);
	if (mapEntry->singlePlayer == 3) intStats[STS_SINGLE]++;
	if (mapEntry->cooperative == 3) intStats[STS_COOP]++;
	if (mapEntry->deathmatch == 3) intStats[STS_DM]++;
	processGeometry(mapEntry);
	if (mapEntry->enemies>0 && mapEntry->totalHP>0)
		processGameplay(mapEntry);
	if (mapEntry->flags&MF_DIFFSET) intStats[STS_MF_DIFFSET]++;
	if (mapEntry->flags&MF_VOODOO) intStats[STS_MF_VOODOO]++;
	if (mapEntry->flags&MF_UNKNOWN) intStats[STS_MF_UNKNOWN]++;
	if (mapEntry->isOwnRated()) {
		intStats[STS_OWNRATED]++;
		intStats[STS_OWNRATING] += mapEntry->ownRating;
	}
	if (mapEntry->played > 0) intStats[STS_PLAYED]++;
}

void MapStatistics::processWad(WadEntry* wadEntry)
{
	intStats[STS_COUNT]+=wadEntry->numberOfMaps;
	processWad(wadEntry, wadEntry->numberOfMaps);
	MapEntry* mapEntry;
	for (int i=0; i<wadEntry->numberOfMaps; i++) {
		mapEntry = wadEntry->mapPointers.at(i);
		if (mapEntry->singlePlayer == 3) intStats[STS_SINGLE]++;
		if (mapEntry->cooperative == 3) intStats[STS_COOP]++;
		if (mapEntry->deathmatch == 3) intStats[STS_DM]++;
		processGeometry(mapEntry);
		if (mapEntry->enemies>0 && mapEntry->totalHP>0)
			processGameplay(mapEntry);
		if (mapEntry->flags&MF_DIFFSET) intStats[STS_MF_DIFFSET]++;
		if (mapEntry->flags&MF_VOODOO) intStats[STS_MF_VOODOO]++;
		if (mapEntry->flags&MF_UNKNOWN) intStats[STS_MF_UNKNOWN]++;
		if (mapEntry->isOwnRated()) {
			intStats[STS_OWNRATED]++;
			intStats[STS_OWNRATING] += mapEntry->ownRating;
		}
		if (mapEntry->played > 0) intStats[STS_PLAYED]++;
	}
}

void MapStatistics::processWad(WadEntry* wadEntry, int maps)
{
	if (wadEntry->year > 0) {
		if (wadEntry->year < intStats[STS_YEAR_MIN])
			intStats[STS_YEAR_MIN] = wadEntry->year;
		if (wadEntry->year > intStats[STS_YEAR_MAX])
			intStats[STS_YEAR_MAX] = wadEntry->year;
	}
	if (wadEntry->flags&WF_IWAD) intStats[STS_WF_IWAD]+=maps;
	if (wadEntry->flags&WF_SPRITES) intStats[STS_WF_SPRITES]+=maps;
	if (wadEntry->flags&WF_TEX) intStats[STS_WF_TEX]+=maps;
	if (wadEntry->flags&WF_GFX) intStats[STS_WF_GFX]+=maps;
	if (wadEntry->flags&WF_COLOR) intStats[STS_WF_COLOR]+=maps;
	if (wadEntry->flags&WF_SOUND) intStats[STS_WF_SOUND]+=maps;
	if (wadEntry->flags&WF_MUSIC) intStats[STS_WF_MUSIC]+=maps;
	if (wadEntry->flags&WF_DEHBEX) intStats[STS_WF_DEHBEX]+=maps;
	if (wadEntry->flags&WF_THINGS) intStats[STS_WF_THINGS]+=maps;
	if (wadEntry->flags&WF_SCRIPT) intStats[STS_WF_SCRIPT]+=maps;
	if (wadEntry->flags&WF_GLNODES) intStats[STS_WF_GLNODES]+=maps;
	if (wadEntry->ownFlags&OF_HAVEFILE) intStats[STS_OF_HAVEFILE]+=maps;
}

void MapStatistics::processGeometry(MapEntry* mapEntry)
{
	intStats[STS_LINEDEFS] += mapEntry->linedefs;
	if (mapEntry->linedefs < intStats[STS_LINEDEFS_MIN])
		intStats[STS_LINEDEFS_MIN] = mapEntry->linedefs;
	if (mapEntry->linedefs > intStats[STS_LINEDEFS_MAX])
		intStats[STS_LINEDEFS_MAX] = mapEntry->linedefs;
	intStats[STS_SECTORS] += mapEntry->sectors;
	if (mapEntry->sectors < intStats[STS_SECTORS_MIN])
		intStats[STS_SECTORS_MIN] = mapEntry->sectors;
	if (mapEntry->sectors > intStats[STS_SECTORS_MAX])
		intStats[STS_SECTORS_MAX] = mapEntry->sectors;
	intStats[STS_THINGS] += mapEntry->things;
	if (mapEntry->things < intStats[STS_THINGS_MIN])
		intStats[STS_THINGS_MIN] = mapEntry->things;
	if (mapEntry->things > intStats[STS_THINGS_MAX])
		intStats[STS_THINGS_MAX] = mapEntry->things;
	intStats[STS_SECRETS] += mapEntry->secrets;
	if (mapEntry->secrets < intStats[STS_SECRETS_MIN])
		intStats[STS_SECRETS_MIN] = mapEntry->secrets;
	if (mapEntry->secrets > intStats[STS_SECRETS_MAX])
		intStats[STS_SECRETS_MAX] = mapEntry->secrets;
	if (mapEntry->area > 0.0) {
		intStats[STS_AREAS]++;
		floatStats[STS_AREA_AVG] += mapEntry->area;
		if (mapEntry->area < floatStats[STS_AREA_MIN])
			floatStats[STS_AREA_MIN] = mapEntry->area;
		if (mapEntry->area > floatStats[STS_AREA_MAX])
			floatStats[STS_AREA_MAX] = mapEntry->area;
	}
}

void MapStatistics::processGameplay(MapEntry* mapEntry)
{
	intStats[STS_GAMESTATS]++;
	intStats[STS_ENEMIES] += mapEntry->enemies;
	if (mapEntry->enemies < intStats[STS_ENEMIES_MIN])
		intStats[STS_ENEMIES_MIN] = mapEntry->enemies;
	if (mapEntry->enemies > intStats[STS_ENEMIES_MAX])
		intStats[STS_ENEMIES_MAX] = mapEntry->enemies;
	intStats[STS_TOTALHP] += mapEntry->totalHP;
	if (mapEntry->totalHP < intStats[STS_TOTALHP_MIN])
		intStats[STS_TOTALHP_MIN] = mapEntry->totalHP;
	if (mapEntry->totalHP > intStats[STS_TOTALHP_MAX])
		intStats[STS_TOTALHP_MAX] = mapEntry->totalHP;
	
	floatStats[STS_HEALTHRAT_AVG] += mapEntry->healthRatio;
	if (mapEntry->healthRatio < floatStats[STS_HEALTHRAT_MIN])
		floatStats[STS_HEALTHRAT_MIN] = mapEntry->healthRatio;
	if (mapEntry->healthRatio > floatStats[STS_HEALTHRAT_MAX])
		floatStats[STS_HEALTHRAT_MAX] = mapEntry->healthRatio;
	floatStats[STS_ARMORRAT_AVG] += mapEntry->armorRatio;
	if (mapEntry->armorRatio < floatStats[STS_ARMORRAT_MIN])
		floatStats[STS_ARMORRAT_MIN] = mapEntry->armorRatio;
	if (mapEntry->armorRatio > floatStats[STS_ARMORRAT_MAX])
		floatStats[STS_ARMORRAT_MAX] = mapEntry->armorRatio;
	floatStats[STS_AMMORAT_AVG] += mapEntry->ammoRatio;
	if (mapEntry->ammoRatio < floatStats[STS_AMMORAT_MIN])
		floatStats[STS_AMMORAT_MIN] = mapEntry->ammoRatio;
	if (mapEntry->ammoRatio > floatStats[STS_AMMORAT_MAX])
		floatStats[STS_AMMORAT_MAX] = mapEntry->ammoRatio;
	
	if (mapEntry->flags&MF_SPAWN) intStats[STS_MF_SPAWN]++;
	if (mapEntry->flags&MF_MORESPAWN) intStats[STS_MF_MORESPAWN]++;
}

void MapStatistics::computeResults()
{
	if (intStats[STS_COUNT] > 0) {
		floatStats[STS_LINEDEFS_AVG] = (double)intStats[STS_LINEDEFS]/(double)intStats[STS_COUNT];
		floatStats[STS_SECTORS_AVG] = (double)intStats[STS_SECTORS]/(double)intStats[STS_COUNT];
		floatStats[STS_THINGS_AVG] = (double)intStats[STS_THINGS]/(double)intStats[STS_COUNT];
		floatStats[STS_SECRETS_AVG] = (double)intStats[STS_SECRETS]/(double)intStats[STS_COUNT];
	} else {
		intStats[STS_LINEDEFS_MIN] = 0;
		intStats[STS_SECTORS_MIN] = 0;
		intStats[STS_THINGS_MIN] = 0;
		intStats[STS_SECRETS_MIN] = 0;
	}
	if (intStats[STS_YEAR_MIN]==INVALID_MIN_INT)
		intStats[STS_YEAR_MIN] = 0;
	if (intStats[STS_AREAS] > 0)
		floatStats[STS_AREA_AVG] = floatStats[STS_AREA_AVG]/(double)intStats[STS_AREAS];
	else
		floatStats[STS_AREA_MIN] = 0.0;
	
	if (intStats[STS_GAMESTATS] > 0) {
		floatStats[STS_ENEMIES_AVG] = (double)intStats[STS_ENEMIES]/(double)intStats[STS_GAMESTATS];
		floatStats[STS_TOTALHP_AVG] = (double)intStats[STS_TOTALHP]/(double)intStats[STS_GAMESTATS];
		floatStats[STS_HEALTHRAT_AVG] = floatStats[STS_HEALTHRAT_AVG]/(double)intStats[STS_GAMESTATS];
		floatStats[STS_ARMORRAT_AVG] = floatStats[STS_ARMORRAT_AVG]/(double)intStats[STS_GAMESTATS];
		floatStats[STS_AMMORAT_AVG] = floatStats[STS_AMMORAT_AVG]/(double)intStats[STS_GAMESTATS];
	} else {
		intStats[STS_ENEMIES_MIN] = 0;
		intStats[STS_TOTALHP_MIN] = 0;
		floatStats[STS_HEALTHRAT_MIN] = 0.0;
		floatStats[STS_ARMORRAT_MIN] = 0.0;
		floatStats[STS_AMMORAT_MIN] = 0.0;
	}
	
	if (intStats[STS_OWNRATED] > 0)
		floatStats[STS_OWNRATING_AVG] = (double)intStats[STS_OWNRATING]/(double)intStats[STS_OWNRATED];
}

void MapStatistics::printReport(TextReport* reportView)
{
	reportView->writeHeading(heading);
	//long lng = intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("Total maps: %u", intStats[STS_COUNT]));
	reportView->writeLine(wxString::Format("Year range: %i - %i", intStats[STS_YEAR_MIN], intStats[STS_YEAR_MAX]));
	
	reportView->writeSubHeading("Counts");
	wxArrayInt tabs;
	tabs.Add(450);tabs.Add(650);tabs.Add(850);tabs.Add(1050);
	reportView->setTabs(tabs);
	reportView->writeText("Played\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_PLAYED]));
	double percent = (double)(intStats[STS_PLAYED]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Made for single player\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_SINGLE]));
	percent = (double)(intStats[STS_SINGLE]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Made for coop\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_COOP]));
	percent = (double)(intStats[STS_COOP]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Made for deathmatch\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_DM]));
	percent = (double)(intStats[STS_DM]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	
	reportView->writeText("Have file\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_OF_HAVEFILE]));
	percent = (double)(intStats[STS_OF_HAVEFILE]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("In IWAD\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_IWAD]));
	percent = (double)(intStats[STS_WF_IWAD]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sprites\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SPRITES]));
	percent = (double)(intStats[STS_WF_SPRITES]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Textures\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_TEX]));
	percent = (double)(intStats[STS_WF_TEX]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Other graphics\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_GFX]));
	percent = (double)(intStats[STS_WF_GFX]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Palette\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_COLOR]));
	percent = (double)(intStats[STS_WF_COLOR]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sounds\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SOUND]));
	percent = (double)(intStats[STS_WF_SOUND]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Music\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_MUSIC]));
	percent = (double)(intStats[STS_WF_MUSIC]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Dehacked/BEX\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_DEHBEX]));
	percent = (double)(intStats[STS_WF_DEHBEX]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Monsters/items\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_THINGS]));
	percent = (double)(intStats[STS_WF_THINGS]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Scripts\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SCRIPT]));
	percent = (double)(intStats[STS_WF_SCRIPT]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("GL nodes\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_GLNODES]));
	percent = (double)(intStats[STS_WF_GLNODES]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	
	reportView->writeText("Difficulty settings\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_MF_DIFFSET]));
	percent = (double)(intStats[STS_MF_DIFFSET]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Voodoo dolls\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_MF_VOODOO]));
	percent = (double)(intStats[STS_MF_VOODOO]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Unknown things\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_MF_UNKNOWN]));
	percent = (double)(intStats[STS_MF_UNKNOWN]*100) / (double)intStats[STS_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	
	reportView->writeSubHeading("Geometry statistics");
	reportView->writeLine("\tTotal\tAverage\tMin\tMax");
	reportView->writeText("Linedefs\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_LINEDEFS]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_LINEDEFS_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_LINEDEFS_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_LINEDEFS_MAX]));
	reportView->writeText("Sectors\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_SECTORS]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_SECTORS_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_SECTORS_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_SECTORS_MAX]));
	reportView->writeText("Things\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_THINGS]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_THINGS_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_THINGS_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_THINGS_MAX]));
	reportView->writeText("Secrets\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_SECRETS]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_SECRETS_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_SECRETS_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_SECRETS_MAX]));
	reportView->writeText("Area (MU)\t\t");
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_AREA_AVG]));
	reportView->writeText(wxString::Format("%.3f\t", floatStats[STS_AREA_MIN]));
	reportView->writeLine(wxString::Format("%.3f", floatStats[STS_AREA_MAX]));
	reportView->writeLine(wxString::Format("Maps with computed area: %i",
			intStats[STS_AREAS]));
	
	reportView->writeSubHeading("Gameplay statistics");
	reportView->writeLine("\tTotal\tAverage\tMin\tMax");
	reportView->writeText("Enemies\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_ENEMIES]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_ENEMIES_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_ENEMIES_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_ENEMIES_MAX]));
	reportView->writeText("Enemy HP\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_TOTALHP]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_TOTALHP_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_TOTALHP_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_TOTALHP_MAX]));
	reportView->writeText("Ammo ratio\t\t");
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_AMMORAT_AVG]));
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_AMMORAT_MIN]));
	reportView->writeLine(wxString::Format("%.2f", floatStats[STS_AMMORAT_MAX]));
	reportView->writeText("Health ratio\t\t");
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_HEALTHRAT_AVG]));
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_HEALTHRAT_MIN]));
	reportView->writeLine(wxString::Format("%.2f", floatStats[STS_HEALTHRAT_MAX]));
	reportView->writeText("Armor ratio\t\t");
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_ARMORRAT_AVG]));
	reportView->writeText(wxString::Format("%.2f\t", floatStats[STS_ARMORRAT_MIN]));
	reportView->writeLine(wxString::Format("%.2f", floatStats[STS_ARMORRAT_MAX]));
	reportView->writeText("Enemy spawning\t");
	reportView->writeLine(wxString::Format("%i", intStats[STS_MF_SPAWN]));
	reportView->writeText("Excessive spawning\t");
	reportView->writeLine(wxString::Format("%i", intStats[STS_MF_MORESPAWN]));
	reportView->writeLine(wxString::Format("Maps with gameplay stats: %i",
			intStats[STS_GAMESTATS]));
	
	reportView->writeSubHeading("Ratings");
	reportView->writeLine(wxString::Format("Rated maps: %i",
			intStats[STS_OWNRATED]));
	percent = floatStats[STS_OWNRATING_AVG] / 10.0;
	reportView->writeLine(wxString::Format("Average rating: %.2f", percent));
}
