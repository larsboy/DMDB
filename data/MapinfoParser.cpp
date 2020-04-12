#include "MapinfoParser.h"

MapinfoParser::MapinfoParser()
{
	mapTitles = new map<string, string>();
	music = new vector<wxString>();
	doomEdNums = NULL;
	blockNest = -1;
	actors = 0;
}

MapinfoParser::~MapinfoParser()
{
	delete mapTitles;
	delete music;
	if (doomEdNums != NULL)
		delete doomEdNums;
}

void MapinfoParser::printReport(TextReport* reportView)
{
	reportView->writeHeading("MAPINFO");
	wxArrayInt tabs;
	tabs.Add(400);tabs.Add(600);tabs.Add(800);tabs.Add(1000);
	reportView->setTabs(tabs);

	if (mapTitles->size() > 0) {
		for (map<string,string>::iterator it = mapTitles->begin(); it!=mapTitles->end(); ++it)
			reportView->writeLine(it->first+"\t"+it->second);
	} else {
		reportView->writeLine("No map names found");
	}
	if (actors>0)
		reportView->writeLine(wxString::Format("Actor DoomEdNums: %i", actors));
}

//Copied from DehackedParser
string MapinfoParser::getMapTitle(string mapName)
{
	map<string,string>::iterator it = mapTitles->find(mapName);
	if (it != mapTitles->end())
		return it->second;
	else
		return "";
}

bool MapinfoParser::isMusic(string lumpName)
{
	wxString wxName(lumpName);
	for (int i=0; i<music->size(); i++) {
		if (wxName.IsSameAs(music->at(i), false))
			return true;
	}
	return false;
}

map<string, uint16_t>* MapinfoParser::exportDoomEdNums()
{
	map<string, uint16_t>* temp = doomEdNums;
	doomEdNums = NULL;
	return temp;
}

void MapinfoParser::processLine(wxString& line)
{
	line.Trim(); line.Trim(false);
	//wxLogVerbose(line);
	wxString rest;
	if (line.StartsWith("map ", &rest)) {
		//map map01 "Orbital Deployment"
		int i=rest.Find(' ');
		if (i!=wxNOT_FOUND) {
			wxString mapName=rest.SubString(0,i);
			mapName.Trim().MakeUpper();
			i=rest.Find('\"');
			int j=rest.Find('\"', true);
			if (i!=wxNOT_FOUND && j!=wxNOT_FOUND) {
				wxString mapTitle=rest.SubString(i+1,j-1);
				(*mapTitles)[mapName.ToStdString()] = mapTitle.ToStdString();
			}
		}
	} else if (line.StartsWith("music ", &rest)) {
		//music mod01
		//music "mod01"
		rest.Trim();
		int i=rest.Find('\"');
		int j=rest.Find('\"', true);
		if (i!=wxNOT_FOUND && j!=wxNOT_FOUND)
			music->push_back(rest.SubString(i+1,j-1));
		else
			music->push_back(rest);
	} else if (line.StartsWith("DoomEdNums")) {
		if (doomEdNums == NULL)
			doomEdNums = new map<string, uint16_t>();
		blockNest=0;
	} else if (line.StartsWith("{") && blockNest==0) {
		//Entering DoomEdNums
		blockNest=1;
	} else if (line.StartsWith("}") && blockNest==1) {
		//Done with DoomEdNums
		blockNest=-1;
	} else if (blockNest==1) {
		//doomednum = actorName
		int i=line.Find('=');
		if (i==wxNOT_FOUND) return;
		wxString str = line.SubString(0, i-1);
		str.Trim();
		long num;
		if (!str.ToLong(&num)) return;
		int endd = line.Find("//");
		if (endd == wxNOT_FOUND)
			endd = line.Len();
		else
			endd--;
		str = line.SubString(i+1, endd);
		str.Trim(false);
		//wxLogVerbose("Found Actor %s with DoomEdNum %i", str, num);
		(*doomEdNums)[str.MakeLower().ToStdString()] = num;
		actors++;
	}
}
