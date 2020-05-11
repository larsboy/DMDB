/*
* DataManager implementation
*/

#include "DataManager.h"
#include "../gui/GuiBase.h"


//******************************************************************
//************************ Author name/sort ************************
//******************************************************************

wxString getAuthorNameFirstLast(const AuthorEntry* ae)
{
	if (ae->namef.Length()==0 && ae->namel.Length()==0)
		return ae->alias1;
	else if (ae->namef.Length()==0)
		return ae->namel;
	else if (ae->namel.Length()==0)
		return ae->namef;
	else
		return ae->namef+" "+ae->namel;
}

wxString getAuthorNameLastFirst(const AuthorEntry* ae)
{
	if (ae->namef.Length()==0 && ae->namel.Length()==0)
		return ae->alias1;
	else if (ae->namef.Length()==0)
		return ae->namel;
	else if (ae->namel.Length()==0)
		return ae->namef;
	else
		return ae->namel+", "+ae->namef;
}

wxString getAuthorNameAlias(const AuthorEntry* ae)
{
	if (ae->alias1.Length()>0) {
		return ae->alias1;
	} else {
		return getAuthorNameFirstLast(ae);
	}
}

/*!
* Function pointer for how to create string representations of
* authors from the various fields in the entries. Can be any of
* the getAuthorName methods above. Configured through DataManager.
*/
wxString (*authorNamingScheme)(const AuthorEntry*);

bool author_comp(const AuthorEntry* first, const AuthorEntry* second)
{
	const wxString fir = authorNamingScheme(first);
	const wxString sec = authorNamingScheme(second);
	return (fir.CmpNoCase(sec) < 0);
}

//*******************************************************************
//************************ Other comparators ************************
//*******************************************************************

bool tag_comp(const TagEntry* first, const TagEntry* second)
{
	return (first->tag.compare(second->tag) < 0);
}

bool filter_comp(const DataFilter* first, const DataFilter* second)
{
	return (first->name.compare(second->name) < 0);
}

bool wad_comp_dbid(WadEntry* first, WadEntry* second)
{
	return (first->dbid < second->dbid);
}

bool wad_comp_fileName(WadEntry* first, WadEntry* second)
{
	return (first->fileName.compare(second->fileName) < 0);
}

bool wad_comp_fileSize(WadEntry* first, WadEntry* second)
{
	return (first->fileSize < second->fileSize);
}

bool wad_comp_idgames(WadEntry* first, WadEntry* second)
{
	return (first->idGames < second->idGames);
}

bool wad_comp_title(WadEntry* first, WadEntry* second)
{
	return (first->title.compare(second->title) < 0);
}

bool wad_comp_year(WadEntry* first, WadEntry* second)
{
	return (first->year < second->year);
}

bool wad_comp_iwad(WadEntry* first, WadEntry* second)
{
	return (first->iwad < second->iwad);
}

bool wad_comp_engine(WadEntry* first, WadEntry* second)
{
	return (first->engine < second->engine);
}

bool wad_comp_playstyle(WadEntry* first, WadEntry* second)
{
	return (first->playStyle < second->playStyle);
}

bool wad_comp_maps(WadEntry* first, WadEntry* second)
{
	return (first->numberOfMaps < second->numberOfMaps);
}

bool wad_comp_rating(WadEntry* first, WadEntry* second)
{
	return (first->rating < second->rating);
}

bool wad_comp_ownrating(WadEntry* first, WadEntry* second)
{
	return (first->ownRating < second->ownRating);
}

bool wad_comp_playtime(WadEntry* first, WadEntry* second)
{
	return (first->getTotalPlaytime() < second->getTotalPlaytime());
}

bool wad_comp_name(WadEntry* first, WadEntry* second)
{
	return (first->getName().compare(second->getName()) < 0);
}


bool map_comp_filename(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->fileName.compare(second->wadPointer->fileName) < 0);
}

bool map_comp_idgames(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->idGames < second->wadPointer->idGames);
}

bool map_comp_wadtitle(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->title.compare(second->wadPointer->title) < 0);
}

bool map_comp_year(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->year < second->wadPointer->year);
}

bool map_comp_iwad(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->iwad < second->wadPointer->iwad);
}

bool map_comp_engine(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->engine < second->wadPointer->engine);
}

bool map_comp_playstyle(MapEntry* first, MapEntry* second)
{
	return (first->wadPointer->playStyle < second->wadPointer->playStyle);
}

bool map_comp_dbid(MapEntry* first, MapEntry* second)
{
	return (first->dbid < second->dbid);
}

bool map_comp_name(MapEntry* first, MapEntry* second)
{
	return (first->name.compare(second->name) < 0);
}

bool map_comp_title(MapEntry* first, MapEntry* second)
{
	return (first->title.compare(second->title) < 0);
}

bool map_comp_author1(MapEntry* first, MapEntry* second)
{
	const wxString fir = (first->author1==NULL)? "": authorNamingScheme(first->author1);
	const wxString sec = (second->author1==NULL)? "": authorNamingScheme(second->author1);
	return (fir.CmpNoCase(sec) < 0);
}

bool map_comp_author2(MapEntry* first, MapEntry* second)
{
	const wxString fir = (first->author2==NULL)? "": authorNamingScheme(first->author2);
	const wxString sec = (second->author2==NULL)? "": authorNamingScheme(second->author2);
	return (fir.CmpNoCase(sec) < 0);
}

bool map_comp_single(MapEntry* first, MapEntry* second)
{
	return (first->singlePlayer < second->singlePlayer);
}

bool map_comp_coop(MapEntry* first, MapEntry* second)
{
	return (first->cooperative < second->cooperative);
}

bool map_comp_dm(MapEntry* first, MapEntry* second)
{
	return (first->deathmatch < second->deathmatch);
}

bool map_comp_mode(MapEntry* first, MapEntry* second)
{
	return (first->otherMode < second->otherMode);
}

bool map_comp_linedefs(MapEntry* first, MapEntry* second)
{
	return (first->linedefs < second->linedefs);
}

bool map_comp_sectors(MapEntry* first, MapEntry* second)
{
	return (first->sectors < second->sectors);
}

bool map_comp_things(MapEntry* first, MapEntry* second)
{
	return (first->things < second->things);
}

bool map_comp_secrets(MapEntry* first, MapEntry* second)
{
	return (first->secrets < second->secrets);
}

bool map_comp_enemies(MapEntry* first, MapEntry* second)
{
	return (first->enemies < second->enemies);
}

bool map_comp_totalhp(MapEntry* first, MapEntry* second)
{
	return (first->totalHP < second->totalHP);
}

bool map_comp_ammorat(MapEntry* first, MapEntry* second)
{
	return (first->ammoRatio < second->ammoRatio);
}

bool map_comp_healthrat(MapEntry* first, MapEntry* second)
{
	return (first->healthRatio < second->healthRatio);
}

bool map_comp_armorrat(MapEntry* first, MapEntry* second)
{
	return (first->armorRatio < second->armorRatio);
}

bool map_comp_area(MapEntry* first, MapEntry* second)
{
	return (first->area < second->area);
}

bool map_comp_lddens(MapEntry* first, MapEntry* second)
{
	return (first->linedefDensity() < second->linedefDensity());
}

bool map_comp_enemydens(MapEntry* first, MapEntry* second)
{
	return (first->enemyDensity() < second->enemyDensity());
}

bool map_comp_hpdens(MapEntry* first, MapEntry* second)
{
	return (first->hitpointDensity() < second->hitpointDensity());
}

bool map_comp_rating(MapEntry* first, MapEntry* second)
{
	return (first->rating < second->rating);
}

bool map_comp_ownrating(MapEntry* first, MapEntry* second)
{
	return (first->ownRating < second->ownRating);
}

bool map_comp_difficulty(MapEntry* first, MapEntry* second)
{
	return (first->difficulty < second->difficulty);
}

bool map_comp_playtime(MapEntry* first, MapEntry* second)
{
	return (first->playTime < second->playTime);
}

bool map_comp_wad(MapEntry* first, MapEntry* second)
{
	if (first->wadPointer == second->wadPointer) //Same wad
		return (first->name.compare(second->name) < 0);
	else
		return (first->wadPointer->getName().compare(second->wadPointer->getName()) < 0);
}


//*********************************************************************
//************************ DataManager general ************************
//*********************************************************************

DataManager::DataManager(DataStatusListener* l)
: authorMaster(NULL), nextAuthorId(1), authorMod(false), firstNewAuthor(-1), authorList(NULL),
authorTextDir(0), authorTextFile(NULL), authorTextIndex(-1), authorText(""),
tagLength(DEFAULT_TAG_LENGTH), tagMaster(), tagList(NULL),
wadMaster(), nextWadId(1), wadMod(false), wadOwnMod(false), firstNewWad(-1), wadList(NULL),
mapMaster(), nextMapId(1), mapMod(false), mapOwnMod(false), firstNewMap(-1), mapList(NULL),
wadText(NULL), dataViewMod(false), wadTitleFilter(NULL)
{
	listener = l;
	authorNamingScheme = getAuthorNameFirstLast;
	wadLists = new list<DataListFilter*>();
	mapLists = new list<DataListFilter*>();
	currentWadFilter = new ComboDataFilter(FILTER_WAD);
	currentMapFilter = new ComboDataFilter(FILTER_MAP);
}

DataManager::~DataManager()
{
	unsigned int i;
	for (i=0; i<authorMaster->size(); i++)
		delete (*authorMaster)[i];
	delete authorMaster;
	if (authorList != NULL)
		delete authorList;
	for (i=0; i<tagMaster.size(); i++)
		delete tagMaster[i];
	if (tagList != NULL)
		delete tagList;
	for (i=0; i<wadMaster.size(); i++)
		delete wadMaster[i];
	if (wadList != NULL)
		delete wadList;
	for (i=0; i<mapMaster.size(); i++)
		delete mapMaster[i];
	if (mapList != NULL)
		delete mapList;
	if (wadText != NULL)
		delete wadText;
	for (list<DataListFilter*>::iterator it=wadLists->begin(); it != wadLists->end(); ++it) {
		delete (*it);
	}
	delete wadLists;
	for (list<DataListFilter*>::iterator it=mapLists->begin(); it != mapLists->end(); ++it) {
		delete (*it);
	}
	delete mapLists;
	if (wadTitleFilter != NULL)
		delete wadTitleFilter;
}

void DataManager::load()
{
	// Author core:
	loadAuthors();
	loadAuthorTextDir();
	makeAuthorList();

	// Tags:
	loadTags();
	makeTagList();

	// Wads & Maps:
	loadWads();
	loadMaps();

	// Load persisted filter lists
	loadDataFilters();

	// Make sure sub-folders exist
	wxFileName mapImgDirname(getMapImgFolder(), "");
	mapImgDirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	wxFileName sshotDirname(getScreenshotFolder(), "");
	sshotDirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	wxFileName textDirname(getTextFolder(), "");
	textDirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	wxFileName tempDirname(getTempFolder(), "");
	tempDirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	wxFileName failDirname(getFailedFolder(), "");
	failDirname.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
}


//********************************************************************
//************************ Author persistence ************************
//********************************************************************

void DataManager::openAuthorTextFile()
{
	if (authorTextFile == NULL) {
		wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORTEXT);
		if (!wxFile::Exists(fname)) {
			wxLogVerbose("File %s not found, will create it", FILE_AUTHORTEXT);
			authorTextFile = new wxFile(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORTEXT, wxFile::write);
			authorTextDir = 5;
			authorTextFile->Write(&authorTextDir, 4);
			authorTextFile->Close();
			delete authorTextFile;
		}
		authorTextFile = new wxFile(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORTEXT, wxFile::read_write);
	}
	if (!authorTextFile->IsOpened()) throw GuiError("Couldn't open file.", FILE_AUTHORTEXT);
	wxLogVerbose("Author text file ready, dir offset is %i", authorTextDir);
}

void DataManager::closeAuthorTextFile()
{
	if (authorTextFile != NULL) {
		//authorTextFile->Flush();
		authorTextFile->Close();
		delete authorTextFile;
		authorTextFile = NULL;
		wxLogVerbose("Author text file closed");
	}
}

void DataManager::writeAuthorTextDir(wxFile* file)
{
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		file->Write(&((*it)->dbid), 4);
		file->Write(&((*it)->textOffset), 4);
	}
}

void DataManager::loadAuthorTextDir()
{
	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORTEXT);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_AUTHORTEXT);
		return;
	}
	wxLogVerbose("Reading directory from file %s", FILE_AUTHORTEXT);
	wxFile file(fname, wxFile::read);
	if (!file.IsOpened()) throw GuiError("Couldn't open file.", FILE_AUTHORTEXT);

	file.Read(&authorTextDir, 4);
	if (authorTextDir < file.Length()) {
		file.Seek(authorTextDir, wxFromStart);
		wxFileInputStream fileStream(file);
		wxBufferedInputStream buf(fileStream, 512);
		uint32_t dbid;
		uint32_t offset;
		long index = 0;
		while ((!buf.Eof()) && (index<authorMaster->size())) {
			buf.Read(&dbid, 4);
			if (buf.Eof()) break;
			buf.Read(&offset, 4);
			if (dbid != (*authorMaster)[index]->dbid)
				index = getAuthorMasterIndex(dbid);
			if (index > -1) {
				(*authorMaster)[index]->textOffset = offset;
			}
			index++;
		}
	}
	file.Close();
}

void DataManager::addAuthorText(AuthorEntry* newEntry, wxString text)
{
	authorTextIndex = authorMaster->size()-1;
	authorText = text;

	if (authorTextFile==NULL || !authorTextFile->IsOpened())
		throw new GuiError("Author text file not open, can't append new entry");

	//Write the text at old authorTextDir
	newEntry->textOffset = authorTextDir;
	authorTextFile->Seek(authorTextDir, wxFromStart);
	wxScopedCharBuffer buf = authorText.utf8_str();
	uint16_t l = buf.length();
	authorTextFile->Write(&l, 2);
	if (l > 0) authorTextFile->Write(buf.data(), l);
	//Whole number of blocks - AUTHOR_DESC_BLOCK
	int blocks = (l / AUTHOR_DESC_BLOCK) + 1;
	int left = (blocks*AUTHOR_DESC_BLOCK) - l;
	char ch = 0;
	for (int i=0; i<left; i++)
		authorTextFile->Write(&ch, 1);

	authorTextDir += (blocks*AUTHOR_DESC_BLOCK)+2;
	writeAuthorTextDir(authorTextFile);
	//Write new authorTextDir
	authorTextFile->Seek(0, wxFromStart);
	authorTextFile->Write(&authorTextDir, 4);
	wxLogVerbose("Appended %i text blocks to author text file, new dir offset is %i",
		blocks, authorTextDir);
}

wxString DataManager::getAuthorText(AuthorEntry* ae)
{
	if (authorTextIndex>-1 && (authorMaster->at(authorTextIndex)->dbid == ae->dbid))
		return authorText; //Already in memory
	if (ae->textOffset == 0)
		return ""; //No entry
	if (authorTextFile == NULL)
		return ""; //Error - file not open
	authorTextFile->Seek(ae->textOffset, wxFromStart);
	uint16_t l;
	char buf[AUTHOR_DESC_LIMIT];
	authorTextFile->Read(&l, 2);
	if (l > 0) {
		authorTextFile->Read(buf, l);
		authorText = wxString::FromUTF8(buf,l);
	} else {
		authorText = "";
	}
	authorTextIndex = getAuthorMasterIndex(ae->dbid);
	wxLogVerbose("Loaded text of author %i - offset %i",
		ae->dbid, ae->textOffset);
	return authorText;
}

void DataManager::authorTextModified(AuthorEntry* ae, wxString text)
{
	if (authorTextFile==NULL || !authorTextFile->IsOpened())
		throw new GuiError("Author text file not open, can't update entry");

	authorText = text;
	if (authorTextIndex==-1 || (authorMaster->at(authorTextIndex)->dbid != ae->dbid))
		authorTextIndex = getAuthorMasterIndex(ae->dbid);

	wxScopedCharBuffer buf = authorText.utf8_str();
	int nextOffset;
	if ((authorTextIndex+1) < authorMaster->size())
		nextOffset = authorMaster->at(authorTextIndex+1)->textOffset;
	else
		nextOffset = authorTextDir;
	if ((ae->textOffset+buf.length()+2) < nextOffset) {
		//Have room in existing space - replace
		authorTextFile->Seek(ae->textOffset, wxFromStart);
		uint16_t l = buf.length();
		authorTextFile->Write(&l, 2);
		if (l > 0) authorTextFile->Write(buf.data(), l);
		char ch = 0;
		for (int i=(ae->textOffset+l+2); i<nextOffset; i++)
			authorTextFile->Write(&ch, 1);
		wxLogVerbose("Updated text of author %i - offset %i",
			ae->dbid, ae->textOffset);
	} else {
		//Must rewrite whole file
		rewriteAuthorText();
	}
}

void DataManager::rewriteAuthorText()
{
	AuthorEntry* ae = authorMaster->at(authorTextIndex);
	wxLogVerbose("Rewriting author text file to fit entry %i", ae->dbid);
	wxTempFile* tempFile = new wxTempFile(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORTEXT);
	authorTextFile->Seek(0, wxFromStart);
	char buf[1024];
	int len;
	//Copy first part of file - from start to ae->textOffset
	int offs = 0;
	while (offs < ae->textOffset) {
		len = (offs+1024 < ae->textOffset)? 1024: ae->textOffset-offs;
		authorTextFile->Read(buf, len);
		tempFile->Write(buf, len);
		offs += len;
	}
	//Skip the entry we are replacing in the old file
	if ((authorTextIndex+1) < authorMaster->size())
		offs = authorMaster->at(authorTextIndex+1)->textOffset;
	else
		offs = authorTextDir;
	authorTextFile->Seek(offs, wxFromStart);

	//Write authorText to new file
	wxScopedCharBuffer cbuf = authorText.utf8_str();
	uint16_t l = cbuf.length();
	tempFile->Write(&l, 2);
	if (l > 0) tempFile->Write(cbuf.data(), l);
	//Whole number of blocks - AUTHOR_DESC_BLOCK
	int blocks = (l / AUTHOR_DESC_BLOCK) + 1;
	int left = (blocks*AUTHOR_DESC_BLOCK) - l;
	char ch = 0;
	for (int i=0; i<left; i++)
		tempFile->Write(&ch, 1);

	//Find change in offset for subsequent entries
	int nextOffset = ae->textOffset + (blocks*AUTHOR_DESC_BLOCK)+2;
	int offsetDelta = nextOffset - offs;

	//Copy rest of text entries
	while (offs < authorTextDir) {
		len = (offs+1024 < authorTextDir)? 1024: authorTextDir-offs;
		authorTextFile->Read(buf, len);
		tempFile->Write(buf, len);
		offs += len;
	}

	//Update offsets and directory
	wxLogVerbose("Written updated text of author %i - subsequent offsets moved %i bytes",
		ae->dbid, offsetDelta);
	for (int i=authorTextIndex+1; i<authorMaster->size(); i++)
		(*authorMaster)[i]->textOffset += offsetDelta;
	authorTextDir += offsetDelta;
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		tempFile->Write(&((*it)->dbid), 4);
		tempFile->Write(&((*it)->textOffset), 4);
	}
	tempFile->Seek(0, wxFromStart);
	tempFile->Write(&authorTextDir, 4);

	closeAuthorTextFile();
	tempFile->Commit();
	delete tempFile;
	wxLogVerbose("Finished rewriting author text file, new dir offset is %i",
		authorTextDir);
	openAuthorTextFile();
}

void DataManager::addAuthor(AuthorEntry* newEntry, wxString text)
{
	newEntry->dbid = nextAuthorId++;
	newEntry->modified = OF_MAINNEW;
	if (firstNewAuthor == -1)
		firstNewAuthor = authorMaster->size();
	authorMaster->push_back(newEntry);
	authorList->push_back(newEntry);
	authorList->sort(author_comp);
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	addAuthorText(newEntry, text);
}

void DataManager::authorModified(AuthorEntry* author)
{
	author->modified |= OF_MAINMOD;
	authorMod = true;
	authorList->sort(author_comp);
}

void DataManager::deleteAuthor(AuthorEntry* author)
{
	author->modified |= OFLG_DELETE;
	removeAuthorFromMaps(author);
	removeAuthorFromGroups(author); //Empty groups will get OFLG_DELETE
	authorMod=true;
	authorDeleteImpl();
}

void DataManager::mergeAuthors(AuthorEntry* authorDelete, AuthorEntry* authorKeep)
{
	authorDelete->modified |= OFLG_DELETE;
	removeAuthorFromMaps(authorDelete, authorKeep);
	removeAuthorFromGroups(authorDelete, authorKeep);
	authorMod=true;
	authorDeleteImpl();
}

void DataManager::removeAuthorFromGroups(AuthorEntry* auth, AuthorEntry* repAuth)
{
	AuthorGroupEntry* age;
	int i;
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		if ((*it)->type > 0) {
			age = dynamic_cast<AuthorGroupEntry*>(*it);
			i = age->findEntry(auth);
			if (i > -1) {
				if (repAuth == NULL)
					age->removeEntry(i);
				else
					age->pointers[i] = repAuth;
				//If an entry was removed, the group can become empty,
				//in which case it must be deleted.
				if (age->type == 0) {
					age->modified |= OFLG_DELETE;
					removeAuthorFromMaps(age);
					removeAuthorFromGroups(age);
				}
			}
		}
	}
}

void DataManager::authorDeleteImpl()
{
	authorTextIndex=-1;
	authorText="";
	int delCount=0;
	//Remake authorMaster, deleting AuthorEntry obj.s with OFLG_DELETE
	vector<AuthorEntry*>* newMaster = new vector<AuthorEntry*>();
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		if ((*it)->modified & OFLG_DELETE) {
			delete (*it);
			delCount++;
		} else {
			newMaster->push_back(*it);
		}
	}
	delete authorMaster;
	authorMaster = newMaster;

	//Rewrite text file directory, at same offset (text is unchanged).
	//Text of deleted authors stay in place, but the space is free to be reused.
	if (authorTextFile==NULL)
		openAuthorTextFile();
	authorTextFile->Seek(authorTextDir, wxFromStart);
	writeAuthorTextDir(authorTextFile);

	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	wxLogVerbose("Deleted %i author entries", delCount);
	saveAuthors();

	//Remake authorList (Note: Any filter is lost)
	if (authorList != NULL) delete authorList;
	makeAuthorList();
}

void DataManager::saveAuthors()
{
	if (authorMod) {
		//Need to save whole list
		wxLogVerbose("Writing all entries to file %s", FILE_AUTHORDB);
		wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORDB);
		if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_AUTHORDB);
		wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);
		for (int i=0; i<authorMaster->size(); i++) {
			writeAuthor(buf, (*authorMaster)[i]);
		}
		delete buf;
		file.Close();

	} else if (firstNewAuthor>-1) {
		//Just append new
		wxLogVerbose("Appending new entries to file %s", FILE_AUTHORDB);
		wxFile file(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORDB, wxFile::write_append);
		if (!file.IsOpened()) throw GuiError("Couldn't open file.", FILE_AUTHORDB);
		wxFileOutputStream filestr(file);
		wxBufferedOutputStream* buf = new wxBufferedOutputStream(filestr, 512);
		for (int i=firstNewAuthor; i<authorMaster->size(); i++) {
			writeAuthor(buf, (*authorMaster)[i]);
		}
		delete buf;
		file.Close();
	}
	authorMod=false;
	firstNewAuthor=-1;
}

void DataManager::writeAuthor(wxOutputStream* file, AuthorEntry* entry)
{
	uint16_t l;
	file->Write(&(entry->type), 1);
	file->Write(&(entry->dbid), 4);
	// First name:
	wxScopedCharBuffer buf = entry->namef.utf8_str();
	l = buf.length();
	file->Write(&l, 2);
	if (l > 0) file->Write(buf.data(), l);
	// Last name:
	buf = entry->namel.utf8_str();
	l = buf.length();
	file->Write(&l, 2);
	if (l > 0) file->Write(buf.data(), l);
	// Alias1:
	buf = entry->alias1.utf8_str();
	l = buf.length();
	file->Write(&l, 2);
	if (l > 0) file->Write(buf.data(), l);
	// Alias2:
	buf = entry->alias2.utf8_str();
	l = buf.length();
	file->Write(&l, 2);
	if (l > 0) file->Write(buf.data(), l);
	entry->modified = 0;
	if (entry->type > 0) {
		AuthorGroupEntry* group = dynamic_cast<AuthorGroupEntry*>(entry);
		AuthorEntry* ae;
		for (int i=0; i<group->type; i++) {
			ae = group->pointers[i];
			file->Write(&(ae->dbid), 4);
		}
	}
}

void DataManager::loadAuthors()
{
	authorMaster = new vector<AuthorEntry*>();
	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_AUTHORDB);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_AUTHORDB);
		return;
	}
	wxLogVerbose("Reading all entries from file %s", FILE_AUTHORDB);
	wxFileInputStream fileStream(fname);
	if (!fileStream.IsOk()) throw GuiError("Couldn't open file.", FILE_AUTHORDB);
	wxBufferedInputStream file(fileStream, 1024);

	unsigned char type;
	uint32_t dbid;
	uint16_t l;
	char buf[256];
	AuthorEntry* entry;
	AuthorGroupEntry* group;
	vector<AuthorGroupEntry*> groupList;
	while (!file.Eof()) {
		file.Read(&type, 1);
		file.Read(&dbid, 4);
		if (file.Eof()) break;
		if (type == 0) {
			entry = new AuthorEntry(dbid);
		} else {
			group = new AuthorGroupEntry(dbid);
			entry = group;
		}
		// First name:
		file.Read(&l, 2);
		if (l > 0) {
			file.Read(buf, l);
			entry->namef = wxString::FromUTF8(buf,l);
		}
		// Last name:
		file.Read(&l, 2);
		if (l > 0) {
			file.Read(buf, l);
			entry->namel = wxString::FromUTF8(buf,l);
		}
		// Alias1:
		file.Read(&l, 2);
		if (l > 0) {
			file.Read(buf, l);
			entry->alias1 = wxString::FromUTF8(buf,l);
		}
		// Alias2:
		file.Read(&l, 2);
		if (l > 0) {
			file.Read(buf, l);
			entry->alias2 = wxString::FromUTF8(buf,l);
		}
		if (type > 0) { //Group
			group->type = type;
			for (int i=0; i<type; i++) {
				file.Read(&dbid, 4);
				group->entryIds[i] = dbid;
			}
			groupList.push_back(group);
		}
		authorMaster->push_back(entry);
	}
	if (authorMaster->size() == 0)
		nextAuthorId = 1;
	else
		nextAuthorId = authorMaster->back()->dbid + 1;
	//Find group entries:
	for (int i=0; i<groupList.size(); i++) {
		group = groupList[i];
		for (int j=0; j<group->type; j++) {
			group->pointers[j] = getAuthorMasterEntry(group->entryIds[j]);
		}
	}
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	wxLogVerbose("Finished reading %i entries", authorMaster->size());
}


//******************************************************************
//************************ Author in-memory ************************
//******************************************************************

void DataManager::setAuthorNaming(AuthorNaming an)
{
	if (an == AUTHNAME_FIRST_LAST) {
		authorNamingScheme = getAuthorNameFirstLast;
	} else if (an == AUTHNAME_LAST_FIRST) {
		authorNamingScheme = getAuthorNameLastFirst;
	} else {
		authorNamingScheme = getAuthorNameAlias;
	}
	if (authorList != NULL) {
		delete authorList;
		makeAuthorList();
	}
}

wxString DataManager::getAuthorString(AuthorEntry* ae)
{
	return authorNamingScheme(ae);
}

unsigned int DataManager::getMapAuthorCount(uint32_t dbid)
{
	unsigned int result = 0;
	AuthorFilter af = AuthorFilter(FILTER_MAP, dbid);
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it!=mapMaster.end(); ++it) {
		if (af.includes(*it))
			result++;
	}
	return result;
}

void DataManager::makeAuthorList()
{
	authorList = new list<AuthorEntry*>();
	const int item_amount = authorMaster->size();
	for (int n=0; n<item_amount; n++) {
		authorList->push_back((*authorMaster)[n]);
	}
	authorList->sort(author_comp);
}

AuthorEntry* DataManager::getAuthor(long index)
{
	AuthorEntry* aut = NULL;
	if ((index>-1) && (index<authorList->size())) {
		list<AuthorEntry*>::iterator it = authorList->begin();
		for (int i=0; i<index; i++)
			it++;
		aut = *it;
	}
	return aut;
}

/*
AuthorEntry* DataManager::getAuthorId(uint32_t dbid)
{
	//Search for id...
	return NULL;
}*/

long DataManager::getAuthorIndex(uint32_t dbid)
{
	long index = -1;
	for (list<AuthorEntry*>::iterator it=authorList->begin(); it != authorList->end(); ++it) {
		index++;
		if ((*it)->dbid == dbid)
			return index;
	}
	return -1;
}

long DataManager::getAuthorMasterIndex(uint32_t id)
{
	long index = -1;
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		index++;
		if ((*it)->dbid == id)
			return index;
	}
	return -1;
}

AuthorEntry* DataManager::getAuthorMasterEntry(uint32_t id)
{
	for (vector<AuthorEntry*>::iterator it=authorMaster->begin(); it != authorMaster->end(); ++it) {
		if ((*it)->dbid == id)
			return *it;
	}
	return NULL;
}

int DataManager::getAuthorIndex(wxString nameStart)
{
	if (nameStart.Length() == 0)
        return -1;
    int index = -1;
    for (list<AuthorEntry*>::iterator it=authorList->begin(); it != authorList->end(); ++it) {
        index++;
        if (authorNamingScheme(*it).CmpNoCase(nameStart) >= 0)
            return index;
    }
    return index;
}

AuthorEntry* DataManager::findAuthor(wxString namef, wxString namel, wxString alias)
{
	const int item_amount = authorMaster->size();
	for (int n=0; n<item_amount; n++) {
		if (namef.size()>0 && (*authorMaster)[n]->namef.IsSameAs(namef,false)) {
			if (namel.size()>0 && (*authorMaster)[n]->namel.IsSameAs(namel,false))
				return (*authorMaster)[n];
		}
		if (alias.size()>0 && (*authorMaster)[n]->alias1.IsSameAs(alias,false))
			return (*authorMaster)[n];
	}
	return NULL;
}

void DataManager::filterAuthorList(wxString filterStr, bool update)
{
	filterStr.MakeLower();
    list<AuthorEntry*>* filterList = new list<AuthorEntry*>();
    if (update) {
		for (list<AuthorEntry*>::iterator it=authorList->begin(); it != authorList->end(); ++it) {
			if ((*it)->findLowerCase(filterStr))
				filterList->push_back(*it);
		}
    } else {
        const int item_amount = authorMaster->size();
        for (int n=1; n<item_amount; n++) {
            if ((*authorMaster)[n]->findLowerCase(filterStr))
                filterList->push_back((*authorMaster)[n]);
        }
    }
	delete authorList;
	authorList = filterList;
	authorList->sort(author_comp);
}

void DataManager::clearAuthorFilter()
{
	delete authorList;
    makeAuthorList();
}


//******************************************************
//************************ Tags ************************
//******************************************************

void DataManager::addTag(TagEntry* newEntry)
{
	newEntry->dbid = tagMaster.size();
	tagMaster.push_back(newEntry);
	tagList->push_back(newEntry);
	tagList->sort(tag_comp);
}

void DataManager::tagModified(TagEntry* tag)
{
	tagList->sort(tag_comp);
}

void DataManager::deleteTag(TagEntry* tag)
{
	tag->tag = "";
	tagList->remove(tag);
	removeTagFromMaps(tag->dbid);
}

void DataManager::mergeTags(TagEntry* tagDelete, uint16_t tagKeep)
{
	tagDelete->tag = "";
	tagList->remove(tagDelete);
	removeTagFromMaps(tagDelete->dbid, tagKeep);
}

void DataManager::saveTags()
{
	wxLogVerbose("Writing all tags to file %s", FILE_TAGS);
	wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_TAGS);
	if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_TAGS);
	wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 512);
	buf->Write(&tagLength, 1);
	const char* ch;
	for (int i=1; i<tagMaster.size(); i++) {
		ch = tagMaster[i]->tag.c_str();
		for (int j=0; j<tagLength; j++) {
			buf->Write(ch, 1);
			if (*ch != 0)
				ch++;
		}
	}
	delete buf;
	file.Close();
}

void DataManager::loadTags()
{
	tagMaster.clear();
	tagMaster.push_back(new TagEntry(0, "")); //Entry 0 is empty

	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_TAGS);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_TAGS);
		return;
	}
	wxLogVerbose("Reading all tags from file %s", FILE_TAGS);
	wxFile* file = new wxFile(fname, wxFile::read);
	if (!file->IsOpened()) throw GuiError("Couldn't open file.", FILE_TAGS);

	//First byte is tag length
	file->Read(&tagLength, 1);
	char buf[128];
	for (int i=0; i<128; i++)
		buf[i] = 0;
	uint16_t index = 1;
	while (!file->Eof()) {
		file->Read(buf, tagLength);
		string str(buf);
		tagMaster.push_back(new TagEntry(index,str));
		index++;
	}
	file->Close();
	wxLogVerbose("Finished reading %i tags", tagMaster.size()-1);
	delete file;
}

void DataManager::makeTagList()
{
	tagList = new list<TagEntry*>();
	const int item_amount = tagMaster.size();
	for (int n=1; n<item_amount; n++) {
		if (tagMaster.at(n)->tag.length() > 0)
			tagList->push_back(tagMaster.at(n));
	}
	tagList->sort(tag_comp);
}

string DataManager::getTagString(uint16_t dbid)
{
	if (dbid>=0 && dbid<tagMaster.size())
		return tagMaster[dbid]->tag;
	else
		return "";
}

TagEntry* DataManager::getTagEntry(int index)
{
	TagEntry* tag = NULL;
	if ((index>-1) && (index<tagList->size())) {
		list<TagEntry*>::iterator it = tagList->begin();
		for (int i=0; i<index; i++)
			it++;
		tag = *it;
	}
	return tag;
}

int DataManager::getTagIndex(uint16_t dbid)
{
	int index = -1;
	for (list<TagEntry*>::iterator it=tagList->begin(); it != tagList->end(); ++it) {
		index++;
		if ((*it)->dbid == dbid)
			return index;
	}
	return -1;
}

int DataManager::getTagIndex(wxString tagStart)
{
    if (tagStart.Length() == 0)
        return -1;
    int index = -1;
    for (list<TagEntry*>::iterator it=tagList->begin(); it != tagList->end(); ++it) {
        index++;
        if (wxString((*it)->tag).CmpNoCase(tagStart) >= 0)
            return index;
    }
    return index;
}

TagEntry* DataManager::findTag(string tag)
{
	const int item_amount = tagMaster.size();
	for (int n=0; n<item_amount; n++) {
		if (tagMaster.at(n)->tag.compare(tag) == 0)
			return tagMaster.at(n);
	}
	return NULL;
}

void DataManager::filterTagList(wxString filterStr, bool update)
{
	filterStr.MakeLower();
    list<TagEntry*>* filterList = new list<TagEntry*>();
    if (update) {
		for (list<TagEntry*>::iterator it=tagList->begin(); it != tagList->end(); ++it) {
			if (wxString((*it)->tag).MakeLower().Find(filterStr) != wxNOT_FOUND)
				filterList->push_back(*it);
		}
    } else {
        const int item_amount = tagMaster.size();
        for (int n=1; n<item_amount; n++) {
            if ( wxString(tagMaster.at(n)->tag).MakeLower().Find(filterStr) != wxNOT_FOUND)
                filterList->push_back(tagMaster.at(n));
        }
    }
	delete tagList;
	tagList = filterList;
	tagList->sort(tag_comp);
}

void DataManager::clearTagFilter()
{
    delete tagList;
    makeTagList();
}


//***********************************************************************
//************************ Wads&Maps persistence ************************
//***********************************************************************

void DataManager::addWad(WadEntry* newEntry)
{
	newEntry->dbid = nextWadId++;
	newEntry->ownFlags |= OF_MAINNEW;
	newEntry->ownFlags |= OF_OWNNEW;
	if (firstNewWad == -1)
		firstNewWad = wadMaster.size();
	if (firstNewWad == 0)
		wadMod = true; //Must write complete file
	wadMaster.push_back(newEntry);
	searchIter = wadMaster.begin();
	if (currentWadFilter->includes(newEntry)) {
		wadList->add(newEntry);
		wadList->sort(currentWadFilter->sortReverse);
	}
	if (wadMod || wadOwnMod) {
		//We must write both files if we have both modified
		//and new entries.
		wadMod = true;
		wadOwnMod = true;
	}

	if (newEntry->numberOfMaps > 0) {
		MapEntry* mapEntry;
		if (firstNewMap == -1)
			firstNewMap = mapMaster.size();
		if (firstNewMap == 0)
			mapMod = true; //Must write complete file
		for (int i=0; i<newEntry->numberOfMaps; i++) {
			mapEntry = newEntry->mapPointers[i];
			mapEntry->dbid = nextMapId++;
			mapEntry->ownFlags |= OF_MAINNEW;
			mapEntry->ownFlags |= OF_OWNNEW;
			mapMaster.push_back(mapEntry);
			if (currentMapFilter->includes(mapEntry))
				mapList->add(mapEntry);
		}
		mapList->sort(currentMapFilter->sortReverse);
		if (mapMod || mapOwnMod) {
			mapMod = true;
			mapOwnMod = true;
		}
	}
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
}

void DataManager::wadModified(WadEntry* wad)
{
	if (wad->dbid != 0) {
		if (wad->ownFlags&OF_MAINMOD)
			wadMod=true;
		if (wad->ownFlags&OF_OWNMOD)
			wadOwnMod=true;
		MapEntry* mapEntry;
		bool newMaps = false;
		for (int i=0; i<wad->numberOfMaps; i++) {
			mapEntry = wad->mapPointers[i];
			if (mapEntry->dbid == 0) {
				//New map
				if (firstNewMap == -1)
					firstNewMap = mapMaster.size();
				mapEntry->dbid = nextMapId++;
				mapEntry->ownFlags |= OF_MAINNEW;
				mapEntry->ownFlags |= OF_OWNNEW;
				mapMaster.push_back(mapEntry);
				if (currentMapFilter->includes(mapEntry))
					mapList->add(mapEntry);
				newMaps = true;
			} else {
				mapModified(mapEntry);
			}
		}
		if (newMaps) {
			mapList->sort(currentMapFilter->sortReverse);
			if (mapMod || mapOwnMod) {
				mapMod = true;
				mapOwnMod = true;
			}
			listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
		}
	}
}

void DataManager::mapModified(MapEntry* me)
{
	if (me->dbid != 0) {
		if (me->ownFlags&OF_MAINMOD)
			mapMod=true;
		if (me->ownFlags&OF_OWNMOD)
			mapOwnMod=true;
	}
}

bool DataManager::unsavedWadMapChanges()
{
	return (wadMod || wadOwnMod || mapMod || mapOwnMod);
}

void DataManager::writeWad(wxOutputStream* file, WadEntry* entry)
{
	const char* ch;
	unsigned char* uc;

	file->Write(&(entry->dbid), 4);
	file->Write(&(entry->numberOfMaps), 1);
	ch = entry->fileName.c_str();
	file->Write(ch, entry->fileName.length()+1);
	file->Write(&(entry->fileSize), 4);
	uc = entry->md5Digest;
	file->Write(uc, 16);
	ch = entry->extraFiles.c_str();
	file->Write(ch, entry->extraFiles.length()+1);
	file->Write(&(entry->idGames), 4);

	ch = entry->title.c_str();
	file->Write(ch, entry->title.length()+1);
	file->Write(&(entry->year), 2);
	file->Write(&(entry->iwad), 1);
	file->Write(&(entry->engine), 1);
	file->Write(&(entry->playStyle), 1);

	file->Write(&(entry->flags), 2);
	file->Write(&(entry->rating), 1);

	entry->ownFlags &= ~OF_MAINNEW;
	entry->ownFlags &= ~OF_MAINMOD;
}

void DataManager::writeWadOwn(wxOutputStream* file, WadEntry* entry)
{
	//OF_MAINNEW and OF_MAINMOD should be clear
	entry->ownFlags &= ~OF_OWNNEW;
	entry->ownFlags &= ~OF_OWNMOD;

	file->Write(&(entry->dbid), 4);
	file->Write(&(entry->ownRating), 1);
	file->Write(&(entry->ownFlags), 1);
}

void DataManager::writeMap(wxOutputStream* file, MapEntry* entry)
{
	const char* ch;
	const uint32_t nll = 0;
	uint16_t* tagPtr;
	file->Write(&(entry->dbid), 4);
	file->Write(&(entry->wadPointer->dbid), 4);
	ch = entry->name.c_str();
	file->Write(ch, entry->name.length()+1);
	ch = entry->title.c_str();
	file->Write(ch, entry->title.length()+1);
	file->Write(&(entry->basedOn), 4);

	if (entry->author1 == NULL)
		file->Write(&nll, 4);
	else
		file->Write(&(entry->author1->dbid), 4);
	if (entry->author2 == NULL)
		file->Write(&nll, 4);
	else
		file->Write(&(entry->author2->dbid), 4);

	file->Write(&(entry->singlePlayer), 1);
	file->Write(&(entry->cooperative), 1);
	file->Write(&(entry->deathmatch), 1);
	file->Write(&(entry->otherMode), 1);

	file->Write(&(entry->linedefs), 4);
	file->Write(&(entry->sectors), 2);
	file->Write(&(entry->things), 2);
	file->Write(&(entry->secrets), 2);
	file->Write(&(entry->enemies), 2);
	file->Write(&(entry->totalHP), 4);

	string str = LtbUtils::doubleToString(entry->healthRatio, 4);
	ch = str.c_str();
	file->Write(ch, str.length()+1);
	str = LtbUtils::doubleToString(entry->armorRatio, 4);
	ch = str.c_str();
	file->Write(ch, str.length()+1);
	str = LtbUtils::doubleToString(entry->ammoRatio, 4);
	ch = str.c_str();
	file->Write(ch, str.length()+1);
	str = LtbUtils::doubleToString(entry->area, 4);
	ch = str.c_str();
	file->Write(ch, str.length()+1);

	tagPtr = entry->tags;
	file->Write(tagPtr, MAXTAGS*2);
	file->Write(&(entry->rating), 1);
	file->Write(&(entry->flags), 1);

	entry->ownFlags &= ~OF_MAINNEW;
	entry->ownFlags &= ~OF_MAINMOD;
}

void DataManager::writeMapOwn(wxOutputStream* file, MapEntry* entry)
{
	//OF_MAINNEW and OF_MAINMOD should be clear
	entry->ownFlags &= ~OF_OWNNEW;
	entry->ownFlags &= ~OF_OWNMOD;

	file->Write(&(entry->dbid), 4);
	file->Write(&(entry->ownRating), 1);
	//file->Write(&(entry->ownFlags), 1);
	file->Write(&(entry->played), 1);
	file->Write(&(entry->difficulty), 1);
	file->Write(&(entry->playTime), 1);
}

void DataManager::loadWads()
{
	wadMaster.clear();
	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_WADDB);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_WADDB);
		return;
	}
	wxLogVerbose("Reading all wad entries from file %s", FILE_WADDB);
	wxFileInputStream fileStream(fname);
	if (!fileStream.IsOk()) throw GuiError("Couldn't open file.", FILE_WADDB);
	wxBufferedInputStream* buf = new wxBufferedInputStream(fileStream, 1024);

	unsigned char ch;
	uint32_t id;
	buf->Read(&ch, 1);
	if (ch != WADDB_FILEV)
		throw GuiError("Wad database file has unsupported version number", FILE_WADDB);

	WadEntry* entry;
	while (!buf->Eof()) {
		buf->Read(&id, 4);
		buf->Read(&ch, 1);
		if (buf->Eof())
			break;
		entry = new WadEntry(id,ch);
		do { //fileName
			buf->Read(&ch, 1);
			if (ch != 0)
				entry->fileName += ch;

		} while (ch != 0);
		buf->Read(&(entry->fileSize), 4);
		buf->Read(entry->md5Digest, 16);
		do { //extraFiles
			buf->Read(&ch, 1);
			if (ch != 0)
				entry->extraFiles += ch;

		} while (ch != 0);
		buf->Read(&(entry->idGames), 4);

		do { //title
			buf->Read(&ch, 1);
			if (ch != 0)
				entry->title += ch;

		} while (ch != 0);
		buf->Read(&(entry->year), 2);
		buf->Read(&(entry->iwad), 1);
		buf->Read(&(entry->engine), 1);
		buf->Read(&(entry->playStyle), 1);

		buf->Read(&(entry->flags), 2);
		buf->Read(&(entry->rating), 1);
		wadMaster.push_back(entry);
	}
	if (wadMaster.size() == 0) {
		nextWadId = 1;
	} else {
		nextWadId = wadMaster.back()->dbid + 1;
		searchIter = wadMaster.begin();
	}
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	wxLogVerbose("Finished reading %i wad entries", wadMaster.size());
	delete buf;

	// Add personal fields:
	fname = dbFolder+wxFILE_SEP_PATH+FILE_WADOWN;
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_WADOWN);
		return;
	}
	wxFileInputStream fileStream2(fname);
	if (!fileStream2.IsOk()) return;
	buf = new wxBufferedInputStream(fileStream2, 1024);
	wxLogVerbose("Reading personal wad entries from file %s", FILE_WADOWN);

	buf->Read(&ch, 1);
	if (ch != WADDB_FILEV) {
		wxLogVerbose("Wad personal file has unsupported version number");
		delete buf;
		return;
	}

	int index = 0;
	while (!buf->Eof() && index<wadMaster.size()) {
		entry = wadMaster.at(index);
		buf->Read(&id, 4); //Must match entry->dbid
		buf->Read(&(entry->ownRating), 1);
		buf->Read(&(entry->ownFlags), 1);
		index++;
	}
	wxLogVerbose("Finished reading %i wad entries", index);
	delete buf;
}

void DataManager::loadMaps()
{
	mapMaster.clear();
	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_MAPDB);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_MAPDB);
		return;
	}
	wxLogVerbose("Reading all map entries from file %s", FILE_MAPDB);
	wxFileInputStream fileStream(fname);
	if (!fileStream.IsOk()) throw GuiError("Couldn't open file.", FILE_MAPDB);
	wxBufferedInputStream* buf = new wxBufferedInputStream(fileStream, 1024);

	unsigned char ch;
	uint32_t id;
	buf->Read(&ch, 1);
	if (ch != MAPDB_FILEV)
		throw GuiError("Map database file has unsupported version number", FILE_MAPDB);

	MapEntry* entry;
	wxString str;
	double doubl;
	uint16_t* tagPtr;
	while (!buf->Eof()) {
		buf->Read(&id, 4);
		if (buf->Eof())
			break;
		entry = new MapEntry(id);
		buf->Read(&id, 4); //wad id
		do { //name
			buf->Read(&ch, 1);
			if (ch != 0)
				entry->name += ch;

		} while (ch != 0);
		do { //title
			buf->Read(&ch, 1);
			if (ch != 0)
				entry->title += ch;

		} while (ch != 0);
		buf->Read(&(entry->basedOn), 4);

		//Pointer to wad and authors
		entry->wadPointer = getWadMasterEntry(id);
		if (entry->wadPointer==NULL)
			wxLogVerbose("Map %i missing ref to wad %i", entry->dbid, id);
		else
			entry->wadPointer->addMap(entry);
		buf->Read(&id, 4);
		if (id != 0)
			entry->author1 = getAuthorMasterEntry(id);
		buf->Read(&id, 4);
		if (id != 0)
			entry->author2 = getAuthorMasterEntry(id);

		buf->Read(&(entry->singlePlayer), 1);
		buf->Read(&(entry->cooperative), 1);
		buf->Read(&(entry->deathmatch), 1);
		buf->Read(&(entry->otherMode), 1);

		buf->Read(&(entry->linedefs), 4);
		buf->Read(&(entry->sectors), 2);
		buf->Read(&(entry->things), 2);
		buf->Read(&(entry->secrets), 2);
		buf->Read(&(entry->enemies), 2);
		buf->Read(&(entry->totalHP), 4);

		//Floats are stored as strings
		do { //healthRatio
			buf->Read(&ch, 1);
			if (ch != 0)
				str << ch;

		} while (ch != 0);
		str.ToCDouble(&doubl);
		entry->healthRatio = doubl;
		str.Clear();

		do { //armorRatio
			buf->Read(&ch, 1);
			if (ch != 0)
				str << ch;

		} while (ch != 0);
		str.ToCDouble(&doubl);
		entry->armorRatio = doubl;
		str.Clear();

		do { //ammoRatio
			buf->Read(&ch, 1);
			if (ch != 0)
				str << ch;

		} while (ch != 0);
		str.ToCDouble(&doubl);
		entry->ammoRatio = doubl;
		str.Clear();

		do { //area
			buf->Read(&ch, 1);
			if (ch != 0)
				str << ch;

		} while (ch != 0);
		str.ToCDouble(&doubl);
		entry->area = doubl;
		str.Clear();

		tagPtr = entry->tags;
		buf->Read(tagPtr, MAXTAGS*2);
		buf->Read(&(entry->rating), 1);
		buf->Read(&(entry->flags), 1);

		//if (entry->dbid != 2319)
		mapMaster.push_back(entry);
	}
	if (mapMaster.size() == 0) {
		nextMapId = 1;
	} else {
		nextMapId = mapMaster.back()->dbid + 1;
	}
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	wxLogVerbose("Finished reading %i map entries", mapMaster.size());
	delete buf;

	// Add personal fields:
	fname = dbFolder+wxFILE_SEP_PATH+FILE_MAPOWN;
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_MAPOWN);
		return;
	}
	wxFileInputStream fileStream2(fname);
	if (!fileStream2.IsOk()) return;
	buf = new wxBufferedInputStream(fileStream2, 1024);
	wxLogVerbose("Reading personal map entries from file %s", FILE_MAPOWN);

	buf->Read(&ch, 1);
	if (ch != MAPDB_FILEV) {
		wxLogVerbose("Map personal file has unsupported version number");
		delete buf;
		return;
	}

	int index = 0;
	while (!buf->Eof() && index<mapMaster.size()) {
		entry = mapMaster.at(index);
		buf->Read(&id, 4); //Must match entry->dbid
		buf->Read(&(entry->ownRating), 1);
		//buf->Read(&(entry->ownFlags), 1);
		buf->Read(&(entry->played), 1);
		buf->Read(&(entry->difficulty), 1);
		buf->Read(&(entry->playTime), 1);
		index++;
	}
	wxLogVerbose("Finished reading %i map entries", index);
	delete buf;
}

void DataManager::saveWadsMaps()
{
	//NOTE: Can't mix firstNew with Mod/OwnMod, unless BOTH Mod and OwnMod are true

	// WadEntries:
	if (wadMod || wadOwnMod) {
		//Need to save whole list
		if (wadMod) {
			wxLogVerbose("Writing all wad entries to file %s", FILE_WADDB);
			wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_WADDB);
			if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_WADDB);
			wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);
			buf->Write(&WADDB_FILEV, 1);
			for (int i=0; i<wadMaster.size(); i++) {
				writeWad(buf, wadMaster.at(i));
			}
			delete buf;
			file.Close();
		}
		if (wadOwnMod) {
			wxLogVerbose("Writing all wad personal data to file %s", FILE_WADOWN);
			wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_WADOWN);
			if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_WADOWN);
			wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);
			buf->Write(&WADDB_FILEV, 1);
			for (int i=0; i<wadMaster.size(); i++) {
				writeWadOwn(buf, wadMaster.at(i));
			}
			delete buf;
			file.Close();
		}

	} else if (firstNewWad>-1) {
		//Just append new
		wxLogVerbose("Appending new wad entries to file %s", FILE_WADDB);
		wxFile file(dbFolder+wxFILE_SEP_PATH+FILE_WADDB, wxFile::write_append);
		if (!file.IsOpened()) throw GuiError("Couldn't open file.", FILE_WADDB);
		wxFileOutputStream filestr(file);
		wxBufferedOutputStream* buf = new wxBufferedOutputStream(filestr, 512);
		for (int i=firstNewWad; i<wadMaster.size(); i++) {
			writeWad(buf, wadMaster.at(i));
		}
		delete buf;
		file.Close();

		wxLogVerbose("Appending new wad personal data to file %s", FILE_WADOWN);
		wxFile file2(dbFolder+wxFILE_SEP_PATH+FILE_WADOWN, wxFile::write_append);
		if (!file2.IsOpened()) throw GuiError("Couldn't open file.", FILE_WADOWN);
		wxFileOutputStream filestr2(file2);
		buf = new wxBufferedOutputStream(filestr2, 512);
		for (int i=firstNewWad; i<wadMaster.size(); i++) {
			writeWadOwn(buf, wadMaster.at(i));
		}
		delete buf;
		file2.Close();
	}
	wadMod=false;
	wadOwnMod=false;
	firstNewWad=-1;

	// MapEntries:
	if (mapMod || mapOwnMod) {
		//Need to save whole list
		if (mapMod) {
			wxLogVerbose("Writing all map entries to file %s", FILE_MAPDB);
			wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_MAPDB);
			if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_MAPDB);
			wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);
			buf->Write(&MAPDB_FILEV, 1);
			for (int i=0; i<mapMaster.size(); i++) {
				writeMap(buf, mapMaster.at(i));
			}
			delete buf;
			file.Close();
		}
		if (mapOwnMod) {
			wxLogVerbose("Writing all map personal data to file %s", FILE_MAPOWN);
			wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_MAPOWN);
			if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_MAPOWN);
			wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);
			buf->Write(&MAPDB_FILEV, 1);
			for (int i=0; i<mapMaster.size(); i++) {
				writeMapOwn(buf, mapMaster.at(i));
			}
			delete buf;
			file.Close();
		}

	} else if (firstNewMap>-1) {
		//Just append new
		wxLogVerbose("Appending new map entries to file %s", FILE_MAPDB);
		wxFile file(dbFolder+wxFILE_SEP_PATH+FILE_MAPDB, wxFile::write_append);
		if (!file.IsOpened()) throw GuiError("Couldn't open file.", FILE_MAPDB);
		wxFileOutputStream filestr(file);
		wxBufferedOutputStream* buf = new wxBufferedOutputStream(filestr, 512);
		for (int i=firstNewMap; i<mapMaster.size(); i++) {
			writeMap(buf, mapMaster.at(i));
		}
		delete buf;
		file.Close();

		wxLogVerbose("Appending new map personal data to file %s", FILE_MAPOWN);
		wxFile file2(dbFolder+wxFILE_SEP_PATH+FILE_MAPOWN, wxFile::write_append);
		if (!file2.IsOpened()) throw GuiError("Couldn't open file.", FILE_MAPOWN);
		wxFileOutputStream filestr2(file2);
		buf = new wxBufferedOutputStream(filestr2, 512);
		for (int i=firstNewMap; i<mapMaster.size(); i++) {
			writeMapOwn(buf, mapMaster.at(i));
		}
		delete buf;
		file2.Close();
	}
	mapMod=false;
	mapOwnMod=false;
	firstNewMap=-1;
}

WadText* DataManager::getWadText(WadEntry* wad)
{
	string md5str = wad->getMd5String();
	if (wadText != NULL) {
		if (md5str.compare(wadText->wadMd5)==0)
			return wadText;
		else
			delete wadText;
	}

	wadText = new WadText(md5str);
	wxString filePath = getTextFolder()+wxFILE_SEP_PATH+md5str+".txt";
	if (wxFileExists(filePath)) {
		wxLogVerbose("Reading wad text file %s", md5str);
		wxTextFile* tFile = new wxTextFile(filePath);
		if (!tFile->Open()) throw GuiError("Couldn't open text file.", filePath);
		wxString line;
		wxString entryName = "";
		wxString entry = "";
		for ( line=tFile->GetFirstLine(); !tFile->Eof(); line=tFile->GetNextLine() ) {
			if (line.StartsWith("$")) {
				//Marks next entry
				if (entryName.Length() > 0) {
					//Store last entry
					wadText->setText(entryName.ToStdString(), entry);
					entryName.Clear();
					entry.Clear();
				}
				line.StartsWith("$", &entryName); //Gets rest of line
				//wxLogVerbose("Found entry %s", entryName);
			} else {
				//Add to current entry
				entry.Append(line);
			}
		}
		if (entryName.Length() > 0) {
			//Store last entry
			wadText->setText(entryName.ToStdString(), entry);
		}
		wxLogVerbose("Loaded %i text entries", wadText->entries->size());
	}
	wadText->changed = false;
	return wadText;
}

void DataManager::saveWadText(WadText* wt)
{
	if (wadText!=NULL && wadText!=wt)
		delete wadText;
	wadText = wt;
	if (!wadText->hasText())
		return;

	wxString filePath = getTextFolder()+wxFILE_SEP_PATH+wadText->wadMd5+".txt";
	wxLogVerbose("Writing wad text file %s", wadText->wadMd5);
	wxFile* file = new wxFile(filePath, wxFile::write);
	if (!file->IsOpened()) throw GuiError("Couldn't open file.", filePath);

	char nl = '\n';
	char tag = '$';
	if (wadText->entries->size() > 0) {
		for (map<string,wxString>::iterator it=wadText->entries->begin(); it!=wadText->entries->end(); ++it) {
			file->Write(&tag, 1);
			file->Write(it->first);
			file->Write(&nl, 1);
			file->Write(it->second);
			file->Write(&nl, 1);
		}
	}
	file->Close();
	delete file;
	wadText->changed = false;
}

void DataManager::deleteMap(MapEntry* me, long index)
{
	if (me->wadPointer->numberOfMaps < 2) {
		//Delete whole wad
		deleteWad(me->wadPointer);
	} else {
		int i = me->wadPointer->getMapIndex(me);
		if (i==-1) return;
		deleteMapFiles(me);
		//Remove map section from WadText
		WadText* wt = getWadText(me->wadPointer);
		if (wt->removeText(me->name)) {
			//Map section was removed
			//Note that if we only have one map left after deleting,
			//only the wad entry is used.
			if (wt->hasText()) {
				saveWadText(wt);
			} else {
				//Delete file, since there are no entries left
				wxString textPath = getTextFolder()+wxFILE_SEP_PATH+me->wadPointer->getMd5String()+".txt";
				if (wxFileExists(textPath)) {
					if (wxRemoveFile(textPath))
						wxLogVerbose("Deleted text file for wad %s", me->wadPointer->fileName);
					else
						wxLogVerbose("Failed to delete text file for wad %s", me->wadPointer->fileName);
				}
				delete wadText;
				wadText = NULL;
			}
		}
		wxLogVerbose("Delete map with dbid %i", me->dbid);
		me->wadPointer->removeMap(i, false);
		wadMod = true;
		removeBasedOn(me->dbid);
		removeMapFromFilters(me->dbid);
		removeMapMaster(me->dbid);
		if (index >= 0) {
			mapList->setIndex(index);
			mapList->erase();
		} else {
			makeMapList();
		}
		delete me;
		listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
	}
}

void DataManager::deleteWad(WadEntry* we, long index)
{
	if (wadText != NULL) {
		delete wadText;
		wadText = NULL;
	}
	deleteWadFiles(we);
	//Delete maps
	MapEntry* me;
	for (int i=0; i<we->numberOfMaps; i++) {
		me = we->mapPointers.at(i);
		wxLogVerbose("Delete map with dbid %i", me->dbid);
		removeBasedOn(me->dbid);
		removeMapFromFilters(me->dbid);
		removeMapMaster(me->dbid);
		delete me;
	}
	makeMapList();
	//Delete wad
	wxLogVerbose("Delete wad with dbid %i", we->dbid);
	removeWadFromFilters(we->dbid);
	removeWadMaster(we->dbid);
	if (index >= 0) {
		wadList->setIndex(index);
		wadList->erase();
	} else {
		makeWadList();
	}
	delete we;
	listener->onTotalCounts(wadMaster.size(), mapMaster.size(), authorMaster->size());
}

void DataManager::deleteMapFiles(MapEntry* me)
{
	wxString filename = me->fileName();
	wxString imgFile = getMapImgFolder()+wxFILE_SEP_PATH+filename+".png";
	if (wxFileExists(imgFile)) {
		if (wxRemoveFile(imgFile))
			wxLogVerbose("Deleted image file for map with dbid %i", me->dbid);
		else
			wxLogVerbose("Failed to delete image file for map with dbid %i", me->dbid);
	}
	imgFile = getScreenshotFolder()+wxFILE_SEP_PATH+filename+".png";
	if (wxFileExists(imgFile)) {
		if (wxRemoveFile(imgFile))
			wxLogVerbose("Deleted screenshot file for map with dbid %i", me->dbid);
		else
			wxLogVerbose("Failed to delete screenshot file for map with dbid %i", me->dbid);
	}
}

void DataManager::deleteWadFiles(WadEntry* wad)
{
	wxLogVerbose("Deleting files for wad %s", wad->fileName);
	wxString textPath = getTextFolder()+wxFILE_SEP_PATH+wad->getMd5String()+".txt";
	if (wxFileExists(textPath)) {
		if (wxRemoveFile(textPath))
			wxLogVerbose("Deleted text file for wad %s", wad->fileName);
		else
			wxLogVerbose("Failed to delete text file for wad %s", wad->fileName);
	}
	wxString imgFolder = getMapImgFolder();
	wxString scrFolder = getScreenshotFolder();
	for (int i=0; i<wad->numberOfMaps; i++) {
		wxString filename = wad->mapPointers.at(i)->fileName();
		wxString imgFile = imgFolder+wxFILE_SEP_PATH+filename+".png";
		if (wxFileExists(imgFile)) {
			if (wxRemoveFile(imgFile))
				wxLogVerbose("Deleted image file for map %i in wad", i);
			else
				wxLogVerbose("Failed to delete image file for map %i in wad", i);
		}
		imgFile = scrFolder+wxFILE_SEP_PATH+filename+".png";
		if (wxFileExists(imgFile)) {
			if (wxRemoveFile(imgFile))
				wxLogVerbose("Deleted screenshot file for map %i in wad", i);
			else
				wxLogVerbose("Failed to delete screenshot file for map %i in wad", i);
		}
	}
}

void DataManager::renameFiles(WadEntry* wad, string newHash)
{
	wxString oldPath = getTextFolder()+wxFILE_SEP_PATH+wad->getMd5String()+".txt";
	wxString newPath;
	if (wxFileExists(oldPath)) {
		newPath = getTextFolder()+wxFILE_SEP_PATH+newHash+".txt";
		if (wxRenameFile(oldPath, newPath, false))
			wxLogVerbose("Renamed text file for new wad hash %s", newHash);
		else
			wxLogVerbose("Failed to rename text file to new hash %s", newHash);
	}
	wxString imgFolder = getMapImgFolder();
	wxString scrFolder = getScreenshotFolder();
	for (int i=0; i<wad->numberOfMaps; i++) {
		wxString filename = wad->mapPointers.at(i)->fileName();
		wxString newname = wxString(newHash).Append("_").Append(wad->mapPointers.at(i)->name);
		oldPath = imgFolder+wxFILE_SEP_PATH+filename+".png";
		if (wxFileExists(oldPath)) {
			newPath = imgFolder+wxFILE_SEP_PATH+newname+".png";
			if (wxRenameFile(oldPath, newPath, false))
				wxLogVerbose("Renamed map image file to %s", newname);
			else
				wxLogVerbose("Failed to rename map image file to %s", newname);
		}
		oldPath = scrFolder+wxFILE_SEP_PATH+filename+".png";
		if (wxFileExists(oldPath)) {
			newPath = scrFolder+wxFILE_SEP_PATH+newname+".png";
			if (wxRenameFile(oldPath, newPath, false))
				wxLogVerbose("Renamed screenshot file to %s", newname);
			else
				wxLogVerbose("Failed to rename screenshot file to %s", newname);
		}
	}
}

//*********************************************************************
//************************ Wads&Maps in-memory ************************
//*********************************************************************

void DataManager::initDataFilters(DataFilter* wadFilter, DataFilter* mapFilter)
{
	//filters[0] is main filter, filters[1] is for text search
	currentWadFilter->addFilter(wadFilter);
	currentWadFilter->addFilter(new TextSearchFilter(FILTER_WAD, ""));
	currentWadFilter->name = wadFilter->name;
	currentWadFilter->sortField = wadFilter->sortField;
	currentWadFilter->sortReverse = wadFilter->sortReverse;
	makeWadList();
	listener->onWadFilter(currentWadFilter->name, wadList->getSize());

	currentMapFilter->addFilter(mapFilter);
	currentMapFilter->addFilter(new TextSearchFilter(FILTER_MAP, ""));
	currentMapFilter->name = mapFilter->name;
	currentMapFilter->sortField = mapFilter->sortField;
	currentMapFilter->sortReverse = mapFilter->sortReverse;
	makeMapList();
	listener->onMapFilter(currentMapFilter->name, mapList->getSize());
}

bool DataManager::changeDataFilter(DataFilter* dataFilter)
{
	if (dataFilter->type < FILTER_MAP) {//wad
		if (dataFilter != currentWadFilter->filters[0]) {
			//wxLogVerbose("Setting wad filter %s", dataFilter->name);
			currentWadFilter->filters[0] = dataFilter;
			currentWadFilter->isChanged = true;
		}
		if (currentWadFilter->hasChanges()) {
			currentWadFilter->name = dataFilter->name;
			makeWadList();
			listener->onWadFilter(currentWadFilter->name, wadList->getSize());
			return true;
		} else {
			listener->onWadFilter(currentWadFilter->name, wadList->getSize());
			return false;
		}
	} else { //dataFilter->type >= FILTER_MAP
		if (dataFilter != currentMapFilter->filters[0]) {
			//wxLogVerbose("Setting map filter %s", dataFilter->name);
			currentMapFilter->filters[0] = dataFilter;
			currentMapFilter->isChanged = true;
		}
		if (currentMapFilter->hasChanges()) {
			currentMapFilter->name = dataFilter->name;
			makeMapList();
			listener->onMapFilter(currentMapFilter->name, mapList->getSize());
			return true;
		} else {
			listener->onMapFilter(currentMapFilter->name, mapList->getSize());
			return false;
		}
	}
}

void DataManager::setTextFilter(wxString filterStr, int filterType, bool update)
{
	TextSearchFilter* tsf = dynamic_cast<TextSearchFilter*>(currentWadFilter->filters[1]);
	tsf->searchStr = filterStr;
	tsf->isChanged = true;
	tsf = dynamic_cast<TextSearchFilter*>(currentMapFilter->filters[1]);
	tsf->searchStr = filterStr;
	tsf->isChanged = true;
	if (filterType < FILTER_MAP) { //wad
		makeWadList(update);
		listener->onWadFilter(currentWadFilter->name, wadList->getSize());
	} else {
		makeMapList(update);
		listener->onMapFilter(currentMapFilter->name, mapList->getSize());
	}
}

void DataManager::makeWadList(bool update)
{
	if ((wadList != NULL) && !update)
		delete wadList;
	ListWrapper<WadEntry*>* newList = new ListWrapper<WadEntry*>();
	if (currentWadFilter->hasFilter()) {
		DataFilter* activeFilter;
		if (currentWadFilter->hasAllFilters())
			activeFilter = currentWadFilter;
		else if (currentWadFilter->filters[0]->hasFilter())
			activeFilter = currentWadFilter->filters[0];
		else
			activeFilter = currentWadFilter->filters[1];
		if (!update) {
			for (vector<WadEntry*>::iterator it=wadMaster.begin(); it!=wadMaster.end(); ++it) {
				if (activeFilter->includes(*it))
					newList->add(*it);
			}
		} else if (wadList->getSize() > 0) { //Use existing list
			wadList->reset();
			WadEntry* entry = wadList->entry();
			if (activeFilter->includes(entry)) newList->add(entry);
			while (wadList->next()) {
				entry = wadList->entry();
				if (activeFilter->includes(entry)) newList->add(entry);
			}
		}
		currentWadFilter->setChanged(false);
		newList->reset();
	} else {
		newList->fromVector(wadMaster);
	}
	if (update) delete wadList;
	wadList = newList;
	setWadSort(currentWadFilter->sortField, currentWadFilter->sortReverse);
}

void DataManager::makeMapList(bool update)
{
	if ((mapList != NULL) && !update)
		delete mapList;
	ListWrapper<MapEntry*>* newList = new ListWrapper<MapEntry*>();
	if (currentMapFilter->hasFilter()) {
		DataFilter* activeFilter;
		if (currentMapFilter->hasAllFilters())
			activeFilter = currentMapFilter;
		else if (currentMapFilter->filters[0]->hasFilter())
			activeFilter = currentMapFilter->filters[0];
		else
			activeFilter = currentMapFilter->filters[1];
		if (!update) {
			for (vector<MapEntry*>::iterator it=mapMaster.begin(); it!=mapMaster.end(); ++it) {
				if (activeFilter->includes(*it))
					newList->add(*it);
			}
		} else if (mapList->getSize() > 0) { //Use existing list
			mapList->reset();
			MapEntry* entry = mapList->entry();
			if (activeFilter->includes(entry)) newList->add(entry);
			while (mapList->next()) {
				entry = mapList->entry();
				if (activeFilter->includes(entry)) newList->add(entry);
			}
		}
		currentMapFilter->setChanged(false);
		newList->reset();
	} else {
		newList->fromVector(mapMaster);
	}
	if (update) delete mapList;
	mapList = newList;
	setMapSort(currentMapFilter->sortField, currentMapFilter->sortReverse);
}

bool DataManager::setWadSort(WadMapFields sortField, bool reverse)
{
	switch (sortField) {
	case WAD_DBID: wadList->comp = wad_comp_dbid; break;
	case WAD_FILENAME: wadList->comp = wad_comp_fileName; break;
	case WAD_FILESIZE: wadList->comp = wad_comp_fileSize; break;
	//Not WAD_MD5DIGEST, WAD_EXTRAFILES
	case WAD_IDGAMES: wadList->comp = wad_comp_idgames; break;
	case WAD_TITLE: wadList->comp = wad_comp_title; break;
	case WAD_YEAR: wadList->comp = wad_comp_year; break;
	case WAD_IWAD: wadList->comp = wad_comp_iwad; break;
	case WAD_ENGINE: wadList->comp = wad_comp_engine; break;
	case WAD_PLAYSTYLE: wadList->comp = wad_comp_playstyle; break;
	case WAD_MAPS: wadList->comp = wad_comp_maps; break;
	//Not flags: WAD_WF_IWAD, WAD_WF_SPRITES, WAD_WF_TEX, WAD_WF_GFX, WAD_WF_COLOR,
	// WAD_WF_SOUND, WAD_WF_MUSIC, WAD_WF_DEHBEX, WAD_WF_THINGS, WAD_WF_SCRIPT,
	// WAD_WF_GLNODES, WAD_OF_HAVEFILE
	case WAD_RATING: wadList->comp = wad_comp_rating; break;
	case WAD_OWNRATING: wadList->comp = wad_comp_ownrating; break;
	case WAD_PLAYTIME : wadList->comp = wad_comp_playtime; break;
	default: return false;
	}
	currentWadFilter->sortField = sortField;
	currentWadFilter->sortReverse = reverse;
	if (wadList->getIndex() > -1)
		wadList->sort(currentWadFilter->sortReverse);
	return true;
}

bool DataManager::setMapSort(WadMapFields sortField, bool reverse)
{
	switch (sortField) {
	//Wad fields in map list:
	case WAD_FILENAME: mapList->comp = map_comp_filename; break;
	//Not WAD_FILESIZE, WAD_MD5DIGEST, WAD_EXTRAFILES
	case WAD_IDGAMES: mapList->comp = map_comp_idgames; break;
	case WAD_TITLE: mapList->comp = map_comp_wadtitle; break;
	case WAD_YEAR: mapList->comp = map_comp_year; break;
	case WAD_IWAD: mapList->comp = map_comp_iwad; break;
	case WAD_ENGINE: mapList->comp = map_comp_engine; break;
	case WAD_PLAYSTYLE: mapList->comp = map_comp_playstyle; break;
	//Map fields:
	case MAP_DBID: mapList->comp = map_comp_dbid; break;
	case MAP_NAME: mapList->comp = map_comp_name; break;
	case MAP_TITLE: mapList->comp = map_comp_title; break;
	case MAP_AUTHOR1: mapList->comp = map_comp_author1; break;
	case MAP_AUTHOR2: mapList->comp = map_comp_author2; break;
	case MAP_SINGLE: mapList->comp = map_comp_single; break;
	case MAP_COOP: mapList->comp = map_comp_coop; break;
	case MAP_DM: mapList->comp = map_comp_dm; break;
	case MAP_MODE: mapList->comp = map_comp_mode; break;
	case MAP_LINEDEFS: mapList->comp = map_comp_linedefs; break;
	case MAP_SECTORS: mapList->comp = map_comp_sectors; break;
	case MAP_THINGS: mapList->comp = map_comp_things; break;
	case MAP_SECRETS: mapList->comp = map_comp_secrets; break;
	case MAP_ENEMIES: mapList->comp = map_comp_enemies; break;
	case MAP_TOTALHP: mapList->comp = map_comp_totalhp; break;
	case MAP_AMMORAT: mapList->comp = map_comp_ammorat; break;
	case MAP_HEALTHRAT: mapList->comp = map_comp_healthrat; break;
	case MAP_ARMORRAT: mapList->comp = map_comp_armorrat; break;
	case MAP_AREA: mapList->comp = map_comp_area; break;
	case MAP_LINEDEF_DENS: mapList->comp = map_comp_lddens; break;
	case MAP_ENEMY_DENS: mapList->comp = map_comp_enemydens; break;
	case MAP_HP_DENS: mapList->comp = map_comp_hpdens; break;
	case MAP_RATING: mapList->comp = map_comp_rating; break;
	//Not flags: MAP_MF_SPAWN, MAP_MF_MORESPAWN, MAP_MF_DIFFSET, MAP_MF_VOODOO, MAP_MF_UNKNOWN
	case MAP_OWNRATING: mapList->comp = map_comp_ownrating; break;
	//case MAP_PLAYED: currently only showed as flag
	case MAP_DIFFICULTY: mapList->comp = map_comp_difficulty; break;
	case MAP_PLAYTIME: mapList->comp = map_comp_playtime; break;
	default: return false;
	}
	currentMapFilter->sortField = sortField;
	currentMapFilter->sortReverse = reverse;
	if (mapList->getIndex() > -1)
		mapList->sort(currentMapFilter->sortReverse);
	return true;
}

WadEntry* DataManager::getWad(long index)
{
	wadList->setIndex(index);
	return wadList->entry();
}

int DataManager::getWadIndex(wxString strStart)
{
	if ((strStart.Length()==0) || (wadList->getSize()==0))
        return -1;
    int index = -1;
    if (currentWadFilter->sortField == WAD_FILENAME) {
		wadList->reset();
		if ( wxString(wadList->entry()->fileName).CmpNoCase(strStart) >= 0)
			index = wadList->getIndex();
		while (wadList->next() && index==-1) {
			if ( wxString(wadList->entry()->fileName).CmpNoCase(strStart) >= 0)
				index = wadList->getIndex();
		}
    } else if (currentWadFilter->sortField == WAD_TITLE) {
		wadList->reset();
		if ( wxString(wadList->entry()->title).CmpNoCase(strStart) >= 0)
			index = wadList->getIndex();
		while (wadList->next() && index==-1) {
			if ( wxString(wadList->entry()->title).CmpNoCase(strStart) >= 0)
				index = wadList->getIndex();
		}
    }
    return index;
}

WadEntry* DataManager::findWad(uint32_t id)
{
	for (vector<WadEntry*>::iterator it=wadMaster.begin(); it != wadMaster.end(); ++it) {
		if ((*it)->dbid == id)
			return *it;
	}
	return NULL;
}

WadEntry* DataManager::findWad(unsigned char* md5Digest)
{
	for (vector<WadEntry*>::iterator it=wadMaster.begin(); it != wadMaster.end(); ++it) {
		if ((*it)->matchMd5(md5Digest))
			return *it;
	}
	return NULL;
}

WadEntry* DataManager::findWad(string fileName)
{
	for (vector<WadEntry*>::iterator it=wadMaster.begin(); it != wadMaster.end(); ++it) {
		if ((*it)->fileName.compare(fileName) == 0)
			return *it;
	}
	return NULL;
}

MapEntry* DataManager::getMap(long index)
{
	mapList->setIndex(index);
	return mapList->entry();
}

int DataManager::getMapIndex(wxString strStart)
{
	if ((strStart.Length()==0) || (mapList->getSize()==0))
        return -1;
    int index = -1;
    if (currentMapFilter->sortField == MAP_NAME) {
		mapList->reset();
		if ( wxString(mapList->entry()->name).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			if ( wxString(mapList->entry()->name).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    } else if (currentMapFilter->sortField == MAP_TITLE) {
		mapList->reset();
		if ( wxString(mapList->entry()->title).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			if ( wxString(mapList->entry()->title).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    } else if (currentMapFilter->sortField == MAP_AUTHOR1) {
		mapList->reset();
		AuthorEntry* ae = mapList->entry()->author1;
		if (ae!=NULL && getAuthorString(ae).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			ae = mapList->entry()->author1;
			if (ae!=NULL && getAuthorString(ae).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    } else if (currentMapFilter->sortField == MAP_AUTHOR2) {
		mapList->reset();
		AuthorEntry* ae = mapList->entry()->author2;
		if (ae!=NULL && getAuthorString(ae).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			ae = mapList->entry()->author2;
			if (ae!=NULL && getAuthorString(ae).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    } else if (currentMapFilter->sortField == WAD_FILENAME) {
		mapList->reset();
		if ( wxString(mapList->entry()->wadPointer->fileName).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			if ( wxString(mapList->entry()->wadPointer->fileName).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    } else if (currentMapFilter->sortField == WAD_TITLE) {
		mapList->reset();
		if ( wxString(mapList->entry()->wadPointer->title).CmpNoCase(strStart) >= 0)
			index = mapList->getIndex();
		while (mapList->next() && index==-1) {
			if ( wxString(mapList->entry()->wadPointer->title).CmpNoCase(strStart) >= 0)
				index = mapList->getIndex();
		}
    }
    return index;
}

MapEntry* DataManager::findMap(uint32_t id)
{
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it != mapMaster.end(); ++it) {
		if ((*it)->dbid == id)
			return *it;
	}
	return NULL;
}

int DataManager::scaleRating(unsigned char rat)
{
	//So far just 0-10 scale
	return (rat>100)? 11: rat/10;
}

unsigned char DataManager::ratingScale(int scaled)
{
	return (scaled>10)? 255: scaled*=10;
}

wxString DataManager::getWadField(WadEntry* wad, WadMapFields field)
{
	switch (field) {
	case WAD_DBID:
		return LtbUtils::intToString(wad->dbid);
	case WAD_FILENAME:
		return wad->fileName;
	case WAD_FILESIZE:
		return LtbUtils::intToString(wad->fileSize);
	case WAD_MD5DIGEST:
		return wad->getMd5String();
	case WAD_EXTRAFILES:
		return wad->extraFiles;
	case WAD_IDGAMES:
		return LtbUtils::intToString(wad->idGames);
	case WAD_TITLE:
		return wad->title;
	case WAD_YEAR:
		return LtbUtils::intToString(wad->year);
	case WAD_IWAD:
		return iwadNames[wad->iwad];
	case WAD_ENGINE:
		return engineNames[wad->engine];
	case WAD_PLAYSTYLE:
		return playStyles[wad->playStyle];
	case WAD_MAPS:
		return LtbUtils::intToString(wad->numberOfMaps);
	case WAD_WF_IWAD:
		return ((wad->flags&WF_IWAD)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_SPRITES:
		return ((wad->flags&WF_SPRITES)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_TEX:
		return ((wad->flags&WF_TEX)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_GFX:
		return ((wad->flags&WF_GFX)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_COLOR:
		return ((wad->flags&WF_COLOR)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_SOUND:
		return ((wad->flags&WF_SOUND)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_MUSIC:
		return ((wad->flags&WF_MUSIC)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_DEHBEX:
		return ((wad->flags&WF_DEHBEX)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_THINGS:
		return ((wad->flags&WF_THINGS)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_SCRIPT:
		return ((wad->flags&WF_SCRIPT)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_WF_GLNODES:
		return ((wad->flags&WF_GLNODES)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case WAD_RATING:
		return wad->isRated()? LtbUtils::intToString(scaleRating(wad->rating)): "-";
	case WAD_OWNRATING:
		return wad->isOwnRated()? LtbUtils::intToString(scaleRating(wad->ownRating)): "-";
	case WAD_PLAYTIME:
		return LtbUtils::intToString(wad->getTotalPlaytime());
	case WAD_OF_HAVEFILE:
		return ((wad->ownFlags&OF_HAVEFILE)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	}
	return "";
}

wxString DataManager::getMapField(MapEntry* me, WadMapFields field)
{
	if (field < WAD_END_MAP_START)
		return getWadField(me->wadPointer, field);

	switch (field) {
	case WAD_END_MAP_START:
		return me->wadPointer->getName();
	case MAP_DBID:
		return LtbUtils::intToString(me->dbid);
	case MAP_NAME:
		return me->name;
	case MAP_TITLE:
		return me->title;
	case MAP_BASEDON:
		return ""; //TODO: pointer to MapEntry
	case MAP_AUTHOR1:
		return (me->author1==NULL? "": getAuthorString(me->author1));
	case MAP_AUTHOR2:
		return (me->author2==NULL? "": getAuthorString(me->author2));
	case MAP_SINGLE:
		return gameModeSupport[me->singlePlayer];
	case MAP_COOP:
		return gameModeSupport[me->cooperative];
	case MAP_DM:
		return gameModeSupport[me->deathmatch];
	case MAP_MODE:
		return otherGameModes[me->otherMode];
	case MAP_LINEDEFS:
		return LtbUtils::intToString(me->linedefs);
	case MAP_SECTORS:
		return LtbUtils::intToString(me->sectors);
	case MAP_THINGS:
		return LtbUtils::intToString(me->things);
	case MAP_SECRETS:
		return LtbUtils::intToString(me->secrets);
	case MAP_ENEMIES:
		return LtbUtils::intToString(me->enemies);
	case MAP_TOTALHP:
		return LtbUtils::intToString(me->totalHP);
	case MAP_AMMORAT:
		return LtbUtils::doubleToString(me->ammoRatio, 2);
	case MAP_HEALTHRAT:
		return LtbUtils::doubleToString(me->healthRatio, 2);
	case MAP_ARMORRAT:
		return LtbUtils::doubleToString(me->armorRatio, 2);
	case MAP_AREA:
		return LtbUtils::doubleToString(me->area, 3);
	case MAP_LINEDEF_DENS:
		return LtbUtils::doubleToString(me->linedefDensity(), 2);
	case MAP_ENEMY_DENS:
		return LtbUtils::doubleToString(me->enemyDensity(), 2);
	case MAP_HP_DENS:
		return LtbUtils::doubleToString(me->hitpointDensity(), 2);
	case MAP_RATING:
		return me->isRated()? LtbUtils::intToString(scaleRating(me->rating)): "-";
	case MAP_MF_SPAWN:
		if (me->flags&MF_MORESPAWN) return FLAGSTR_TRUE+FLAGSTR_TRUE;
		else if (me->flags&MF_SPAWN) return FLAGSTR_TRUE;
		else return FLAGSTR_FALSE;
	case MAP_MF_MORESPAWN:
		return ((me->flags&MF_MORESPAWN)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_MF_DIFFSET:
		return ((me->flags&MF_DIFFSET)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_MF_VOODOO:
		return ((me->flags&MF_VOODOO)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_MF_UNKNOWN:
		return ((me->flags&MF_UNKNOWN)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_MF_SAMEAS:
		return ((me->flags&MF_SAMEAS)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_MF_NOTLEVEL:
		return ((me->flags&MF_NOTLEVEL)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_OWNRATING:
		return me->isOwnRated()? LtbUtils::intToString(scaleRating(me->ownRating)): "-";
	case MAP_PLAYED:
		return ((me->played>0)? FLAGSTR_TRUE: FLAGSTR_FALSE);
	case MAP_DIFFICULTY:
		return LtbUtils::intToString(me->difficulty);
	case MAP_PLAYTIME:
		return LtbUtils::intToString(me->playTime);
	}
	return "";
}

WadEntry* DataManager::getWadMasterEntry(uint32_t id)
{
	if (wadMaster.size() == 0)
		return NULL;
	uint32_t startId = (*searchIter)->dbid;
	while (searchIter != wadMaster.end()) {
		if ((*searchIter)->dbid == id)
			return *searchIter;
		searchIter++;
	}
	//Reached end, search from start to startId
	searchIter = wadMaster.begin();
	while ( (*searchIter)->dbid != startId) {
		if ((*searchIter)->dbid == id)
			return *searchIter;
		searchIter++;
	}
	return NULL;
}

void DataManager::removeWadMaster(uint32_t id)
{
	for (vector<WadEntry*>::iterator it=wadMaster.begin(); it != wadMaster.end(); ++it) {
		if ((*it)->dbid == id) {
			wadMaster.erase(it);
			searchIter = wadMaster.begin();
			wadMod = true;
			wadOwnMod = true;
			return;
		}
	}
}

void DataManager::removeMapMaster(uint32_t id)
{
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it != mapMaster.end(); ++it) {
		if ((*it)->dbid == id) {
			mapMaster.erase(it);
			mapMod = true;
			mapOwnMod = true;
			return;
		}
	}
}

void DataManager::removeTagFromMaps(uint16_t tagId, uint16_t repId)
{
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it != mapMaster.end(); ++it) {
		for (int i=0; i<MAXTAGS; i++) {
			if ((*it)->tags[i] == tagId)
				(*it)->tags[i] = repId;
		}
	}
	mapMod = true;
}

void DataManager::removeAuthorFromMaps(AuthorEntry* auth, AuthorEntry* repAuth)
{
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it != mapMaster.end(); ++it) {
		if ((*it)->author1 == auth)
			(*it)->author1 = repAuth;
		if ((*it)->author2 == auth)
			(*it)->author2 = repAuth;
	}
	mapMod = true;
}

void DataManager::removeBasedOn(uint32_t dbid)
{
	for (vector<MapEntry*>::iterator it=mapMaster.begin(); it != mapMaster.end(); ++it) {
		if ((*it)->basedOn == dbid)
			(*it)->basedOn = 0;
	}
	mapMod = true;
}

ListWrapper<WadEntry*>* DataManager::getWadTitleList(wxString filterStr, ListWrapper<WadEntry*>* fromOld)
{
	if (wadTitleFilter == NULL)
		wadTitleFilter = new TitleSearchFilter(FILTER_WAD, filterStr);
	else
		wadTitleFilter->searchStr = filterStr;
	ListWrapper<WadEntry*>* newList = new ListWrapper<WadEntry*>();
	if (wadTitleFilter->hasFilter()) {
		if (fromOld == NULL) {
			for (vector<WadEntry*>::iterator it=wadMaster.begin(); it!=wadMaster.end(); ++it) {
				if (wadTitleFilter->includes(*it))
					newList->add(*it);
			}
		} else if (fromOld->getSize() > 0) {
			fromOld->reset();
			WadEntry* entry = fromOld->entry();
			if (wadTitleFilter->includes(entry)) newList->add(entry);
			while (fromOld->next()) {
				entry = fromOld->entry();
				if (wadTitleFilter->includes(entry)) newList->add(entry);
			}
		}
		newList->reset();
	} else {
		newList->fromVector(wadMaster);
	}
	newList->comp = wad_comp_title;
	if (newList->getIndex() > -1)
		newList->sort(false);
	return newList;
}

//*************************************************************
//************************ DataFilters ************************
//*************************************************************

void DataManager::saveDataFilters()
{
	//TODO: Only if dataViewMod
	wxLogVerbose("Writing data filters to file %s", FILE_VIEWS);
	wxFileOutputStream file(dbFolder+wxFILE_SEP_PATH+FILE_VIEWS);
	if (!file.IsOk()) throw GuiError("Couldn't open file.", FILE_VIEWS);
	wxBufferedOutputStream* buf = new wxBufferedOutputStream(file, 1024);

	buf->Write(&VIEWS_FILEV, 1);
	//writeDataFilter(file, mainWadView);
	//writeDataFilter(file, mainMapView);
	for (list<DataListFilter*>::iterator it=wadLists->begin(); it != wadLists->end(); ++it) {
		writeDataFilter(buf, (*it));
	}
	for (list<DataListFilter*>::iterator it=mapLists->begin(); it != mapLists->end(); ++it) {
		writeDataFilter(buf, (*it));
	}
	delete buf;
	file.Close();
	dataViewMod = false;
}

void DataManager::writeDataFilter(wxOutputStream* file, DataFilter* filter)
{
	file->Write(&(filter->type), 1);
	wxScopedCharBuffer buf = filter->name.utf8_str();
	uint16_t l = buf.length();
	file->Write(&l, 2);
	if (l > 0) file->Write(buf.data(), l);

	/*file->Write(&(view->fieldCount), 1);
	for (int i=0; i<view->fieldCount; i++) {
		fld = view->fields[i];
		file->Write(&fld, 1);
		file->Write(&(view->width[i]), 2);
	}*/
	unsigned char fld = filter->sortField;
	file->Write(&fld, 1);

	if (filter->type==FILTER_WAD_LIST || filter->type==FILTER_MAP_LIST) {
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(filter);
		uint32_t id;
		if (dlv->entries->size() > 0) {
			for (list<uint32_t>::iterator it=dlv->entries->begin(); it!=dlv->entries->end(); ++it) {
				id = *it;
				file->Write(&id, 4);
			}
		}
		id = 0;
		file->Write(&id, 4);
	}
}

bool DataManager::loadDataFilters()
{
	wxString fname(dbFolder+wxFILE_SEP_PATH+FILE_VIEWS);
	if (!wxFile::Exists(fname)) {
		wxLogVerbose("File %s not found", FILE_VIEWS);
		return false;
	}
	wxLogVerbose("Reading data filters from file %s", FILE_VIEWS);
	wxFileInputStream fileStream(fname);
	if (!fileStream.IsOk()) throw GuiError("Couldn't open file.", FILE_VIEWS);
	wxBufferedInputStream* file = new wxBufferedInputStream(fileStream, 1024);

	unsigned char ch;
	file->Read(&ch, 1);
	if (ch != VIEWS_FILEV)
		throw GuiError("List database file has unsupported version number", FILE_VIEWS);

	DataFilter* filter;
	DataListFilter* dlv;
	uint16_t l;
	char buf[128];
	unsigned char fld;
	uint32_t id;
	while (!file->Eof()) {
		file->Read(&ch, 1); //Type
		if (file->Eof())
			break;
		if (ch==FILTER_WAD_LIST || ch==FILTER_MAP_LIST) {
			dlv = new DataListFilter(ch);
			filter = dlv;
		} else {
			filter = new DataFilter(ch);
		}
		file->Read(&l, 2);
		if (l > 0) {
			file->Read(buf, l);
			filter->name = wxString::FromUTF8(buf,l);
		}
		/*
		file->Read(&(view->fieldCount), 1);
		for (int i=0; i<view->fieldCount; i++) {
			file->Read(&fld, 1);
			view->fields[i] = (WadMapFields)fld;
			file->Read(&(view->width[i]), 2);
		}*/
		file->Read(&fld, 1);
		filter->sortField = (WadMapFields)fld;

		if (ch==FILTER_WAD_LIST || ch==FILTER_MAP_LIST) {
			file->Read(&id, 4);
			while (id != 0) {
				dlv->entries->push_back(id);
				file->Read(&id, 4);
			}
			//dlv->entries->sort();
			if (ch==FILTER_WAD_LIST)
				wadLists->push_back(dlv);
			else
				mapLists->push_back(dlv);
		}
	}
	wadLists->sort(filter_comp);
	mapLists->sort(filter_comp);
	dataViewMod = false;
	//wxLogVerbose("Finished reading %i map entries", index);
	delete file;
	return true;
}

void DataManager::addDataFilter(DataFilter* dataFilter)
{
	if (dataFilter->type == FILTER_WAD_LIST) {
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(dataFilter);
		if (dlv != NULL) {
			wadLists->push_back(dlv);
			//wadLists->sort(filter_comp);
			dataViewMod = true;
		}
	} else if (dataFilter->type == FILTER_MAP_LIST) {
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(dataFilter);
		if (dlv != NULL) {
			mapLists->push_back(dlv);
			//mapLists->sort(filter_comp);
			dataViewMod = true;
		}
	}
}

void DataManager::removeDataFilter(DataFilter* dataFilter)
{
	if (dataFilter->type == FILTER_WAD_LIST) {
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(dataFilter);
		wadLists->remove(dlv);
	} else if (dataFilter->type == FILTER_MAP_LIST) {
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(dataFilter);
		mapLists->remove(dlv);
	}
	dataViewMod = true;
}

bool DataManager::removeFromListFilter(int type, long index)
{
	bool result = false;
	if (type==0 && currentWadFilter->filters[0]->type==FILTER_WAD_LIST) {
		WadEntry* we = getWad(index);
		if (we==NULL) return false;
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(currentWadFilter->filters[0]);
		result = dlv->removeEntry(we->dbid);
	} else if (type==1 && currentMapFilter->filters[0]->type==FILTER_MAP_LIST) {
		MapEntry* me = getMap(index);
		if (me==NULL) return false;
		DataListFilter* dlv = dynamic_cast<DataListFilter*>(currentMapFilter->filters[0]);
		result = dlv->removeEntry(me->dbid);
	}
	if (result) dataViewMod = true;
	return result;
}

bool DataManager::removeWadFromFilters(uint32_t dbid)
{
	bool changed = false;
	for (list<DataListFilter*>::iterator it=wadLists->begin(); it != wadLists->end(); ++it) {
		if ((*it)->removeEntry(dbid))
			changed = true;
	}
	if (changed) dataViewMod = true;
	return changed;
}

bool DataManager::removeMapFromFilters(uint32_t dbid)
{
	bool changed = false;
	for (list<DataListFilter*>::iterator it=mapLists->begin(); it != mapLists->end(); ++it) {
		if ((*it)->removeEntry(dbid))
			changed = true;
	}
	if (changed) dataViewMod = true;
	return changed;
}

/*
DataFilter* DataManager::getWadFilter(wxString name)
{
	for (int i=0; i<wadLists->size(); i++) {
		if (wadLists->at(i)->name.compare(name) == 0)
			return wadLists->at(i);
	}
	return NULL;
}

DataFilter* DataManager::getMapFilter(wxString name)
{
	for (int i=0; i<mapLists->size(); i++) {
		if (mapLists->at(i)->name.compare(name) == 0)
			return mapLists->at(i);
	}
	return NULL;
}*/

//************************************************************
//************************ Statistics ************************
//************************************************************

MapStatistics* DataManager::getStatistics(int filterType)
{
	MapStatistics* ms;
	if (filterType == 0) {
		ms = new MapStatistics(currentWadFilter->name);
		if (wadList->getSize() > 0) {
			wadList->reset();
			WadEntry* we = wadList->entry();
			ms->processWad(we);
			while (wadList->next()) {
				we = wadList->entry();
				ms->processWad(we);
			}
			ms->computeResults();
		}
	} else {
		ms = new MapStatistics(currentMapFilter->name);
		if (mapList->getSize() > 0) {
			mapList->reset();
			MapEntry* me = mapList->entry();
			ms->processMap(me);
			while (mapList->next()) {
				me = mapList->entry();
				ms->processMap(me);
			}
			ms->computeResults();
		}
	}
	return ms;
}

WadStatistics* DataManager::getWadStatistics()
{
	WadStatistics* ws = new WadStatistics(currentWadFilter->name);
	if (wadList->getSize() > 0) {
		wadList->reset();
		WadEntry* we = wadList->entry();
		ws->processWad(we);
		while (wadList->next()) {
			we = wadList->entry();
			ws->processWad(we);
		}
		ws->computeResults();
	}
	return ws;
}

void DataManager::makeStatisticsWads(StatisticSet* sset)
{
	if (wadList->getSize() > 0) {
		wadList->reset();
		WadEntry* we = wadList->entry();
		sset->processWad(we);
		while (wadList->next()) {
			we = wadList->entry();
			sset->processWad(we);
		}
		sset->computeResults();
	}
}

void DataManager::makeStatisticsMaps(StatisticSet* sset)
{
	if (mapList->getSize() > 0) {
		mapList->reset();
		MapEntry* me = mapList->entry();
		sset->processMap(me);
		while (mapList->next()) {
			me = mapList->entry();
			sset->processMap(me);
		}
		sset->computeResults();
	}
}

StatisticSet* DataManager::getYearStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new YearStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new YearStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getIwadStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new IwadStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new IwadStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getEngineStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new EngineStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new EngineStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getRatingStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new RatingStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new RatingStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getDifficultyStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new DifficultyStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new DifficultyStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getPlaystyleStats(int filterType)
{
	StatisticSet* ss;
	if (filterType == 0) {
		ss = new PlaystyleStatSet(currentWadFilter->name);
		makeStatisticsWads(ss);
	} else {
		ss = new PlaystyleStatSet(currentMapFilter->name);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getAuthorStats(int filterType)
{
	map<int, MapStatistics*>* authorStats = new map<int, MapStatistics*>();
	AuthorEntry* ae;
	for (int i=0; i<authorMaster->size(); i++) {
		ae = (*authorMaster)[i];
		MapStatistics* ms = new MapStatistics(getAuthorString(ae));
		(*authorStats)[ae->dbid] = ms;
	}

	StatisticSet* ss;
	if (filterType == 0) {
		ss = new AuthorStatSet(currentWadFilter->name, authorStats);
		makeStatisticsWads(ss);
	} else {
		ss = new AuthorStatSet(currentMapFilter->name, authorStats);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getTagStats(int filterType)
{
	map<int, MapStatistics*>* tagStats = new map<int, MapStatistics*>();
	TagEntry* te;
	for (int i=0; i<tagMaster.size(); i++) {
		te = tagMaster.at(i);
		if (te->tag.length() > 0) {
			MapStatistics* ms = new MapStatistics(te->tag);
			(*tagStats)[te->dbid] = ms;
		}
	}

	StatisticSet* ss;
	if (filterType == 0) {
		ss = new TagStatSet(currentWadFilter->name, tagStats);
		makeStatisticsWads(ss);
	} else {
		ss = new TagStatSet(currentMapFilter->name, tagStats);
		makeStatisticsMaps(ss);
	}
	return ss;
}

StatisticSet* DataManager::getYearWadStats()
{
	StatisticSet* ss = new YearWadStatSet(currentWadFilter->name);
	makeStatisticsWads(ss);
	return ss;
}

StatisticSet* DataManager::getIwadWadStats()
{
	StatisticSet* ss = new IwadWadStatSet(currentWadFilter->name);
	makeStatisticsWads(ss);
	return ss;
}

StatisticSet* DataManager::getEngineWadStats()
{
	StatisticSet* ss = new EngineWadStatSet(currentWadFilter->name);
	makeStatisticsWads(ss);
	return ss;
}

StatisticSet* DataManager::getRatingWadStats()
{
	StatisticSet* ss = new RatingWadStatSet(currentWadFilter->name);
	makeStatisticsWads(ss);
	return ss;
}
