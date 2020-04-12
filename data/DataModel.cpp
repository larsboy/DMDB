#include "DataModel.h"


//*************************************************************
//************************ AuthorEntry ************************
//*************************************************************

AuthorEntry::AuthorEntry(uint32_t id)
: type(0), dbid(id), namef(""), namel(""), alias1(""), alias2(""), modified(0),
textOffset(0)
{}

wxString AuthorEntry::getName()
{
	if (namef.Length()==0 && namel.Length()==0)
		return "";
	else if (namef.Length()==0)
		return namel;
	else if (namel.Length()==0)
		return namef;
	else
		return namef+" "+namel;
}

bool AuthorEntry::findLowerCase(const wxString& sub)
{
	if (namef.Length() >= sub.Length())
	{
		if (namef.Lower().Find(sub) != wxNOT_FOUND) return true;
	}
	if (namel.Length() >= sub.Length())
	{
		if (namel.Lower().Find(sub) != wxNOT_FOUND) return true;
	}
	if (alias1.Length() >= sub.Length())
	{
		if (alias1.Lower().Find(sub) != wxNOT_FOUND) return true;
	}
	if (alias2.Length() >= sub.Length())
	{
		if (alias2.Lower().Find(sub) != wxNOT_FOUND) return true;
	}
	return false;
}

AuthorGroupEntry::AuthorGroupEntry(uint32_t id)
: AuthorEntry(id)
{}

bool AuthorGroupEntry::addEntry(AuthorEntry* entry)
{
	if (type==AUTHORGROUP)
		return false; //full
	for (int i=0; i<type; i++) {
		if (pointers[i] == entry)
			return false;
	}
	pointers[type] = entry;
	type++;
	return true;
}

int AuthorGroupEntry::findEntry(AuthorEntry* entry)
{
	for (int i=0; i<type; i++) {
		if (pointers[i] == entry)
			return i;
	}
	return -1;
}

void AuthorGroupEntry::removeEntry(int index)
{
	if (index>=0 && index<type) {
		for (int i=index+1; i<type; i++)
			pointers[i-1] = pointers[i];
		type--;
		pointers[type] = NULL;
	}
}

//**********************************************************
//************************ WadEntry ************************
//**********************************************************

WadEntry::WadEntry(uint32_t id, int mapSize)
: dbid(id), fileName(""), fileSize(0), extraFiles(""), idGames(0),
title(""), year(0), iwad(0), engine(0), playStyle(0),
mapPointers(mapSize), numberOfMaps(0), flags(0), rating(255),
ownRating(255), ownFlags(0)
{}

string WadEntry::getName() const
{
	if (title.length()==0)
		return fileName;
	else
		return title;
}

void WadEntry::addMap(MapEntry* mapEntry)
{
	if (numberOfMaps < mapPointers.size())
		mapPointers[numberOfMaps++] = mapEntry;
}

MapEntry* WadEntry::getMap(string& mapName)
{
	for (int i=0; i<numberOfMaps; i++) {
		if (mapPointers[i]->name.compare(mapName)==0)
			return mapPointers[i];
	}
	return NULL;
}

void WadEntry::removeMap(int index, bool del)
{
	vector<MapEntry*>::iterator it = mapPointers.begin();
	int i=0;
	while (it!=mapPointers.end() && i<index) {
		it++;
		i++;
	}
	if (it!=mapPointers.end() && i==index) {
		if (del) delete mapPointers.at(i);
		mapPointers.erase(it);
		numberOfMaps--;
	}
}

int WadEntry::getMapIndex(MapEntry* mapEntry)
{
	for (int i=0; i<numberOfMaps; i++) {
		if (mapPointers[i] == mapEntry)
			return i;
	}
	return -1;
}

bool WadEntry::matchMd5(unsigned char* md5ptr)
{
	for (int i=0; i<16; i++) {
		if (md5Digest[i] != *md5ptr)
			return false;
		md5ptr++;
	}
	return true;
}

string WadEntry::getMd5String() const
{
	MD5 md5Hash;
	for (int i=0; i<16; i++)
		md5Hash.setBytedigest(i, md5Digest[i]);
	return md5Hash.hexdigest();
}

uint16_t WadEntry::getTotalPlaytime()
{
	uint16_t result(0);
	for (int i=0; i<numberOfMaps; i++)
		result += mapPointers[i]->playTime;
	return result;
}

void WadEntry::getCommonFields(MapEntry* mapTempl)
{
	if (numberOfMaps == 0) return;
	MapEntry* me = mapPointers.at(0);
	mapTempl->author1 = me->author1;
	mapTempl->author2 = me->author2;
	mapTempl->singlePlayer = me->singlePlayer;
	mapTempl->cooperative = me->cooperative;
	mapTempl->deathmatch = me->deathmatch;
	mapTempl->otherMode = me->otherMode;
	for (int j=0; j<MAXTAGS; j++)
		mapTempl->tags[j] = me->tags[j];
	mapTempl->rating = me->rating;
	mapTempl->ownRating = me->ownRating;
	mapTempl->played = me->played;
	mapTempl->difficulty = me->difficulty;
	mapTempl->playTime = me->playTime;

	bool sameTags = true;
	for (int i=1; i<numberOfMaps; i++) {
		me = mapPointers.at(i);
		if (mapTempl->author1 != me->author1)
			mapTempl->author1 = NULL;
		if (mapTempl->author2 != me->author2)
			mapTempl->author2 = NULL;
		if (mapTempl->singlePlayer != me->singlePlayer)
			mapTempl->singlePlayer = 4;
		if (mapTempl->cooperative != me->cooperative)
			mapTempl->cooperative = 4;
		if (mapTempl->deathmatch != me->deathmatch)
			mapTempl->deathmatch = 4;
		if (mapTempl->otherMode != me->otherMode)
			mapTempl->otherMode = 0;
		for (int j=0; j<MAXTAGS; j++)
			if (mapTempl->tags[j] != me->tags[j]) sameTags = false;
		if (mapTempl->rating != me->rating)
			mapTempl->rating = 255;
		if (mapTempl->ownRating != me->ownRating)
			mapTempl->ownRating = 255;
		if (mapTempl->played != me->played)
			mapTempl->played = 5;
		if (mapTempl->difficulty != me->difficulty)
			mapTempl->difficulty = 0;
		if (mapTempl->playTime != me->playTime)
			mapTempl->playTime = 0;
	}
	if (!sameTags) {
		for (int j=0; j<MAXTAGS; j++)
			mapTempl->tags[j] = 0;
	}
}

void WadEntry::setCommonFields(MapEntry* mapTempl, bool setModFlags)
{
	MapEntry* me;
	unsigned char of;
	int tags = 0;
	for (int j=0; j<MAXTAGS; j++) {
		if (mapTempl->tags[j] > 0)
			tags++;
	}
	for (int i=0; i<numberOfMaps; i++) {
		me = mapPointers.at(i);
		of = me->ownFlags;
		if (mapTempl->author1 != NULL) {
			me->author1 = mapTempl->author1;
			of |= OF_MAINMOD;
		}
		if (mapTempl->author2 != NULL) {
			me->author2 = mapTempl->author2;
			of |= OF_MAINMOD;
		}
		if ((mapTempl->singlePlayer<4) && (me->singlePlayer>0)) {
			if (mapTempl->singlePlayer == 0) me->singlePlayer = 1;
			else me->singlePlayer = mapTempl->singlePlayer;
			of |= OF_MAINMOD;
		}
		if ((mapTempl->cooperative<4) && (me->cooperative>0)) {
			if (mapTempl->cooperative == 0) me->cooperative = 1;
			else me->cooperative = mapTempl->cooperative;
			of |= OF_MAINMOD;
		}
		if ((mapTempl->deathmatch<4) && (me->deathmatch>0)) {
			if (mapTempl->deathmatch == 0) me->deathmatch = 1;
			else me->deathmatch = mapTempl->deathmatch;
			of |= OF_MAINMOD;
		}
		if (mapTempl->otherMode > 0) {
			me->otherMode = mapTempl->otherMode;
			of |= OF_MAINMOD;
		}
		if (tags > 0) {
			for (int j=0; j<MAXTAGS; j++)
				me->tags[j] = mapTempl->tags[j];
			of |= OF_MAINMOD;
		}
		if (mapTempl->rating <= 100) {
			me->rating = mapTempl->rating;
			of |= OF_MAINMOD;
		}
		if (mapTempl->ownRating <= 100) {
			me->ownRating = mapTempl->ownRating;
			of |= OF_OWNMOD;
		}
		if (mapTempl->played < 5) {
			me->played = mapTempl->played;
			of |= OF_OWNMOD;
		}
		if (mapTempl->difficulty > 0) {
			me->difficulty = mapTempl->difficulty;
			of |= OF_OWNMOD;
		}
		if (mapTempl->playTime > 0) {
			me->playTime = mapTempl->playTime;
			of |= OF_OWNMOD;
		}
		if (setModFlags)
			me->ownFlags = of;
	}
}

//*********************************************************
//************************ WadText ************************
//*********************************************************

WadText::WadText(string md5Digest)
: wadMd5(md5Digest), changed(false)
{
	entries = new map<string, wxString>();
}

WadText::~WadText()
{
	delete entries;
}

void WadText::setText(string entry, wxString content)
{
	map<string,wxString>::iterator it = entries->find(entry);
	if (entries->empty() || it==entries->end()) {
		//New entry
		if (content.Length() > 0) {
			(*entries)[entry] = content;
			changed=true;
		}
	} else {
		//Existing entry
		if (content.Length()==0) {
			entries->erase(it);
			changed=true;
		} else if (!it->second.IsSameAs(content)) {
			(*entries)[entry] = content;
			changed=true;
		}
	}
}

wxString WadText::getText(string entry)
{
	if (entries->empty())
		return "";
	map<string,wxString>::iterator it = entries->find(entry);
	if (it == entries->end())
		return "";
	else
		return it->second;
}

bool WadText::removeText(string entry)
{
	if (entries->empty())
		return false;
	map<string,wxString>::iterator it = entries->find(entry);
	if (it == entries->end()) {
		return false;
	} else {
		entries->erase(it);
		return true;
	}
}

bool WadText::hasText()
{
	return !entries->empty();
}

//**********************************************************
//************************ TagEntry ************************
//**********************************************************

TagEntry::TagEntry(uint16_t id, string t)
: dbid(id), tag(t)
{}

//**********************************************************
//************************ MapEntry ************************
//**********************************************************

MapEntry::MapEntry(uint32_t id)
: dbid(id), wadPointer(NULL), name(""), title(""), basedOn(0),
author1(NULL), author2(NULL), singlePlayer(0), cooperative(0), deathmatch(0), otherMode(0),
linedefs(0), sectors(0), things(0), secrets(0), enemies(0), totalHP(0),
healthRatio(0.0), armorRatio(0.0), ammoRatio(0.0), area(0.0),
rating(255), flags(0), ownRating(255), ownFlags(0), played(0), difficulty(0), playTime(0)
{
	for (int i=0; i<MAXTAGS; i++)
		tags[i]=0;
}

string MapEntry::getTitle() const
{
	if (title.length()>0)
		return title;
	else
		return wadPointer->getName() + " " + name;
}

wxString MapEntry::fileName()
{
	return wxString(wadPointer->getMd5String()).Append("_").Append(name);
}
