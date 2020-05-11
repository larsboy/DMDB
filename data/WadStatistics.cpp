#include "WadStatistics.h"

WadStatistics::WadStatistics(wxString name)
: DBStatistics(name)
{
}

WadStatistics::~WadStatistics()
{
}

void WadStatistics::processWad(WadEntry* wadEntry)
{
	intStats[STS_WADS]++;
	if (wadEntry->year > 0) {
		if (wadEntry->year < intStats[STS_YEAR_MIN])
			intStats[STS_YEAR_MIN] = wadEntry->year;
		if (wadEntry->year > intStats[STS_YEAR_MAX])
			intStats[STS_YEAR_MAX] = wadEntry->year;
	}
	floatStats[STS_SIZE] += wadEntry->fileSize;
	if (wadEntry->fileSize < intStats[STS_SIZE_MIN])
		intStats[STS_SIZE_MIN] = wadEntry->fileSize;
	if (wadEntry->fileSize > intStats[STS_SIZE_MAX])
		intStats[STS_SIZE_MAX] = wadEntry->fileSize;

	if (wadEntry->flags&WF_IWAD) intStats[STS_WF_IWAD]++;
	if (wadEntry->flags&WF_SPRITES) intStats[STS_WF_SPRITES]++;
	if (wadEntry->flags&WF_TEX) intStats[STS_WF_TEX]++;
	if (wadEntry->flags&WF_GFX) intStats[STS_WF_GFX]++;
	if (wadEntry->flags&WF_COLOR) intStats[STS_WF_COLOR]++;
	if (wadEntry->flags&WF_SOUND) intStats[STS_WF_SOUND]++;
	if (wadEntry->flags&WF_MUSIC) intStats[STS_WF_MUSIC]++;
	if (wadEntry->flags&WF_DEHBEX) intStats[STS_WF_DEHBEX]++;
	if (wadEntry->flags&WF_THINGS) intStats[STS_WF_THINGS]++;
	if (wadEntry->flags&WF_SCRIPT) intStats[STS_WF_SCRIPT]++;
	if (wadEntry->flags&WF_GLNODES) intStats[STS_WF_GLNODES]++;
	if (wadEntry->ownFlags&OF_HAVEFILE) intStats[STS_OF_HAVEFILE]++;

	intStats[STS_MAPS] += wadEntry->numberOfMaps;
	if (wadEntry->numberOfMaps < intStats[STS_MAPS_MIN])
		intStats[STS_MAPS_MIN] = wadEntry->numberOfMaps;
	if (wadEntry->numberOfMaps > intStats[STS_MAPS_MAX])
		intStats[STS_MAPS_MAX] = wadEntry->numberOfMaps;

	if (wadEntry->isOwnRated()) {
		intStats[STS_OWNRATED]++;
		intStats[STS_OWNRATING] += wadEntry->ownRating;
	}
}

void WadStatistics::computeResults()
{
	if (intStats[STS_WADS] > 0) {
		floatStats[STS_MAPS_AVG] = (double)intStats[STS_MAPS]/(double)intStats[STS_WADS];
		floatStats[STS_SIZE_AVG] = floatStats[STS_SIZE]/(double)intStats[STS_WADS];
	} else {
		intStats[STS_MAPS_MIN] = 0;
		intStats[STS_SIZE_MIN] = 0;
	}
	if (intStats[STS_YEAR_MIN]==INVALID_MIN_INT)
		intStats[STS_YEAR_MIN] = 0;

	if (intStats[STS_OWNRATED] > 0)
		floatStats[STS_OWNRATING_AVG] = (double)intStats[STS_OWNRATING]/(double)intStats[STS_OWNRATED];
}

void WadStatistics::printReport(TextReport* reportView)
{
	reportView->writeHeading(heading);
	reportView->writeLine(wxString::Format("Total wads: %u", intStats[STS_WADS]));
	reportView->writeLine(wxString::Format("Year range: %i - %i", intStats[STS_YEAR_MIN], intStats[STS_YEAR_MAX]));

	reportView->writeSubHeading("Counts");
	wxArrayInt tabs;
	tabs.Add(450);tabs.Add(650);tabs.Add(850);tabs.Add(1050);
	reportView->setTabs(tabs);
	reportView->writeText("Have file\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_OF_HAVEFILE]));
	double percent = (double)(intStats[STS_OF_HAVEFILE]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Is IWAD\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_IWAD]));
	percent = (double)(intStats[STS_WF_IWAD]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sprites\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SPRITES]));
	percent = (double)(intStats[STS_WF_SPRITES]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Textures\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_TEX]));
	percent = (double)(intStats[STS_WF_TEX]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Other graphics\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_GFX]));
	percent = (double)(intStats[STS_WF_GFX]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Palette\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_COLOR]));
	percent = (double)(intStats[STS_WF_COLOR]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Sounds\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SOUND]));
	percent = (double)(intStats[STS_WF_SOUND]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Music\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_MUSIC]));
	percent = (double)(intStats[STS_WF_MUSIC]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Dehacked/BEX\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_DEHBEX]));
	percent = (double)(intStats[STS_WF_DEHBEX]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Monsters/items\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_THINGS]));
	percent = (double)(intStats[STS_WF_THINGS]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("Scripts\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_SCRIPT]));
	percent = (double)(intStats[STS_WF_SCRIPT]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));
	reportView->writeText("GL nodes\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_WF_GLNODES]));
	percent = (double)(intStats[STS_WF_GLNODES]*100) / (double)intStats[STS_WADS];
	reportView->writeLine(wxString::Format("%.1f%", percent));

	reportView->writeSubHeading("Amounts");
	reportView->writeLine("\tTotal\tAverage\tMin\tMax");
	reportView->writeText("Maps\t");
	reportView->writeText(wxString::Format("%i\t", intStats[STS_MAPS]));
	reportView->writeText(wxString::Format("%.1f\t", floatStats[STS_MAPS_AVG]));
	reportView->writeText(wxString::Format("%i\t", intStats[STS_MAPS_MIN]));
	reportView->writeLine(wxString::Format("%i", intStats[STS_MAPS_MAX]));
	reportView->writeText("File size (MB)\t");
	double mb = floatStats[STS_SIZE] / 1048576.0;
	reportView->writeText(wxString::Format("%.1f\t", mb));
	mb = floatStats[STS_SIZE_AVG] / 1048576.0;
	reportView->writeText(wxString::Format("%.3f\t", mb));
	mb = (double)intStats[STS_SIZE_MIN] / 1048576.0;
	reportView->writeText(wxString::Format("%.3f\t", mb));
	mb = (double)intStats[STS_SIZE_MAX] / 1048576.0;
	reportView->writeLine(wxString::Format("%.3f", mb));

	reportView->writeSubHeading("Ratings");
	reportView->writeLine(wxString::Format("Rated wads: %i",
			intStats[STS_OWNRATED]));
	percent = floatStats[STS_OWNRATING_AVG] / 10.0;
	reportView->writeLine(wxString::Format("Average rating: %.2f", percent));
}
