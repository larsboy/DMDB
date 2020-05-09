#include "WadStatistics.h"

WadStatistics::WadStatistics(wxString name)
: heading(name)
{
	for (int i=0; i<WST_MAPS_AVG; i++)
		intStats[i] = 0;
	setIntMins(INVALID_MIN_INT);

	for (int i=0; i<WST_END; i++)
		floatStats[i] = 0.0;
	//setFloatMins(INVALID_MIN_FLOAT);
}

WadStatistics::~WadStatistics()
{
}

void WadStatistics::setIntMins(unsigned long minValue)
{
	intStats[WST_YEAR_MIN] = minValue;
	intStats[WST_MAPS_MIN] = minValue;
}

void WadStatistics::processWad(WadEntry* wadEntry)
{
	intStats[WST_COUNT]++;
	if (wadEntry->year > 0) {
		if (wadEntry->year < intStats[WST_YEAR_MIN])
			intStats[WST_YEAR_MIN] = wadEntry->year;
		if (wadEntry->year > intStats[WST_YEAR_MAX])
			intStats[WST_YEAR_MAX] = wadEntry->year;
	}
	if (wadEntry->flags&WF_IWAD) intStats[WST_WF_IWAD]++;
	if (wadEntry->flags&WF_SPRITES) intStats[WST_WF_SPRITES]++;
	if (wadEntry->flags&WF_TEX) intStats[WST_WF_TEX]++;
	if (wadEntry->flags&WF_GFX) intStats[WST_WF_GFX]++;
	if (wadEntry->flags&WF_COLOR) intStats[WST_WF_COLOR]++;
	if (wadEntry->flags&WF_SOUND) intStats[WST_WF_SOUND]++;
	if (wadEntry->flags&WF_MUSIC) intStats[WST_WF_MUSIC]++;
	if (wadEntry->flags&WF_DEHBEX) intStats[WST_WF_DEHBEX]++;
	if (wadEntry->flags&WF_THINGS) intStats[WST_WF_THINGS]++;
	if (wadEntry->flags&WF_SCRIPT) intStats[WST_WF_SCRIPT]++;
	if (wadEntry->flags&WF_GLNODES) intStats[WST_WF_GLNODES]++;
	if (wadEntry->ownFlags&OF_HAVEFILE) intStats[WST_OF_HAVEFILE]++;

	intStats[WST_MAPS]+=wadEntry->numberOfMaps;
	if (wadEntry->numberOfMaps < intStats[WST_MAPS_MIN])
		intStats[WST_MAPS_MIN] = wadEntry->numberOfMaps;
	if (wadEntry->numberOfMaps > intStats[WST_MAPS_MAX])
		intStats[WST_MAPS_MAX] = wadEntry->numberOfMaps;

	if (wadEntry->isOwnRated()) {
		intStats[WST_OWNRATED]++;
		intStats[WST_OWNRATING] += wadEntry->ownRating;
	}
}

void WadStatistics::computeResults()
{
	if (intStats[WST_COUNT] > 0) {
		floatStats[WST_MAPS_AVG] = (double)intStats[WST_MAPS]/(double)intStats[WST_COUNT];
	} else {
		intStats[WST_MAPS_MIN] = 0;
	}
	if (intStats[WST_YEAR_MIN]==INVALID_MIN_INT)
		intStats[WST_YEAR_MIN] = 0;

	if (intStats[WST_OWNRATED] > 0)
		floatStats[WST_OWNRATING_AVG] = (double)intStats[WST_OWNRATING]/(double)intStats[WST_OWNRATED];
}

void WadStatistics::printReport(TextReport* reportView)
{
	reportView->writeHeading(heading);
	reportView->writeLine(wxString::Format("Total wads: %u", intStats[WST_COUNT]));
	reportView->writeLine(wxString::Format("Year range: %i - %i", intStats[WST_YEAR_MIN], intStats[WST_YEAR_MAX]));

	reportView->writeSubHeading("Counts");
	wxArrayInt tabs;
	tabs.Add(450);tabs.Add(650);tabs.Add(850);tabs.Add(1050);
	reportView->setTabs(tabs);
	reportView->writeText("Have file\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_OF_HAVEFILE]));
	double percent = (double)(intStats[WST_OF_HAVEFILE]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("In IWAD\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_IWAD]));
	percent = (double)(intStats[WST_WF_IWAD]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sprites\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_SPRITES]));
	percent = (double)(intStats[WST_WF_SPRITES]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Textures\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_TEX]));
	percent = (double)(intStats[WST_WF_TEX]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Other graphics\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_GFX]));
	percent = (double)(intStats[WST_WF_GFX]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Palette\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_COLOR]));
	percent = (double)(intStats[WST_WF_COLOR]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sounds\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_SOUND]));
	percent = (double)(intStats[WST_WF_SOUND]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Music\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_MUSIC]));
	percent = (double)(intStats[WST_WF_MUSIC]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Dehacked/BEX\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_DEHBEX]));
	percent = (double)(intStats[WST_WF_DEHBEX]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Monsters/items\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_THINGS]));
	percent = (double)(intStats[WST_WF_THINGS]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Scripts\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_SCRIPT]));
	percent = (double)(intStats[WST_WF_SCRIPT]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("GL nodes\t");
	reportView->writeText(wxString::Format("%i\t", intStats[WST_WF_GLNODES]));
	percent = (double)(intStats[WST_WF_GLNODES]*100) / (double)intStats[WST_COUNT];
	reportView->writeLine(wxString::Format("%.1f%", percent));

	reportView->writeSubHeading("Maps");
	reportView->writeText("Total\t");
	reportView->writeLine(wxString::Format("%i", intStats[WST_MAPS]));
	reportView->writeText("Average\t");
	reportView->writeLine(wxString::Format("%.1f", floatStats[WST_MAPS_AVG]));
	reportView->writeText("Min\t");
	reportView->writeLine(wxString::Format("%i", intStats[WST_MAPS_MIN]));
	reportView->writeText("Max\t");
	reportView->writeLine(wxString::Format("%i", intStats[WST_MAPS_MAX]));

	reportView->writeSubHeading("Ratings");
	reportView->writeLine(wxString::Format("Rated maps: %i",
			intStats[WST_OWNRATED]));
	percent = floatStats[WST_OWNRATING_AVG] / 10.0;
	reportView->writeLine(wxString::Format("Average rating: %.2f", percent));
}
