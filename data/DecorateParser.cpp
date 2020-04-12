#include "DecorateParser.h"

//*******************************************************
//************************ Actor ************************
//*******************************************************

Actor::Actor(ThingDef* td)
: pName(""), replaces(""), drop("")
{
	thingDef = td;
	parent = NULL;
}

Actor::Actor(string name, bool modified)
: pName(""), replaces(""), drop("")
{
	thingDef = new ThingDef(name);
	thingDef->modified = modified;
	parent = NULL;
}

bool Actor::hasParentName(string name)
{
	if (thingDef->name.compare(name) == 0)
		return true;
	if (pName.compare(name) == 0)
		return true;
	if (parent != NULL)
		return parent->hasParentName(name);
	else
		return false;
}

void Actor::replaceThing(ThingDef* td)
{
	thingDef->name = td->name;
	thingDef->cat = td->cat;
	thingDef->spawner = td->spawner;
	thingDef->hp = td->hp;
	thingDef->health = td->health;
	thingDef->ammo = td->ammo;
	thingDef->armor = td->armor;
}

void Actor::completeThing(ThingDef* td)
{
	if (td->cat==THING_UNKNOWN)
		td->cat = thingDef->cat;
	if (td->hp==0)
		td->hp = thingDef->hp;
	if (td->health==0)
		td->health = thingDef->health;
	if (td->ammo==0)
		td->ammo = thingDef->ammo;
	if (td->armor==0)
		td->armor = thingDef->armor;
	if (parent != NULL)
		parent->completeThing(thingDef);
}

void Actor::completeSelf()
{
    if (parent != NULL) {
		// Fill in thingDef with parent properties
		parent->completeThing(thingDef);
    }
    if (hasParentName("ammo")) {
		thingDef->cat = THING_PICKUP;
		thingDef->ammo = thingDef->health;
		thingDef->health = 0;
	} else if (hasParentName("weapon")) {
		thingDef->cat = THING_WEAPON;
	}
    // Default category DECORATION
    if (thingDef->cat == THING_UNKNOWN)
		thingDef->cat = THING_DECORATION;
}

void Actor::addThingCounts(ThingDef* td)
{
	thingDef->health += td->health;
	thingDef->ammo += td->ammo;
	thingDef->armor += td->armor;
}


//****************************************************************
//************************ DecorateParser ************************
//****************************************************************

DecorateParser::DecorateParser()
: actorNums(NULL), masterList(NULL), blockNest(0), currentThing(NULL),
actors(NULL), actorsThing(0), actorsOther(0), actorsReplace(0)
{
}

DecorateParser::~DecorateParser()
{
	if (actors!=NULL) delete actors;
	if (actorNums!=NULL) delete actorNums;
}

void DecorateParser::printReport(TextReport* reportView)
{
	reportView->writeHeading("DECORATE");
	wxArrayInt tabs;
	tabs.Add(600);tabs.Add(800);tabs.Add(1000);
	reportView->setTabs(tabs);
	reportView->writeLine(wxString::Format("Actors with doomednum\t%i", actorsThing));
	reportView->writeLine(wxString::Format("Other actors\t%i", actorsOther));
	reportView->writeLine(wxString::Format("Replacement actors\t%i", actorsReplace));
}

void DecorateParser::setInitialDefs(ThingDefList* things)
{
	masterList = things;
	vector<ThingDef*>* tv = things->exportVector();
	actors = new vector<Actor*>();
	for (int i=0; i<tv->size(); i++) {
		actors->push_back(new Actor(tv->at(i)));
	}
}

void DecorateParser::setActorNums(map<string, uint16_t>* an)
{
	if (actorNums != NULL)
		delete actorNums;
	actorNums = an;
}

void DecorateParser::parseFile(const wxString& filename, int32_t offset, int32_t size)
{
	if (actors==NULL) actors = new vector<Actor*>();
	blockNest = 0;
	wxLogVerbose("Parse decorate in file %s", filename);
	TextLumpParser::parseFile(filename, offset, size);
}

void DecorateParser::parseFiles(const wxString &dir)
{
	if (actors==NULL) actors = new vector<Actor*>();
	wxDir wxd(dir);
	if ( !wxd.IsOpened() ) {
		wxLogVerbose("Could not access folder!");
		return;
	}
	wxString filename;
	bool cont = wxd.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
	while (cont)
	{
		wxString wxs = wxd.GetNameWithSep() + filename;
		parseFile(wxs.c_str(), 0, -1);
		cont = wxd.GetNext(&filename);
	}
}

ThingDefList* DecorateParser::extractThingDefs()
{
	//Process Actors:
	wxLogVerbose("Extract ThingDefs from %i Actors", actors->size());
	findParents();
	//wxLogVerbose("Complete Actor contents");
	for (int i=0; i<actors->size(); i++)
		actors->at(i)->completeSelf();
	//wxLogVerbose("Add counts from dropped items");
	findDrops();
	replaceActors();

	wxLogVerbose("Sorting things in map");
	map<int, ThingDef*>* sortedThings = new map<int, ThingDef*>();
	ThingDef* td;
	for (int i=0; i<actors->size(); i++) {
		td = actors->at(i)->thingDef;
		if (td->id != 0) {
			(*sortedThings)[td->id] = td;
			if (td->modified) actorsThing++;
		} else {
			if (td->modified) actorsOther++;
			delete td;
		}
		delete actors->at(i);
	}
	delete actors;
	actors = NULL;

	wxLogVerbose("Add basic things if missing");
	// Add basic things, if not found
	if (sortedThings->find(1) == sortedThings->end()) {
		td = new ThingDef("player1start");
		td->id = 1;
		td->cat = THING_PLAYER1;
		(*sortedThings)[td->id] = td;
	}
	if (sortedThings->find(2) == sortedThings->end()) {
		td = new ThingDef("player2start");
		td->id = 2;
		td->cat = THING_COOP;
		(*sortedThings)[td->id] = td;
	}
	if (sortedThings->find(3) == sortedThings->end()) {
		td = new ThingDef("player3start");
		td->id = 3;
		td->cat = THING_COOP;
		(*sortedThings)[td->id] = td;
	}
	if (sortedThings->find(4) == sortedThings->end()) {
		td = new ThingDef("player4start");
		td->id = 4;
		td->cat = THING_COOP;
		(*sortedThings)[td->id] = td;
	}
	if (sortedThings->find(11) == sortedThings->end()) {
		td = new ThingDef("dmstart");
		td->id = 11;
		td->cat = THING_DM;
		(*sortedThings)[td->id] = td;
	}
	if (sortedThings->find(14) == sortedThings->end()) {
		td = new ThingDef("teleportdest");
		td->id = 14;
		td->cat = THING_EFFECT;
		(*sortedThings)[td->id] = td;
	}
	vector<ThingDef*>* result = new vector<ThingDef*>();
	for (map<int,ThingDef*>::iterator it=sortedThings->begin(); it!=sortedThings->end(); ++it)
		result->push_back(it->second);
	delete sortedThings;
	if (masterList != NULL) {
		masterList->importVector(result);
		return masterList;
	} else {
		return new ThingDefList(result);
	}
}

void DecorateParser::processLine(wxString& line)
{
	line.Trim(false);
	if (line.IsEmpty())
		return;
	//wxLogVerbose("Process line: %s", line);
	if (line.StartsWith("//") || line.StartsWith("/*") || line.StartsWith("*/")) //Comment
		return;
	if (line.StartsWith("{")) {
		blockNest++;
		return;
	}
	if (line.StartsWith("}")) {
		blockNest--;
		return;
	}
	if (line.StartsWith('#')) //#include?
		return;
	line.MakeLower();
	if (line.StartsWith("actor")) {
		currentThing = processActor(line.Append(' '));
		return;
	}
	if (currentThing != NULL) {
		//Properties of currentThing
		if (line.StartsWith("doomednum")) {
			currentThing->id = processNumber(line, 10);
			return;
		}
		if (line.StartsWith("health")) {
			currentThing->hp = processNumber(line, 7) / 10;
			return;
		}
		if (line.StartsWith("monster") || line.StartsWith("+ismonster") || line.StartsWith("+countkill")) {
			currentThing->cat = THING_MONSTER;
			return;
		}
		if (line.StartsWith("inventory.amount")) {
			currentThing->cat = THING_PICKUP; //Could be ammo or health
			currentThing->health = processNumber(line, 17);
			//Moved to ammo by completeSelf if has parent "ammo"
			return;
		}
		if (line.StartsWith("armor.saveamount")) {
			currentThing->armor = processNumber(line, 17);
			return;
		}
		if (line.StartsWith("weapon.ammogive")) {
			currentThing->ammo = processNumber(line, 16);
			return;
		}
		if (line.StartsWith("inventory")) {
			currentThing->cat = THING_PICKUP;
			//completeSelf overrides this for weapons
			return;
		}
		if (line.StartsWith("weapon.")) {
			currentThing->cat = THING_WEAPON;
			return;
		}
		if (line.StartsWith("skip_super")) { //Cancel parent property inheritance
			actors->back()->pName = "";
			return;
		}
		if (line.StartsWith("dropitem")) {
			actors->back()->drop = processQuoted(line, 8);
			return;
		}
	}
	if (blockNest == 0) {
		//Check for old decorate format:
		//classname | projectile classname | breakable classname | pickup classname
		if (line.StartsWith("pickup"))
			currentThing = processOldActor(line, 7, THING_PICKUP);
		else if (line.StartsWith("projectile"))
			currentThing = processOldActor(line, 11, THING_UNKNOWN);
		else if (line.StartsWith("breakable"))
			currentThing = processOldActor(line, 10, THING_DECORATION);
		else
			currentThing = processOldActor(line, 0, THING_DECORATION);
	}
}

ThingDef* DecorateParser::processActor(wxString& line)
{
	//actor classname [ : parentclassname] [replaces replaceclassname] [doomednum]
	int ipar = line.Find(':');
	int irep = line.Find("replaces");
	int i = 6;
	while (line.GetChar(i)==' ' || line.GetChar(i)=='\t')
		i++;
	int spc = line.find(' ', i);
	//if (spc == string::npos)
	//	spc = line.Len();
	if (ipar!=wxNOT_FOUND && ipar<spc)
		spc = ipar;
	Actor* result = new Actor(line.SubString(i, spc-1).ToStdString(), true);
	if (ipar != wxNOT_FOUND) {
		i = ipar+1;
		while (line.GetChar(i)==' ' || line.GetChar(i)=='\t')
			i++;
		spc = line.find(' ', i);
		result->pName = line.SubString(i, spc-1).ToStdString();
		//ThingCat:
		if (result->pName.compare("specialspot") == 0)
			result->thingDef->cat = THING_EFFECT;
	}
	if (irep != wxNOT_FOUND) {
		i = irep+9;
		while (line.GetChar(i)==' ' || line.GetChar(i)=='\t')
			i++;
		spc = line.find(' ', i);
		wxString temp = line.SubString(i, spc-1).Trim(true);
		//wxLogVerbose("replaces %s found in actor def to pos %i", temp, spc);
		result->replaces = temp.ToStdString();
	}
	result->thingDef->id = processNumber(line, spc+1);
	//wxLogVerbose("Found Actor %s with id %i", result->thingDef->name, result->thingDef->id);
	if ((result->thingDef->id==0) && (actorNums!=NULL)) {
		map<string,uint16_t>::iterator it = actorNums->find(result->thingDef->name);
		if (it != actorNums->end())
			result->thingDef->id = it->second;
	}
	actors->push_back(result);
	return result->thingDef;
}

ThingDef* DecorateParser::processOldActor(wxString& line, int start, ThingCat cat)
{
	int i = start;
	while (line.GetChar(i)==' ' || line.GetChar(i)=='\t')
		i++;
	int spc = line.find(' ', i);
	if (spc == string::npos)
		spc = line.Len();
	Actor* result = new Actor(line.SubString(i, spc-1).ToStdString(), true);
	result->thingDef->cat = cat;
	actors->push_back(result);
	return result->thingDef;
}

uint32_t DecorateParser::processNumber(wxString& line, int start)
{
	if (start >= line.Len())
		return 0;
	int endd = line.Find("//");
	if (endd == wxNOT_FOUND)
		endd = line.Len();
	else
		endd--;
	wxString str = line.SubString(start, endd);
	str.Trim();
	long result;
	if (str.ToLong(&result)) {
		if (result < 0) return 0;
		else return result;
	} else {
		return 0;
	}
}

string DecorateParser::processQuoted(wxString& line, int start)
{
	if (start >= line.Len())
		return "";
	int i1 = line.find('\"', start);
	int i2 = line.find('\"', i1+1);
	if (i1==string::npos || i2==string::npos)
		return "";
	return line.SubString(i1+1, i2-1).ToStdString();
}

Actor* DecorateParser::findActor(string name)
{
	wxString wxName(name);
	Actor* a;
	for (int i=0; i<actors->size(); i++) {
		a = actors->at(i);
		if (wxName.CmpNoCase(a->thingDef->name) == 0)
		//if (a->thingDef->name.compare(name) == 0)
			return a;
	}
	return NULL;
}

void DecorateParser::findParents()
{
	Actor* a;
	for (int i=0; i<actors->size(); i++) {
		a = actors->at(i);
		if (a->pName.size() > 0) {
			a->parent = findActor(a->pName);
			//if (a->parent==NULL) wxLogVerbose("Didn't find parent Actor %s", a->pName);
			//else wxLogVerbose("Found parent Actor %s", a->pName);
		}
	}
}

void DecorateParser::findDrops()
{
	//Add health/ammo/armor from drops
	Actor* a;
	Actor* d;
	for (int i=0; i<actors->size(); i++) {
		a = actors->at(i);
		if (a->drop.size() > 0) {
			d = findActor(a->drop);
			if (d != NULL)
				a->addThingCounts(d->thingDef);
		}
	}
}

void DecorateParser::replaceActors()
{
	Actor* a;
	Actor* r;
	for (int i=0; i<actors->size(); i++) {
		a = actors->at(i);
		if (a->replaces.size() > 0) {
			r = findActor(a->replaces);
			if (r != NULL) {
				//wxLogVerbose("Found Actor %s to be replaced", a->replaces);
				//r is replaced by a
				//We give r the ThingDef properties of a
				r->replaceThing(a->thingDef);
				//a is counted as a replacer, and the modified-flag
				//is cleared so it is not counted again.
				//It should not have an id
				actorsReplace++;
				a->thingDef->modified = false;
			} /*else {
				wxLogVerbose("Did not find Actor %s to be replaced", a->replaces);
			}*/
		}
	}
}
