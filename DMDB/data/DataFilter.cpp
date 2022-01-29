#include "DataFilter.h"


DataFilter::DataFilter(unsigned char t)
: type(t), name(""), isChanged(false), sortField(MAP_END), sortReverse(false)
{
}

//*****************************************************************
//************************ ComboDataFilter ************************
//*****************************************************************

ComboDataFilter::ComboDataFilter(unsigned char t)
: DataFilter(t), filters()
{
	//Specify vector size?
}

bool ComboDataFilter::hasAllFilters()
{
	if (filters.size() == 0) return false;
	for (int i=0; i<filters.size(); i++) {
		if (!filters[i]->hasFilter()) return false;
	}
	return true;
}


bool ComboDataFilter::hasFilter()
{
	if (filters.size() == 0) return false;
	for (int i=0; i<filters.size(); i++) {
		if (filters[i]->hasFilter()) return true;
	}
	return false;
}

bool ComboDataFilter::isList()
{
	if (filters.size() == 0) return false;
	for (int i=0; i<filters.size(); i++) {
		if (filters[i]->isList()) return true;
	}
	return false;
}

bool ComboDataFilter::includes(WadEntry* we)
{
	for (int i=0; i<filters.size(); i++) {
		if (!filters[i]->includes(we))
			return false;
	}
	return true;
}

bool ComboDataFilter::includes(MapEntry* me)
{
	for (int i=0; i<filters.size(); i++) {
		if (!filters[i]->includes(me))
			return false;
	}
	return true;
}

bool ComboDataFilter::hasChanges()
{
	if (isChanged) return true;
	for (int i=0; i<filters.size(); i++) {
		if (filters[i]->isChanged) return true;
	}
	return false;
}

void ComboDataFilter::setChanged(bool ch)
{
	isChanged = ch;
	for (int i=0; i<filters.size(); i++) {
		filters[i]->isChanged = ch;
	}
}

//**************************************************************
//************************ DataIdFilter ************************
//**************************************************************

DataIdFilter::DataIdFilter(unsigned char t, DataFilterOp dfop, uint32_t dbid)
: DataFilter(t), op(dfop), val(dbid)
{
}

bool DataIdFilter::includes(WadEntry* we)
{
	if (op == DFOP_MORE) return (we->dbid > val);
	else if (op == DFOP_LESS) return (we->dbid < val);
	else return (we->dbid == val);
}

bool DataIdFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->dbid > val);
	else if (op == DFOP_LESS) return (me->dbid < val);
	else return (me->dbid == val);
}

//****************************************************************
//************************ FileSizeFilter ************************
//****************************************************************

FileSizeFilter::FileSizeFilter(unsigned char t, DataFilterOp dfop, uint32_t fileSize)
: DataFilter(t), op(dfop), val(fileSize)
{
}

bool FileSizeFilter::includes(WadEntry* we)
{
	if (op == DFOP_MORE) return (we->fileSize > val);
	else if (op == DFOP_LESS) return (we->fileSize < val);
	else return (we->fileSize == val);
}

bool FileSizeFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->wadPointer->fileSize > val);
	else if (op == DFOP_LESS) return (me->wadPointer->fileSize < val);
	else return (me->wadPointer->fileSize == val);
}

//******************************************************************
//************************ ExtraFilesFilter ************************
//******************************************************************

ExtraFilesFilter::ExtraFilesFilter(unsigned char t)
: DataFilter(t)
{
}

bool ExtraFilesFilter::includes(WadEntry* we)
{
	return (we->extraFiles.length() > 0);
}

bool ExtraFilesFilter::includes(MapEntry* me)
{
	return (me->wadPointer->extraFiles.length() > 0);
}

//************************************************************
//************************ YearFilter ************************
//************************************************************

YearFilter::YearFilter(unsigned char t, DataFilterOp dfop, uint16_t year)
: DataFilter(t), op(dfop), val(year)
{
}

bool YearFilter::includes(WadEntry* we)
{
	if (op == DFOP_EQUALS) return (we->year == val);
	else if (op == DFOP_MORE) return (we->year > val);
	else return (we->year < val);
}

bool YearFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->wadPointer->year == val);
	else if (op == DFOP_MORE) return (me->wadPointer->year > val);
	else return (me->wadPointer->year < val);
}

//************************************************************
//************************ IwadFilter ************************
//************************************************************

IwadFilter::IwadFilter(unsigned char t, DataFilterOp dfop, unsigned char iwad)
: DataFilter(t), op(dfop), val(iwad)
{
}

bool IwadFilter::includes(WadEntry* we)
{
	if (op == DFOP_EQUALS) return (we->iwad == val);
	else if (op == DFOP_MORE) return (we->iwad > val);
	else return (we->iwad < val);
}

bool IwadFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->wadPointer->iwad == val);
	else if (op == DFOP_MORE) return (me->wadPointer->iwad > val);
	else return (me->wadPointer->iwad < val);
}

//**************************************************************
//************************ EngineFilter ************************
//**************************************************************

EngineFilter::EngineFilter(unsigned char t, DataFilterOp dfop, unsigned char engine)
: DataFilter(t), op(dfop), val(engine)
{
}

bool EngineFilter::includes(WadEntry* we)
{
	if (op == DFOP_EQUALS) return (we->engine == val);
	else if (op == DFOP_MORE) return (we->engine > val);
	else return (we->engine < val);
}

bool EngineFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->wadPointer->engine == val);
	else if (op == DFOP_MORE) return (me->wadPointer->engine > val);
	else return (me->wadPointer->engine < val);
}

//*****************************************************************
//************************ PlayStyleFilter ************************
//*****************************************************************

PlayStyleFilter::PlayStyleFilter(unsigned char t, DataFilterOp dfop, unsigned char playStyle)
: DataFilter(t), op(dfop), val(playStyle)
{
}

bool PlayStyleFilter::includes(WadEntry* we)
{
	if (op == DFOP_EQUALS) return (we->playStyle == val);
	else if (op == DFOP_MORE) return (we->playStyle > val);
	else return (we->playStyle < val);
}

bool PlayStyleFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->wadPointer->playStyle == val);
	else if (op == DFOP_MORE) return (me->wadPointer->playStyle > val);
	else return (me->wadPointer->playStyle < val);
}

//*****************************************************************
//************************ MapCountFilter ************************
//*****************************************************************

MapCountFilter::MapCountFilter(unsigned char t, DataFilterOp dfop, unsigned char mapCount)
: DataFilter(t), op(dfop), val(mapCount)
{
}

bool MapCountFilter::includes(WadEntry* we)
{
	if (op == DFOP_EQUALS) return (we->numberOfMaps == val);
	else if (op == DFOP_MORE) return (we->numberOfMaps > val);
	else return (we->numberOfMaps < val);
}

bool MapCountFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->wadPointer->numberOfMaps == val);
	else if (op == DFOP_MORE) return (me->wadPointer->numberOfMaps > val);
	else return (me->wadPointer->numberOfMaps < val);
}

//*****************************************************************
//************************ WadFlagsFilter ************************
//*****************************************************************

WadFlagsFilter::WadFlagsFilter(unsigned char t, uint16_t flags)
: DataFilter(t), val(flags)
{
}

bool WadFlagsFilter::includes(WadEntry* we)
{
	return (we->flags & val);
}

bool WadFlagsFilter::includes(MapEntry* me)
{
	return (me->wadPointer->flags & val);
}

//*******************************************************************
//************************ WadFlagsInvFilter ************************
//*******************************************************************

WadFlagsInvFilter::WadFlagsInvFilter(unsigned char t, uint16_t flags)
: DataFilter(t), val(flags)
{
}

bool WadFlagsInvFilter::includes(WadEntry* we)
{
	return (!(we->flags & val));
}

bool WadFlagsInvFilter::includes(MapEntry* me)
{
	return (!(me->wadPointer->flags & val));
}

//**************************************************************
//************************ RatingFilter ************************
//**************************************************************

RatingFilter::RatingFilter(unsigned char t, DataFilterOp dfop, unsigned char rating)
: DataFilter(t), op(dfop), val(rating)
{
}

bool RatingFilter::includes(WadEntry* we)
{
	if (op == DFOP_MORE) return (we->rating > val);
	else if (op == DFOP_LESS) return (we->rating < val);
	else return (we->rating == val);
}

bool RatingFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->rating > val);
	else if (op == DFOP_LESS) return (me->rating < val);
	else return (me->rating == val);
}

//*****************************************************************
//************************ OwnRatingFilter ************************
//*****************************************************************

OwnRatingFilter::OwnRatingFilter(unsigned char t, DataFilterOp dfop, unsigned char ownRating)
: DataFilter(t), op(dfop), val(ownRating)
{
}

bool OwnRatingFilter::includes(WadEntry* we)
{
	if (op == DFOP_MORE) return (we->ownRating > val);
	else if (op == DFOP_LESS) return (we->ownRating < val);
	else return (we->ownRating == val);
}

bool OwnRatingFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->ownRating > val);
	else if (op == DFOP_LESS) return (me->ownRating < val);
	else return (me->ownRating == val);
}

//****************************************************************
//************************ OwnFlagsFilter ************************
//****************************************************************

OwnFlagsFilter::OwnFlagsFilter(unsigned char t, unsigned char flags)
: DataFilter(t), val(flags)
{
}

bool OwnFlagsFilter::includes(WadEntry* we)
{
	return (we->ownFlags & val);
}

bool OwnFlagsFilter::includes(MapEntry* me)
{
	return (me->ownFlags & val);
}

//*******************************************************************
//************************ OwnFlagsInvFilter ************************
//*******************************************************************

OwnFlagsInvFilter::OwnFlagsInvFilter(unsigned char t, unsigned char flags)
: DataFilter(t), val(flags)
{
}

bool OwnFlagsInvFilter::includes(WadEntry* we)
{
	return (!(we->ownFlags & val));
}

bool OwnFlagsInvFilter::includes(MapEntry* me)
{
	return (!(me->ownFlags & val));
}

//*******************************************************************
//************************ TitleSearchFilter ************************
//*******************************************************************

TitleSearchFilter::TitleSearchFilter(unsigned char t, wxString str)
: DataFilter(t), searchStr(str)
{
	searchStr.Lower();
}

bool TitleSearchFilter::includes(WadEntry* we)
{
	wxString str1(we->title);
	if (str1.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	else return false;
}

bool TitleSearchFilter::includes(MapEntry* me)
{
	wxString str1(me->title);
	if (str1.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	else return true;
}

//******************************************************************
//************************ TextSearchFilter ************************
//******************************************************************

TextSearchFilter::TextSearchFilter(unsigned char t, wxString str)
: DataFilter(t), searchStr(str)
{
	searchStr.Lower();
}

bool TextSearchFilter::includes(WadEntry* we)
{
	wxString str1(we->title);
	if (str1.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	wxString str2(we->fileName);
	if (str2.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	return false;
}

bool TextSearchFilter::includes(MapEntry* me)
{
	wxString str1(me->title);
	if (str1.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	wxString str2(me->name);
	if (str2.Lower().Find(searchStr) != wxNOT_FOUND) return true;
	if (me->author1 != NULL)
		if (me->author1->findLowerCase(searchStr)) return true;
	if (me->author2 != NULL)
		if (me->author2->findLowerCase(searchStr)) return true;
	return includes(me->wadPointer);
}

//***************************************************************
//************************ BasedOnFilter ************************
//***************************************************************

BasedOnFilter::BasedOnFilter(unsigned char t)
: DataFilter(t)
{
}

bool BasedOnFilter::includes(MapEntry* me)
{
	return (me->basedOn > 0);
}

//**************************************************************
//************************ AuthorFilter ************************
//**************************************************************

AuthorFilter::AuthorFilter(unsigned char t, uint32_t authorId)
: DataFilter(t), val(authorId)
{
}

bool AuthorFilter::includes(MapEntry* me)
{
	if (me->author1 != NULL) {
		bool b = checkAuthor(me->author1);
		if (b) return true;
	}
	if (me->author2 != NULL) {
		bool b = checkAuthor(me->author2);
		if (b) return true;
	}
	return false;
}

bool AuthorFilter::checkAuthor(AuthorEntry* ae)
{
	if (ae->dbid == val) return true;
	if (ae->type > 0) {
		AuthorGroupEntry* age = (AuthorGroupEntry*)ae;
		for (int i=0; i<ae->type; i++) {
			if (checkAuthor(age->pointers[i]))
				return true;
		}
	}
	return false;
}

//********************************************************************
//************************ SinglePlayerFilter ************************
//********************************************************************

SinglePlayerFilter::SinglePlayerFilter(unsigned char t, DataFilterOp dfop, unsigned char singlePlayer)
: DataFilter(t), op(dfop), val(singlePlayer)
{
}

bool SinglePlayerFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->singlePlayer > val);
	else if (op == DFOP_LESS) return (me->singlePlayer < val);
	else return (me->singlePlayer == val);
}

//*******************************************************************
//************************ CooperativeFilter ************************
//*******************************************************************

CooperativeFilter::CooperativeFilter(unsigned char t, DataFilterOp dfop, unsigned char coop)
: DataFilter(t), op(dfop), val(coop)
{
}

bool CooperativeFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->cooperative > val);
	else if (op == DFOP_LESS) return (me->cooperative < val);
	else return (me->cooperative == val);
}

//******************************************************************
//************************ DeathmatchFilter ************************
//******************************************************************

DeathmatchFilter::DeathmatchFilter(unsigned char t, DataFilterOp dfop, unsigned char deathmatch)
: DataFilter(t), op(dfop), val(deathmatch)
{
}

bool DeathmatchFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->deathmatch > val);
	else if (op == DFOP_LESS) return (me->deathmatch < val);
	else return (me->deathmatch == val);
}

//*****************************************************************
//************************ OtherModeFilter ************************
//*****************************************************************

OtherModeFilter::OtherModeFilter(unsigned char t, DataFilterOp dfop, unsigned char otherMode)
: DataFilter(t), op(dfop), val(otherMode)
{
}

bool OtherModeFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->otherMode == val);
	else if (op == DFOP_MORE) return (me->otherMode > val);
	else return (me->otherMode < val);
}

//************************************************************
//************************ AreaFilter ************************
//************************************************************

AreaFilter::AreaFilter(unsigned char t, DataFilterOp dfop, float area)
: DataFilter(t), op(dfop), val(area)
{
}

bool AreaFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->area > val);
	else if (op == DFOP_LESS) return (me->area < val);
	else return (me->area == val);
}

//***********************************************************
//************************ TagFilter ************************
//***********************************************************

TagFilter::TagFilter(unsigned char t, uint32_t tagId)
: DataFilter(t), val(tagId)
{
}

bool TagFilter::includes(MapEntry* me)
{
	for (int i=0; i<MAXTAGS; i++) {
		if (me->tags[i] == val) return true;
	}
	return false;
}

//****************************************************************
//************************ MapFlagsFilter ************************
//****************************************************************

MapFlagsFilter::MapFlagsFilter(unsigned char t, unsigned char flags)
: DataFilter(t), val(flags)
{
}

bool MapFlagsFilter::includes(MapEntry* me)
{
	return (me->flags & val);
}

//**************************************************************
//************************ PlayedFilter ************************
//**************************************************************

PlayedFilter::PlayedFilter(unsigned char t, DataFilterOp dfop, unsigned char played)
: DataFilter(t), op(dfop), val(played)
{
}

bool PlayedFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->played > val);
	else if (op == DFOP_LESS) return (me->played < val);
	else return (me->played == val);
}

//******************************************************************
//************************ DifficultyFilter ************************
//******************************************************************

DifficultyFilter::DifficultyFilter(unsigned char t, DataFilterOp dfop, unsigned char difficulty)
: DataFilter(t), op(dfop), val(difficulty)
{
}

bool DifficultyFilter::includes(MapEntry* me)
{
	if (op == DFOP_EQUALS) return (me->difficulty == val);
	else if (op == DFOP_MORE) return (me->difficulty > val);
	else return (me->difficulty < val);
}

//****************************************************************
//************************ PlayTimeFilter ************************
//****************************************************************

PlayTimeFilter::PlayTimeFilter(unsigned char t, DataFilterOp dfop, unsigned char playTime)
: DataFilter(t), op(dfop), val(playTime)
{
}

bool PlayTimeFilter::includes(MapEntry* me)
{
	if (op == DFOP_MORE) return (me->playTime > val);
	else if (op == DFOP_LESS) return (me->playTime < val);
	else return (me->playTime == val);
}

//****************************************************************
//************************ DataListFilter ************************
//****************************************************************

DataListFilter::DataListFilter(unsigned char t) : DataFilter(t)
{
	entries = new list<uint32_t>();
}

DataListFilter::~DataListFilter()
{
	delete entries;
}

bool DataListFilter::hasFilter()
{
	//This is called before calling includes on each entry
	//in a list, so we initiate the iterator here.
	searchIter = entries->begin();
	return true;
}

bool DataListFilter::includes(WadEntry* we)
{
	if (entries->size()==0 || (we->dbid > entries->back()))
		return false;
	while (searchIter!=entries->end() && (*searchIter)<=we->dbid ) {
		if ( (*searchIter) == we->dbid )
			return true;
		searchIter++;
	}
	return false;
}

bool DataListFilter::includes(MapEntry* me)
{
	if (entries->size()==0 || (me->dbid > entries->back()))
		return false;
	while (searchIter!=entries->end() && (*searchIter)<=me->dbid ) {
		if ( (*searchIter) == me->dbid )
			return true;
		searchIter++;
	}
	return false;
}

void DataListFilter::addEntry(uint32_t id)
{
	for (list<uint32_t>::iterator it=entries->begin(); it != entries->end(); ++it) {
		if ((*it) == id)
			return;
	}
	entries->push_back(id);
	entries->sort();
	isChanged = true;
}

bool DataListFilter::removeEntry(uint32_t id)
{
	list<uint32_t>::iterator it = entries->begin();
	while (it!=entries->end() && (*it)<=id ) {
		if ((*it) == id) {
			it = entries->erase(it);
			isChanged = true;
			return true;
		}
		it++;
	}
	return false;
}
