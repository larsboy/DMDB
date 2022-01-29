#include "ThingDef.h"

//**********************************************************
//************************ ThingDef ************************
//**********************************************************

ThingDef::ThingDef(string n)
: id(0), name(n), dehacked(0), cat(THING_UNKNOWN),
spawner(false), hp(0), health(0), ammo(0), armor(0), modified(false)
{}

void ThingDef::copyFrom(ThingDef* other)
{
	id = other->id;
	name = other->name;
	dehacked = other->dehacked;
	cat = other->cat;
	spawner = other->spawner;
	hp = other->hp;
	health = other->health;
	ammo = other->ammo;
	armor = other->armor;
}

//**************************************************************
//************************ ThingDefList ************************
//**************************************************************

ThingDefList::ThingDefList()
: thingDefs(NULL), thingMap(NULL)
{
}

ThingDefList::ThingDefList(vector<ThingDef*>* things)
{
	thingDefs = things;
	thingMap = NULL;
}

ThingDefList::~ThingDefList()
{
	deleteContent();
}

void ThingDefList::deleteContent()
{
	if (thingMap!=NULL) {
		delete thingMap;
		thingMap=NULL;
	}
	if (thingDefs!=NULL) {
		for (int i=0; i<thingDefs->size(); i++)
			delete thingDefs->at(i);
		delete thingDefs;
		thingDefs=NULL;
	}
}

bool ThingDefList::hasModifiedDefs()
{
	if (thingDefs != NULL) {
		const int item_amount = thingDefs->size();
		for (int n=0; n<item_amount; n++) {
			if (thingDefs->at(n)->modified)
				return true;
		}
	}
	return false;
}

void ThingDefList::setModified(bool modified)
{
	if (thingDefs != NULL) {
		const int item_amount = thingDefs->size();
		for (int n=0; n<item_amount; n++) {
			thingDefs->at(n)->modified = modified;
		}
	}
}

void ThingDefList::loadDefs(wxString fileName)
{
	deleteContent();

	wxFile file(fileName, wxFile::read);
	if (!file.IsOpened()) throw GuiError("Couldn't open file.");
	int fileSize = file.Length();
	wxLogVerbose("Loading ThingDefs from file %s of size %i", fileName, fileSize);
	wxFileInputStream is(file);
	wxBufferedInputStream bis(is, 1024);

	thingDefs = new vector<ThingDef*>();
	ThingDef* td;
	char ch;
	char catByte;
	uint16_t id;
	while (!bis.Eof()) {
		bis.Read(&id, 2);
		if (bis.Eof()) break;
		td = new ThingDef("");
		td->id = id;
		//Read name as zero-terminated string
		do {
			ch = bis.GetC();
			if (ch != 0)
				td->name += ch;

		} while (ch != 0);
		bis.Read(&(td->dehacked), 2);
		catByte = bis.GetC();
		td->cat = ThingCat(catByte);
		bis.Read(&(td->spawner), 1);
		bis.Read(&(td->hp), 4);
		bis.Read(&(td->health), 4);
		bis.Read(&(td->ammo), 4);
		bis.Read(&(td->armor), 4);
		thingDefs->push_back(td);
	}
	//file.Close();
	wxLogVerbose("Loaded %i ThingDefs from file", thingDefs->size());
}

void ThingDefList::saveDefs(wxString fileName)
{
	const int item_amount = thingDefs->size();
	wxFile file(fileName, wxFile::write);
	if (!file.IsOpened()) throw GuiError("Couldn't open file.");
	wxLogVerbose("Saving %i ThingDefs to file %s", item_amount, fileName);
	wxFileOutputStream os(file);
	wxBufferedOutputStream bos(os, 1024);

	ThingDef* td;
	const char* ch;
	char catByte;
	for (int n=0; n<item_amount; n++) {
		td = thingDefs->at(n);
		bos.Write(&(td->id), 2);
		//Write name as zero-terminated string
		ch = td->name.c_str();
		bos.Write(ch, td->name.length()+1);
		bos.Write(&(td->dehacked), 2);
		catByte = td->cat;
		bos.Write(&catByte, 1);
		bos.Write(&(td->spawner), 1);
		bos.Write(&(td->hp), 4);
		bos.Write(&(td->health), 4);
		bos.Write(&(td->ammo), 4);
		bos.Write(&(td->armor), 4);
	}
	bos.Close();
	wxLogVerbose("Finished writing file");
}

map<int, ThingDef*>* ThingDefList::getMapPointer()
{
	if (thingMap == NULL) {
		thingMap = new map<int, ThingDef*>();
		if (thingDefs != NULL) {
			ThingDef* td;
			for (int i=0; i<thingDefs->size(); i++) {
				td = thingDefs->at(i);
				(*thingMap)[td->id] = td;
			}
		}
	}
	return thingMap;
}

vector<ThingDef*>* ThingDefList::exportVector()
{
	if (thingDefs==NULL)
		thingDefs = new vector<ThingDef*>();
	return thingDefs;
}

void ThingDefList::importVector(vector<ThingDef*>* thingVec)
{
	if (thingMap!=NULL) {
		delete thingMap;
		thingMap=NULL;
	}
	if (thingDefs!=NULL)
		delete thingDefs;
	thingDefs = thingVec;
}

list<ThingDef*>* ThingDefList::exportToList()
{
	list<ThingDef*>* thingList = new list<ThingDef*>();
	const int item_amount = thingDefs->size();
	for (int n=0; n<item_amount; n++) {
		thingList->push_back(thingDefs->at(n));
	}
	return thingList;
}

void ThingDefList::importFromList(list<ThingDef*>* thingList)
{
	if (thingMap!=NULL) {
		delete thingMap;
		thingMap=NULL;
	}
	if (thingDefs!=NULL)
		delete thingDefs;
	thingDefs = new vector<ThingDef*>();
	for (list<ThingDef*>::iterator it=thingList->begin(); it != thingList->end(); ++it) {
		thingDefs->push_back(*it);
	}
}
