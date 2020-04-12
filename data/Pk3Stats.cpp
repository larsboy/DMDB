#include "Pk3Stats.h"

Pk3Stats::Pk3Stats(wxString file, wxString tempFolder)
: WadStats(file), tempPath(tempFolder)
{
	wadType = "pk3";
	engine = DENG_ZDOOM;
	extracted = new vector<wxString>();
}

Pk3Stats::~Pk3Stats()
{
	delete extracted;
}

void Pk3Stats::cleanup()
{
	for (int i=0; i<extracted->size(); i++) {
		if (wxRemoveFile(extracted->at(i)))
			wxLogVerbose("Deleted %s", extracted->at(i));
		else
			wxLogVerbose("Could not delete %s", extracted->at(i));
	}
	extracted->clear();
}

void Pk3Stats::readFile(TaskProgress* progress, bool findMd5)
{
	wxFile file(filePathName, wxFile::read);
	if (!file.IsOpened()) {
		progress->fatalError(wxString::Format("Couldn't open file %s",fileName));
		return;
	}
	fileSize = file.Length();
	wxLogVerbose("Processing file %s of size %i", fileName, fileSize);
	progress->startCount((fileSize/8)+200);
	if (findMd5) makeMd5(file);
	progress->incrCount(100);
	file.Close();

	DirEntry* dir;
	numberOfLumps = 0;

	wxFFileInputStream in(filePathName);
	wxZipInputStream zip(in);
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry != NULL) {
		if (!entry->IsDir()) {
			numberOfLumps++;
			dir = new DirEntry();
			dir->name = entry->GetName();
			dir->size = entry->GetSize(); //wxFileOffset
			if (processFile(dir, zip)) {
				//Extract file
				extractFile(zip, entry, progress); //Can fail
			}
			progress->incrCount(dir->size / 8);
			delete dir;
		}
		delete entry;
		if (progress->hasFailed())
			return;
		entry = zip.GetNextEntry();
	}

	if (content[WDECORATE]!=NULL && content[WUNKNOWN]!=NULL) {
		TaskProgress* sub = new TaskProgress("", progress);
		findLumpIncludes(WDECORATE, sub);
		delete sub;
	}
	if (content[WMAPINFO]!=NULL && content[WUNKNOWN]!=NULL) {
		TaskProgress* sub = new TaskProgress("", progress);
		findLumpIncludes(WMAPINFO, sub);
		delete sub;
	}
	if (content[WZSCRIPT]!=NULL && content[WUNKNOWN]!=NULL) {
		TaskProgress* sub = new TaskProgress("", progress);
		findLumpIncludes(WZSCRIPT, sub);
		delete sub;
	}
	progress->completeCount();

	if (content[WWAD] != NULL) {
		//Process each extracted wad file
		WadContentX* wcx = dynamic_cast<WadContentX*>(content[WWAD]);
		for (int j=0; j<wcx->lumps->size(); j++) {
			dir = wcx->lumps->at(j);
			TaskProgress* sub = new TaskProgress(wxString::Format("Analysing contents of %s",dir->name), progress);
			processWad(dir, sub);
			bool failed = sub->hasFailed(true);
			delete sub;
			if (failed) return;
		}
	}
	checkIwadEngine();
	if (!progress->hasFailed() && (content[WERROR]!=NULL)) {
		//Log number of lump errors
		int errCount = 0;
		WadContentX* wcx = dynamic_cast<WadContentX*>(content[WERROR]);
		while (wcx != NULL) {
			errCount++;
			wcx = wcx->next;
		}
		progress->warnError(wxString::Format("%i lump errors",errCount));
	}
}

WadContentX* Pk3Stats::findUnknownLumps(vector<wxString>* inclNames, WadContentType wct, TaskProgress* progress)
{
	WadContentX* unkn = dynamic_cast<WadContentX*>(content[WUNKNOWN]);
	WadContentX* result = NULL;
	WadContentX* newFile;
	DirEntry* dir;
	for (int i=0; i<inclNames->size(); i++) {
		wxString inclfile = inclNames->at(i);
		//NOTE: Windows-style file path
		if (inclfile.Find('/') != wxNOT_FOUND)
			inclfile.Replace("/", "\\");
		dir = findFileEntry(unkn->lumps, inclfile);
		if (dir != NULL) { //Found in UNKNOWN entries
			wxLogVerbose("File %s recognized as known type", dir->name);
			if (wct==WDECORATE || wct==WMAPINFO) {
				//Extract files for these types, as we want to process them all
				wxString path = extractFile(dir->name, progress);
				newFile = new WadContentX(wct, path);
			} else {
				newFile = new WadContentX(wct, "");
			}
			newFile->addLump(dir);
			if (result == NULL) result=newFile;
			else result->mergeContent(newFile);
			dir->size = -1; //To delete
		} else {
			wxLogVerbose("Did not find file for include %s", inclfile);
		}
	}
	deleteUnknown(); //Deletes entries with -1
	return result;
}

DirEntry* Pk3Stats::findFileEntry(vector<DirEntry*>* dirEntries, wxString name)
{
	DirEntry* de;
	for (int i=0; i<dirEntries->size(); i++) {
		de = dirEntries->at(i);
		if (name.IsSameAs(de->name, false))
			return de;
	}
	if (name.Find('.') == wxNOT_FOUND) {
		//Try with extensions
		wxString txtName = name+".txt";
		for (int i=0; i<dirEntries->size(); i++) {
			de = dirEntries->at(i);
			if (txtName.IsSameAs(de->name, false))
				return de;
		}
		wxString lmpName = name+".lmp";
		for (int i=0; i<dirEntries->size(); i++) {
			de = dirEntries->at(i);
			if (lmpName.IsSameAs(de->name, false))
				return de;
		}
	}
	return NULL;
}

bool Pk3Stats::processFile(DirEntry* dir, wxZipInputStream& file)
{
	wxLogVerbose("Archive entry %s of size %i", dir->name, dir->size);
	wxFileName pathName = wxFileName(dir->name);
	if (pathName.GetDirCount() > 0) {
		//Get type from dir
		bool result = processDirFile(dir, pathName);
		if (result) return false;
	}
	wxString fext = pathName.GetExt();
	if (fext.IsSameAs("wad", false)) {
		if (content[WWAD] == NULL)
			content[WWAD] = new WadContentX(WWAD, "");
		content[WWAD]->addLump(dir);
		return true;
	} else if (fext.IsSameAs("acs", false)) {
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(dir);
		return false;
	} else if (fext.IsSameAs("png", false)) {
		if (content[WGFX] == NULL)
			content[WGFX] = new WadContent(WGFX);
		content[WGFX]->addLump(dir);
		return false;
	} else if (fext.IsSameAs("bmf", false) || fext.IsSameAs("fon2", false)) {
		if (content[WFONT] == NULL)
			content[WFONT] = new WadContent(WFONT);
		content[WFONT]->addLump(dir);
		return false;
	}

	wxString lname = pathName.GetName();
	lname.MakeUpper();
	if (lname.IsSameAs("DEHACKED")) {
		if (content[WDEHACKED] == NULL)
			content[WDEHACKED] = new WadContentX(WDEHACKED, tempPath+wxFILE_SEP_PATH+pathName.GetFullPath());
		content[WDEHACKED]->addLump(dir);
		return true;
	} else if (lname.IsSameAs("DECORATE")) {
		if (content[WDECORATE] == NULL)
			content[WDECORATE] = new WadContentX(WDECORATE, tempPath+wxFILE_SEP_PATH+pathName.GetFullPath());
		content[WDECORATE]->addLump(dir);
		return true;
	} else if (lname.IsSameAs("MAPINFO") || lname.IsSameAs("ZMAPINFO")) {
		if (content[WMAPINFO] == NULL)
			content[WMAPINFO] = new WadContentX(WMAPINFO, tempPath+wxFILE_SEP_PATH+pathName.GetFullPath());
		content[WMAPINFO]->addLump(dir);
		return true;
	} else if (lname.IsSameAs("ZSCRIPT")) {
		if (content[WZSCRIPT] == NULL)
			content[WZSCRIPT] = new WadContentX(WZSCRIPT, tempPath+wxFILE_SEP_PATH+pathName.GetFullPath());
		content[WZSCRIPT]->addLump(dir);
		return true;
	} else if (lname.IsSameAs("HIRESTEX") || lname.IsSameAs("TEXTURES")) {
		if (content[WTEXTURES] == NULL)
			content[WTEXTURES] = new WadContent(WTEXTURES);
		content[WTEXTURES]->bytes += dir->size;
		return false;
	}

	if (processLumpName(dir, lname))
		return false;
	if (processLumpName2(dir, lname))
		return false;
	//By now we know we won't extract the file, so we can read the first bytes
	char bytes[4];
	file.Read(bytes, 4);
	if (checkTexLists(dir, lname, bytes)) return false;
	if (checkBmp(dir, bytes)) return false;
	if (checkFon12(dir, bytes)) return false;
	if (checkMus(dir, bytes)) return false;
	if (checkMidi(dir, bytes)) return false;
	if (checkSfx(dir, bytes)) return false;
	if (checkWav(dir, bytes)) return false;
	if (checkPcs(dir, bytes)) return false;
	if (checkAcs(dir, bytes)) return false;

	if (content[WUNKNOWN] == NULL)
		content[WUNKNOWN] = new WadContentX(WUNKNOWN, "");
	content[WUNKNOWN]->addLump(dir);
	return false;
}

bool Pk3Stats::processDirFile(DirEntry* dir, const wxFileName& pathName)
{
	wxArrayString dirs = pathName.GetDirs();
	//"maps" - each wad has a single map, file name identifies map (NOT map label)
	if (dirs[0].CmpNoCase("textures")==0) {
		if (content[WTX] == NULL)
			content[WTX] = new WadContent(WTX);
		content[WTX]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("patches")==0) {
		if (content[WPATCH] == NULL)
			content[WPATCH] = new WadContent(WPATCH);
		content[WPATCH]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("flats")==0) {
		if (content[WFLAT] == NULL)
			content[WFLAT] = new WadContent(WFLAT);
		content[WFLAT]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("sprites")==0) {
		if (content[WSPRITE] == NULL)
			content[WSPRITE] = new WadContent(WSPRITE);
		content[WSPRITE]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("graphics")==0) {
		if (content[WGFX] == NULL)
			content[WGFX] = new WadContent(WGFX);
		content[WGFX]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("hires")==0) {
		if (content[WTX] == NULL)
			content[WTX] = new WadContent(WTX);
		content[WTX]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("sounds")==0) {
		wxString ext = pathName.GetExt();
		WadContentType t = WOTHERSND;
		if (ext.CmpNoCase("wav")==0)
			t = WWAV;
		if (content[t] == NULL)
			content[t] = new WadContent(t);
		content[t]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("music")==0) {
		if (content[WOTHERMUS] == NULL)
			content[WOTHERMUS] = new WadContent(WOTHERMUS);
		content[WOTHERMUS]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("acs")==0) {
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(dir);
		return true;
	}
	if (dirs[0].CmpNoCase("colormaps")==0) {
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->addLump(dir);
		return true;
	}
	// "/voices" for Strife dialog sounds
	// "/voxels" for voxel objects
	return false;
}

void Pk3Stats::processWad(DirEntry* dir, TaskProgress* progress)
{
	wxString fn(tempPath+wxFILE_SEP_PATH+dir->name);
	WadStats* ws = new WadStats(fn);
	ws->readFile(progress, false);
	if (progress->hasFailed()) {
		delete ws;
		return;
	}

	if (ws->hexenMap) hexenMap = true;
	if (ws->hexenLumps) hexenLumps=true;
	if (ws->iwad > iwad) iwad=ws->iwad;
	if (ws->engine > engine) engine=ws->engine;
	numberOfLumps += ws->numberOfLumps;
	priority += ws->priority;

	WadContentX* wcx = ws->getMapContent();
	if (wcx != NULL) { //Wad has map(s)
		WadContentX* mine = getMapContent();
		wxFileName fn = wxFileName(dir->name);
		if (fn.GetDirCount() > 0) {
			wxArrayString dirs = fn.GetDirs();
			if (dirs[0].CmpNoCase("maps")==0) {
				//Wad in maps folder: single map, file name identifies map
				string nam = fn.GetName().MakeUpper().ToStdString();
				if (wcx->lumps->at(0)->name.compare(nam) != 0)
					wcx->lumps->insert(wcx->lumps->begin(), new DirEntry(nam));
			}
		}
		if (mine==NULL) {
			content[WMAP] = wcx;
		} else {
			int cmp = wcx->lumps->at(0)->name.compare(mine->lumps->at(0)->name);
			if (cmp == 0) {
				//Identical with map marker from other wad
				//Replace the first with the second(?)
				if (mine->next != NULL)
					wcx->addSorted(mine->next);
				mine->next = NULL;
				delete mine;
				content[WMAP] = wcx;
			} else if (cmp < 0) {
				wcx->addSorted(mine);
				content[WMAP] = wcx;
			} else {
				mine->addSorted(wcx);
			}
		}
		ws->content[WMAP] = NULL;
	}
	for (int i=WSPRITE; i<WEND; i++) {
		if (ws->content[i] != NULL) {
			if (content[i] == NULL)
				content[i] = ws->content[i];
			else
				content[i]->mergeContent(ws->content[i]);
			ws->content[i] = NULL;
		}
	}
	delete ws;
}

wxString Pk3Stats::extractFile(wxZipInputStream& zip, wxZipEntry* entry, TaskProgress* progress)
{
	wxFileName tempName(tempPath+wxFILE_SEP_PATH+entry->GetName());
	if (!tempName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		progress->fatalError(wxString::Format("Can't extract file %s",entry->GetName()));
		return "";
	}
	wxFileOutputStream file(tempName.GetFullPath());
	if (!file) {
		progress->fatalError(wxString::Format("Can't extract file %s",tempName.GetFullPath()));
		return "";
	}
	zip.Read(file);
	file.Close();
	wxLogVerbose("Extracted file %s", entry->GetName());
	extracted->push_back(tempName.GetFullPath());
	return tempName.GetFullPath();
}

wxString Pk3Stats::extractFile(wxString file, TaskProgress* progress)
{
	wxFFileInputStream in(filePathName);
	wxZipInputStream zip(in);
	wxString intName = wxZipEntry::GetInternalName(file);
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry!=NULL && entry->GetInternalName()!=intName) {
		delete entry;
		entry = zip.GetNextEntry();
	}
	if (entry != NULL) {
		wxString result = extractFile(zip, entry, progress);
		delete entry;
		return result;
	} else {
		wxLogVerbose("Failed to find %s in archive", file);
		return "";
	}
}
