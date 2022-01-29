#include "DehackedParser.h"

DehackedParser::DehackedParser(ThingDefList* things, wxString fileNam)
: fileName(fileNam)
{
	thingDefs = things;
	changedDefs = new vector<ThingDef*>();
	mapTitles = new map<string, string>();
}

DehackedParser::~DehackedParser()
{
	delete changedDefs;
	delete mapTitles;
}

void DehackedParser::parseFile(const wxString& filename, int32_t offset, int32_t size)
{
	wxLogVerbose("Parsing Dehacked from file %s", filename);
	for (int i=0; i<DEH_END; i++)
		typeCounts[i] = 0;
	currentEntry = DEH_END;

	TextLumpParser::parseFile(filename,offset,size);
}

void DehackedParser::printReport(TextReport* reportView)
{
	if (fileName.Len() == 0)
		reportView->writeHeading("Dehacked patch");
	else
		reportView->writeHeading(fileName);
	reportView->writeSubHeading("Dehacked content");
	wxArrayInt tabs;
	tabs.Add(400);tabs.Add(600);tabs.Add(800);tabs.Add(1000);
	reportView->setTabs(tabs);
	reportView->writeLine(wxString::Format("Thing entries\t%i", typeCounts[DEH_THING]));
	reportView->writeLine(wxString::Format("Sound entries\t%i", typeCounts[DEH_SOUND]));
	reportView->writeLine(wxString::Format("Frame entries\t%i", typeCounts[DEH_FRAME]));
	reportView->writeLine(wxString::Format("Pointer entries\t%i", typeCounts[DEH_POINTER]));
	reportView->writeLine(wxString::Format("Sprite entries\t%i", typeCounts[DEH_SPRITE]));
	reportView->writeLine(wxString::Format("Ammo entries\t%i", typeCounts[DEH_AMMO]));
	reportView->writeLine(wxString::Format("Weapon entries\t%i", typeCounts[DEH_WEAPON]));
	reportView->writeLine(wxString::Format("Text entries\t%i", typeCounts[DEH_TEXT]));

	if (changedDefs->size() > 0) {
		reportView->writeSubHeading("Modified hit-points");
		ThingDef* td;
		for (int i=0; i<changedDefs->size(); i++) {
			td = changedDefs->at(i);
			reportView->writeLine(td->name+"\t"+wxString::Format("%i",td->hp));
		}
	}
	if (mapTitles->size() > 0) {
		reportView->writeSubHeading("Map titles");
		for (map<string,string>::iterator it = mapTitles->begin(); it!=mapTitles->end(); ++it)
			reportView->writeLine(it->first+"\t"+it->second);
	}
}

string DehackedParser::getMapTitle(string mapName)
{
	map<string,string>::iterator it = mapTitles->find(mapName);
	if (it != mapTitles->end())
		return it->second;
	else
		return "";
}

void DehackedParser::processLine(wxString& line)
{
	line.Trim();
	//wxLogVerbose(line);
	wxString rest;
	if (currentEntry == DEH_END) { //Not currently in an entry
		if (line.StartsWith("Thing ", &rest)) {
			currentEntry=DEH_THING;
			currentThing=NULL;
			int i=rest.Find(' ');
			if (i!=wxNOT_FOUND) {
				wxString num=rest.SubString(0,i);
				long dehNum=0;
				num.ToCLong(&dehNum);
				findCurrentThing(dehNum);
			}
		} else if (line.StartsWith("Sound")) {
			currentEntry=DEH_SOUND;
			//Just counted
		} else if (line.StartsWith("Frame")) {
			currentEntry=DEH_FRAME;
			//Just counted
		} else if (line.StartsWith("Pointer")) {
			currentEntry=DEH_POINTER;
			//Just counted
		} else if (line.StartsWith("Sprite")) {
			currentEntry=DEH_SPRITE;
			//Just counted
		} else if (line.StartsWith("Ammo")) {
			currentEntry=DEH_AMMO;
			//Just counted
		} else if (line.StartsWith("Weapon")) {
			currentEntry=DEH_WEAPON;
			//Just counted
		} else if (line.StartsWith("Text ", &rest)) {
			currentEntry=DEH_TEXT;
			textIndex = 0;
			int i=rest.Find(' ');
			if (i!=wxNOT_FOUND) {
				wxString num=rest.SubString(0,i);
				num.ToCLong(&textIndex);
			}
		} else if (line.StartsWith("HUSTR_", &rest)) {
			//BEX-style string
			typeCounts[DEH_TEXT]++;
			int i=rest.Find(' ');
			if (i!=wxNOT_FOUND) {
				wxString mapNum=rest.SubString(0,i-1);
				wxString mapTitle = rest.SubString(i+1, line.Length()-1);
				mapTitle.Trim(); mapTitle.Trim(false);
				if (mapTitle.StartsWith("=")) {
					mapTitle = mapTitle.SubString(1, line.Length()-1);
					mapTitle.Trim(false);
				}
				//TODO: Can be inside quotes "..."
				if (mapNum.Len()==1 || mapNum.Len()==2) {
					//MAP##
					string mapName = "MAP";
					if (mapNum.Len()==1)
						mapName.append("0");
					mapName.append(mapNum.ToStdString());
					(*mapTitles)[mapName] = mapTitle.ToStdString();

				} else if (mapNum.Matches("E?M?")) {
					(*mapTitles)[mapNum.ToStdString()] = mapTitle.ToStdString();
				}
			}
		}
		if (currentEntry<DEH_END) {
			typeCounts[currentEntry]++;
		}
	} else if (line.Length() == 0) {
		currentEntry=DEH_END;
	} else if ((currentEntry == DEH_THING) && (currentThing!=NULL)) {
		//Check for thing stuff
		if (line.StartsWith("Hit points = ", &rest)) {
			long dehNum=0;
			rest.ToCLong(&dehNum);
			//wxLogVerbose("Hit points: %i", dehNum);
			currentThing->hp = dehNum/10;
			currentThing->modified = true;
			changedDefs->push_back(currentThing);
		}
		//What else can be changed by dehacked?
		//ThingCat, pickup:health,ammo,armor?
	} else if ((currentEntry==DEH_TEXT) && (textIndex>0)) {
		//Check if map name
		if (line.StartsWith("level ", &rest)) {
			int i=rest.Find(':');
			if (i!=wxNOT_FOUND) {
				wxString num=rest.SubString(0,i);
				long dehNum=0;
				num.ToCLong(&dehNum);
				wxString mapTitle = line.SubString(textIndex, line.Length()-1);
				string mapName = "MAP";
				if (dehNum < 10)
					mapName.append("0");
				mapName.append(LtbUtils::intToString(dehNum));
				(*mapTitles)[mapName] = mapTitle.ToStdString();
				//wxLogVerbose("Text for MAP %i: %s", dehNum, mapTitle);
			}
		} else if (line.Matches("E?M?:*")) {
			wxString mapTitle = line.SubString(textIndex, line.Length()-1);
			string mapName = line.Left(4).ToStdString();
			(*mapTitles)[mapName] = mapTitle.ToStdString();
		}
	}
}

void DehackedParser::findCurrentThing(int num)
{
	//wxLogVerbose("Thing #: %i", num);
	vector<ThingDef*>* tv = thingDefs->exportVector();
	for (int i=0; i<tv->size(); i++) {
		currentThing = tv->at(i);
		if (currentThing->dehacked == num)
			return;
	}
	currentThing = NULL;
}
