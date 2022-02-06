#include "WadStats.h"
#include "md5.h"
#include <wx/filename.h>
#include "..\LtbUtils.h"


//***************************************************************
//************************ WadContent(X) ************************
//***************************************************************

WadContent::WadContent(WadContentType t)
: type(t), count(0), bytes(0), markerStack(0)
{}

wxString WadContent::getString()
{
	wxString result(wadContentNames[type]);
	//if (count > 1) {
		result.Append("\t").Append(LtbUtils::intToString(count));
		result.Append("\t").Append(LtbUtils::intToString(bytes));
	//}
	return result;
}

void WadContent::addLump(DirEntry* lump)
{
	count++;
	bytes += lump->size;
}

void WadContent::mergeContent(WadContent* wc)
{
	count += wc->count;
	bytes += wc->bytes;
	delete wc;
}

WadContentX::WadContentX(WadContentType t, wxString str, WadContentX* wcx)
: WadContent(t), name(str), lumps(NULL), next(wcx)
{}

WadContentX::~WadContentX()
{
	if (lumps != NULL) {
		for (int i=0; i<lumps->size(); i++)
			delete lumps->at(i);
		delete lumps;
	}
	if (next != NULL)
		delete next;
}

wxString WadContentX::getString()
{
	//Currently gets total counts when other objects are added with mergeContent,
	//don't need to summarize whole list
	wxString result(wadContentNames[type]);
	//if (count > 1) {
		result.Append("\t").Append(LtbUtils::intToString(count));
		result.Append("\t").Append(LtbUtils::intToString(bytes));
	//}
	return result;
}

void WadContentX::addLump(DirEntry* lump)
{
	WadContent::addLump(lump);
	if (lumps == NULL)
		lumps = new vector<DirEntry*>();
	lumps->push_back(new DirEntry(*lump));
}

void WadContentX::mergeContent(WadContent* wc)
{
	count += wc->count;
	bytes += wc->bytes;
	WadContentX* wcx = dynamic_cast<WadContentX*>(wc);
	if (wcx != NULL) {
		if (wcx->name.Len() == 0) {
			//Does not repr. file - just add lumps (can't have next)
			if (wcx->lumps != NULL) {
				for (int i=0; i<wcx->lumps->size(); i++)
					addLump(wcx->lumps->at(i));
			}
			delete wcx;
		} else {
			//File(s) - add to list
			addToEnd(wcx);
		}
	}
}

void WadContentX::addToEnd(WadContentX* wcx)
{
	if (next==NULL)
		next = wcx;
	else
		next->addToEnd(wcx);
}

void WadContentX::addSorted(WadContentX* wcx)
{
	if (next == NULL) {
		next = wcx;
	} else {
		int cmp = wcx->lumps->at(0)->name.compare(next->lumps->at(0)->name);
		if (cmp == 0) {
			//Two identical map markers in same wad
			//Replace the first with the second(?)
			if (next->next != NULL)
				wcx->addSorted(next->next);
			next->next = NULL;
			delete next;
			next = wcx;
		} else if (cmp < 0) {
			wcx->addSorted(next);
			next = wcx;
		} else {
			next->addSorted(wcx);
		}
	}
}

bool WadContentX::containsLump(string lumpName)
{
	if (lumps == NULL)
		return false;
	DirEntry* lump;
	for (int i=0; i<lumps->size(); i++) {
		lump = lumps->at(i);
		wxString lname(lump->name);
		if (lname.CmpNoCase(lumpName)==0)
			return true;
	}
	return false;
}


//**********************************************************
//************************ WadStats ************************
//**********************************************************

WadStats::WadStats(wxString file)
: year(0), wadType(), hexenMap(false), hexenLumps(false), iwad(IWAD_NONE),
engine(DENG_ORIGINAL), priority(0), currentMap(NULL)
{
	filePathName = file;
	wxFileName path = wxFileName(file);
	fileName = string(path.GetFullName().c_str());
	for (int i=0; i<WEND; i++)
		content[i] = NULL;
}

WadStats::~WadStats()
{
	for (int i=0; i<WEND; i++) {
		if (content[i] != NULL)
			delete content[i];
	}
}

void WadStats::cleanup()
{
}

void WadStats::readFile(TaskProgress* progress, bool findMd5)
{
	//Can't call if already has contents!
	wxFile file(filePathName, wxFile::read);
	if (!file.IsOpened()) {
		progress->fatalError(wxString::Format("Couldn't open file %s",fileName));
		return;
	}
	fileSize = file.Length();
	wxLogVerbose("Processing file %s of size %i", fileName, fileSize);
	if (fileSize<12) {
		progress->fatalError(wxString::Format("No contents in file %s",fileName));
		return;
	}
	char* ch = new char(0);
	int32_t* lng = new int32_t(0);

	//Read wad header:
	for (int i=0; i<4; i++) {
		file.Read(ch, 1);
		wadType += *ch;
	}
	wxLogVerbose("Wad type: %s", wadType);
	file.Read(lng, 4);
	numberOfLumps = *lng;
	wxLogVerbose("Number of lumps: %i", numberOfLumps);
	file.Read(lng, 4);
	int32_t directoryOffset = *lng;
	wxLogVerbose("Reading lump list at offset %i", directoryOffset);
	progress->startCount(numberOfLumps+2);

	//Read wad directory:
	if (file.Seek(directoryOffset, wxFromStart) == wxInvalidOffset)
		progress->fatalError(wxString::Format("Couldn't find lump list in %s",fileName));
	else if (fileSize < directoryOffset+(numberOfLumps*16))
		progress->fatalError("Lump list not of specified size.");
	if (progress->hasFailed()) {
		delete ch;
		delete lng;
		return;
	}

	vector<DirEntry*> directory(numberOfLumps);
	wxFileInputStream* fileStream = new wxFileInputStream(file);
	wxBufferedInputStream* buf = new wxBufferedInputStream(*fileStream, 1024); //16*numberOfLumps
	bool ok = true;
	for (int i=0; i<numberOfLumps; i++) {
		directory[i] = new DirEntry();
		buf->Read(lng, 4);
		directory[i]->offset = *lng;
		buf->Read(lng, 4);
		directory[i]->size = *lng;
		ok = true;
		for (int j=0; j<8; j++) {
			buf->Read(ch, 1);
			if (ok && (*ch != 0)) directory[i]->name += *ch;
			else ok = false;
		}
		if ((i<(numberOfLumps-1)) && (buf->Eof())) {
			numberOfLumps = i+1;
			progress->fatalError("Premature end of file.");
			break;
		}
	}
	delete ch;
	delete lng;
	delete buf;

	if (progress->hasFailed()) {
		for (int i=0; i<numberOfLumps; i++)
			delete directory[i];
	} else {
		unsigned int dirIndex = 0;
		while (dirIndex < directory.size()) {
			processLump(directory[dirIndex], fileStream);
			delete directory[dirIndex];
			dirIndex++;
			progress->incrCount();
		}
		if (content[WDECORATE]!=NULL && content[WUNKNOWN]!=NULL)
			findLumpIncludes(WDECORATE, progress);
		if (content[WMAPINFO]!=NULL && content[WUNKNOWN]!=NULL)
			findLumpIncludes(WMAPINFO, progress);
		if (content[WZSCRIPT]!=NULL && content[WUNKNOWN]!=NULL)
			findLumpIncludes(WZSCRIPT, progress);
		progress->incrCount();
		if (content[WMAP] != NULL)
			validateMaps();

		checkIwadEngine();
		if (findMd5) makeMd5(file);
		progress->completeCount();
		if (content[WERROR] != NULL) {
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
	delete fileStream;
	file.Close();
}

MapinfoParser* WadStats::getMapinfo(TaskProgress* progress)
{
	MapinfoParser* mapinfo = NULL;
	WadContentX* wcx = getContent(WMAPINFO);
	//NOTE: Its possible to have both MAPINFO and ZMAPINFO
	while (wcx != NULL) {
		if (wcx->lumps->size() > 0) {
			DirEntry* de = wcx->lumps->at(0);
			if (de->offset > -1) {
				if (mapinfo == NULL)
					mapinfo = new MapinfoParser();
				try {
					mapinfo->parseFile(wcx->name, de->offset, de->size);
				} catch (GuiError e) {
					progress->warnError("Failed parsing MAPINFO");
					delete mapinfo;
					mapinfo = NULL;
				}
			}
		}
		wcx = wcx->next;
	}
	//Check if any unknown lumps are defined as music by MAPINFO
	if (mapinfo!=NULL) {
		wcx = getContent(WUNKNOWN);
		if ((wcx!=NULL) && (wcx->lumps->size()>0)) {
			vector<DirEntry*>* dirEntries = wcx->lumps;
			for (int i=0; i<dirEntries->size(); i++) {
				DirEntry* de = dirEntries->at(i);
				if (mapinfo->isMusic(de->name)) {
					addLump(de,WOTHERMUS);
					de->size = -1; //Mark to delete
				}
			}
			deleteUnknown();
		}
	}
	return mapinfo;
}

void WadStats::makeMd5(wxFile& file)
{
	wxLogVerbose("Generating MD5 checksum...");
	file.Seek(0, wxFromStart);
	MD5 md;
	char buf[1024];
	uint32_t bytesLeft = fileSize;
	while (bytesLeft > 0) {
		//wxLogVerbose("Bytes left %i", bytesLeft);
		if (bytesLeft > 1024) {
			file.Read(buf, 1024);
			md.update(buf, 1024);
			bytesLeft -= 1024;
		} else {
			file.Read(buf, bytesLeft);
			md.update(buf, bytesLeft);
			bytesLeft = 0;
		}
	}
	md.finalize();
	for (int i=0; i<16; i++)
		md5Digest[i] = md.bytedigest(i);
	wxLogVerbose("MD5 ready: %s", md.hexdigest());
}

void WadStats::addLump(DirEntry* de, WadContentType type)
{
	if (content[type] == NULL)
		content[type] = new WadContent(type);
	content[type]->addLump(de);
}

void WadStats::lumpError(wxString error, DirEntry* lump)
{
	content[WERROR] = new WadContentX(WERROR, error, dynamic_cast<WadContentX*>(content[WERROR]));
	content[WERROR]->addLump(lump);
}

void WadStats::processMarkerLump(DirEntry* lump, const wxString& lname)
{
	if (lname.Matches("S*_START")) { //Sprites
		wxLogVerbose("Start marker for SPRITES");
		if (content[WSPRITE] == NULL)
			content[WSPRITE] = new WadContent(WSPRITE);
		content[WSPRITE]->markerStack++;
	} else if (lname.Matches("S*_END")) {
		wxLogVerbose("End marker for SPRITES");
		if (content[WSPRITE]==NULL || content[WSPRITE]->markerStack==0) {
			lumpError("S_END marker without start", lump);
		} else {
			content[WSPRITE]->markerStack--;
		}
	} else if (lname.Matches("P*_START")) {
		wxLogVerbose("Start marker for PATCHES");
		if (content[WPATCH] == NULL)
			content[WPATCH] = new WadContent(WPATCH);
		content[WPATCH]->markerStack++;
	} else if (lname.Matches("P*_END")) {
		wxLogVerbose("End marker for PATCHES");
		if (content[WPATCH]==NULL || content[WPATCH]->markerStack==0) {
			lumpError("P_END marker without start", lump);
		} else {
			content[WPATCH]->markerStack--;
		}
	} else if (lname.Matches("F*_START")) {
		wxLogVerbose("Start marker for FLATS");
		if (content[WFLAT] == NULL)
			content[WFLAT] = new WadContent(WFLAT);
		content[WFLAT]->markerStack++;
	} else if (lname.Matches("F*_END")) {
		wxLogVerbose("End marker for FLATS");
		if (content[WFLAT]==NULL || content[WFLAT]->markerStack==0) {
			lumpError("F_END marker without start", lump);
		} else {
			content[WFLAT]->markerStack--;
		}
	} else if (lname.IsSameAs("C_START")) {
		wxLogVerbose("Start marker for COLORMAPS");
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->markerStack++;
		if (engine<DENG_BOOM) engine=DENG_BOOM;
	} else if (lname.IsSameAs("C_END")) {
		wxLogVerbose("End marker for COLORMAPS");
		if (content[WCOLORMAP]==NULL || content[WCOLORMAP]->markerStack==0) {
			lumpError("C_END marker without start", lump);
		} else {
			content[WCOLORMAP]->markerStack--;
		}
	} else if (lname.Matches("FONT*_S")) {
		wxLogVerbose("Start marker for FONT");
		if (content[WFONT] == NULL)
			content[WFONT] = new WadContent(WFONT);
		content[WFONT]->markerStack++;
	} else if (lname.Matches("FONT*_E")) {
		wxLogVerbose("End marker for FONT");
		if (content[WFONT]==NULL || content[WFONT]->markerStack==0) {
			lumpError("FONT end marker without start", lump);
		} else {
			content[WFONT]->markerStack--;
		}
	} else if (lname.IsSameAs("TX_START")) {
		wxLogVerbose("Start marker for TEXTURES");
		if (content[WTX] == NULL)
			content[WTX] = new WadContent(WTX);
		content[WTX]->markerStack++;
		if (engine<DENG_ZDOOM) engine=DENG_ZDOOM;
	} else if (lname.IsSameAs("TX_END")) {
		wxLogVerbose("End marker for TEXTURES");
		if (content[WTX]==NULL || content[WTX]->markerStack==0) {
			lumpError("TX_END marker without start", lump);
		} else {
			content[WTX]->markerStack--;
		}
	} else if (lname.Matches("*_START") || lname.Matches("*_END")) {
		wxLogVerbose("Unknown marker %s", lname);
	} else if (lname.StartsWith("_")) {
		//Comment marker
	} else if (lname.Length()==0) {
		//Empty marker - junk
	} else { //Should be map
		mapMarker(lump);
	}
}

void WadStats::mapMarker(DirEntry* lump)
{
	string name = lump->name;
	if ((lump->name.length()>3) && (lump->name.substr(0,3).compare("GL_") == 0)) {
		//Map marker for GL nodes
		name = lump->name.substr(3,string::npos);
		lump->name = name;
		if ((currentMap!=NULL) && (currentMap->lumps->at(0)->name.compare(name)==0))
			return; //Same as current
	}
	wxLogVerbose("Start marker for map %s", name);
	currentMap = new WadContentX(WMAP, filePathName);
	currentMap->addLump(lump);
	priority++;
	//Iwad type
	if (iwad == IWAD_NONE) {
		if (name.substr(0,1).compare("E") == 0)
			iwad = IWAD_DOOM;
		else if (name.substr(0,3).compare("MAP") == 0)
			iwad = IWAD_DOOM2;
	}
	if (content[WMAP] == NULL) {
		content[WMAP] = currentMap;
	} else {
		//Add sorted on name
		WadContentX* wcx = dynamic_cast<WadContentX*>(content[WMAP]);
		int cmp = name.compare(wcx->lumps->at(0)->name);
		if (cmp < 0) {
			currentMap->next = wcx;
			content[WMAP] = currentMap;
		} else if (cmp == 0) {
			//Two identical map markers in same wad
			//Replace the first with the second(?)
			lumpError("Duplicate map marker", lump);
			currentMap->next = wcx->next;
			wcx->next = NULL;
			delete wcx;
			content[WMAP] = currentMap;
		} else {
			wcx->addSorted(currentMap);
		}
	}
}

bool WadStats::processLumpName(DirEntry* lump, const wxString& lname)
{
	if (lname.IsSameAs("PLAYPAL")) {
		if (content[WPLAYPAL] == NULL)
			content[WPLAYPAL] = new WadContent(WPLAYPAL);
		content[WPLAYPAL]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("E2PAL")) {
		if (content[WPLAYPAL] == NULL)
			content[WPLAYPAL] = new WadContent(WPLAYPAL);
		content[WPLAYPAL]->addLump(lump);
		iwad = IWAD_HERETIC;
		return true;
	}
	if (lname.IsSameAs("COLORMAP")) {
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("ENDOOM") && lump->size==4000) {
		if (content[WENDOOM] == NULL)
			content[WENDOOM] = new WadContent(WENDOOM);
		content[WENDOOM]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("ENDTEXT") && lump->size==4000) {
		if (content[WENDOOM] == NULL)
			content[WENDOOM] = new WadContent(WENDOOM);
		content[WENDOOM]->addLump(lump);
		iwad = IWAD_HERETIC;
		return true;
	}
	if (lname.IsSameAs("ENDBOOM") && lump->size==4000) {
		if (content[WENDOOM] == NULL)
			content[WENDOOM] = new WadContent(WENDOOM);
		content[WENDOOM]->addLump(lump);
		if (engine<DENG_BOOM) engine=DENG_BOOM;
		return true;
	}
	if ((lname.IsSameAs("DMXGUS")) || (lname.IsSameAs("GENMIDI")) ||
			(lname.IsSameAs("DMXGUSC"))) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("_DEUTEX_")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("TEXTCOLO")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.Matches("DEMO?")) {
		if (content[WDEMOS] == NULL)
			content[WDEMOS] = new WadContent(WDEMOS);
		content[WDEMOS]->addLump(lump);
		return true;
    }
    if ((lname.IsSameAs("ANIMATED")) || (lname.IsSameAs("SWITCHES"))) {
		if (content[WANIM] == NULL)
			content[WANIM] = new WadContent(WANIM);
		content[WANIM]->addLump(lump);
		if (engine<DENG_BOOM) engine=DENG_BOOM;
		return true;
	}
	if (lname.IsSameAs("ANIMDEFS")) {
		if (content[WANIM] == NULL)
			content[WANIM] = new WadContent(WANIM);
		content[WANIM]->addLump(lump);
		hexenLumps = true;
		return true;
	}
	if ((lname.IsSameAs("SNDSEQ")) || (lname.IsSameAs("SNDINFO"))) {
		if (content[WSNDDEF] == NULL)
			content[WSNDDEF] = new WadContent(WSNDDEF);
		content[WSNDDEF]->addLump(lump);
		hexenLumps = true;
		return true;
	}
	if (lname.IsSameAs("SNDCURVE")) {
		if (content[WSNDDEF] == NULL)
			content[WSNDDEF] = new WadContent(WSNDDEF);
		content[WSNDDEF]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("DECALDEF")) {
		if (content[WZDEF] == NULL)
			content[WZDEF] = new WadContent(WZDEF);
		content[WZDEF]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("DEHACKED")) {
		if (content[WDEHACKED] == NULL)
			content[WDEHACKED] = new WadContentX(WDEHACKED, filePathName);
		content[WDEHACKED]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("DECORATE")) {
		if (content[WDECORATE] == NULL)
			content[WDECORATE] = new WadContentX(WDECORATE, filePathName);
		content[WDECORATE]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("MAPINFO")) {
		if (content[WMAPINFO] == NULL)
			content[WMAPINFO] = new WadContentX(WMAPINFO, filePathName);
		content[WMAPINFO]->addLump(lump);
		hexenLumps = true;
		return true;
	}
	if (lname.IsSameAs("ZMAPINFO")) {
		if (content[WMAPINFO] == NULL)
			content[WMAPINFO] = new WadContentX(WMAPINFO, filePathName);
		content[WMAPINFO]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("KEYCONF")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("TERRAIN")) {
		if (content[WZDEF] == NULL)
			content[WZDEF] = new WadContent(WZDEF);
		content[WZDEF]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if ((lname.IsSameAs("GAMEINFO")) || (lname.IsSameAs("SBARINFO"))
		|| (lname.IsSameAs("MENUDEF")))
	{
		if (content[WZDEF] == NULL)
			content[WZDEF] = new WadContent(WZDEF);
		content[WZDEF]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("LOCKDEFS")) {
		if (content[WZDEF] == NULL)
			content[WZDEF] = new WadContent(WZDEF);
		content[WZDEF]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("FONTDEFS")) {
		if (content[WZDEF] == NULL)
			content[WZDEF] = new WadContent(WZDEF);
		content[WZDEF]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("GLDEFS") || lname.IsSameAs("DOOMDEFS")) {
		if (content[WGLDEF] == NULL)
			content[WGLDEF] = new WadContent(WGLDEF);
		content[WGLDEF]->addLump(lump);
		if (engine<DENG_GZDOOM) engine = DENG_GZDOOM;
		return true;
	}
	if (lname.IsSameAs("LOADACS")) {
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("SCRIPT??")) {
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("ZSCRIPT")) {
		if (content[WZSCRIPT] == NULL)
			content[WZSCRIPT] = new WadContentX(WZSCRIPT, filePathName);
		content[WZSCRIPT]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("CVARINFO")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	return false;
}

bool WadStats::processLumpName2(DirEntry* lump, const wxString& lname)
{
	if ((lname.IsSameAs("FOGMAP")) || lname.StartsWith("TRANTBL")) {
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->addLump(lump);
		hexenLumps = true;
		return true;
	}
	if (lname.IsSameAs("TINTTAB")) {
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->addLump(lump);
		return true;
	}
	if (lname.Matches("CLUS?MSG") || lname.Matches("WIN?MSG")) {
		if (content[WTEXT] == NULL)
			content[WTEXT] = new WadContent(WTEXT);
		content[WTEXT]->addLump(lump);
		iwad = IWAD_HEXEN;
		return true;
	}
	if (lname.IsSameAs("HTICDEFS")) {
		if (content[WGLDEF] == NULL)
			content[WGLDEF] = new WadContent(WGLDEF);
		content[WGLDEF]->addLump(lump);
		if (engine<DENG_GZDOOM) engine = DENG_GZDOOM;
		iwad = IWAD_HERETIC;
		return true;
	}
	if (lname.IsSameAs("HEXNDEFS")) {
		if (content[WGLDEF] == NULL)
			content[WGLDEF] = new WadContent(WGLDEF);
		content[WGLDEF]->addLump(lump);
		if (engine<DENG_GZDOOM) engine = DENG_GZDOOM;
		iwad = IWAD_HEXEN;
		return true;
	}
	if (lname.IsSameAs("STRFDEFS")) {
		if (content[WGLDEF] == NULL)
			content[WGLDEF] = new WadContent(WGLDEF);
		content[WGLDEF]->addLump(lump);
		if (engine<DENG_GZDOOM) engine = DENG_GZDOOM;
		iwad = IWAD_STRIFE;
		return true;
	}
	if (lname.IsSameAs("AUTOPAGE")) {
		//Heretic/Hexen automap background
		if (content[WBITMAP] == NULL)
			content[WBITMAP] = new WadContent(WBITMAP);
		content[WBITMAP]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("XLATAB")) {
		if (content[WCOLORMAP] == NULL)
			content[WCOLORMAP] = new WadContent(WCOLORMAP);
		content[WCOLORMAP]->addLump(lump);
		iwad = IWAD_STRIFE;
		return true;
	}
	if (lname.IsSameAs("ENDSTRF")) {
		if (content[WENDOOM] == NULL)
			content[WENDOOM] = new WadContent(WENDOOM);
		content[WENDOOM]->addLump(lump);
		iwad = IWAD_STRIFE;
		return true;
	}
	if (lname.IsSameAs("T1TEXT") || lname.IsSameAs("C1TEXT")
			|| lname.IsSameAs("COPYRITE")) {
		if (content[WTEXT] == NULL)
			content[WTEXT] = new WadContent(WTEXT);
		content[WTEXT]->addLump(lump);
		iwad = IWAD_STRIFE;
		return true;
	}
	if (lname.Matches("LOG*")) {
		if (content[WTEXT] == NULL)
			content[WTEXT] = new WadContent(WTEXT);
		content[WTEXT]->addLump(lump);
		iwad = IWAD_STRIFE;
		return true;
	}
	if (lname.Matches("LANGUAGE")) {
		//Pk3 can have multiple such files with different extensions
		if (content[WTEXT] == NULL)
			content[WTEXT] = new WadContent(WTEXT);
		content[WTEXT]->addLump(lump);
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return true;
	}
	if (lname.IsSameAs("LOADING") && lump->size==4000) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("EXTENDED")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("SERIAL")) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("GFRAG")) {
		//In Heretic iwad, sound fx?
		if (content[WSFX] == NULL)
			content[WSFX] = new WadContent(WSFX);
		content[WSFX]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("FSGLOBAL")) { //FraggleScript global
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("NOTCH") || (lump->size==184)) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("NETNOTCH") || (lump->size==32)) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("STARTUP") && (lump->size==153648)) {
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("REVERBS")) {
		if (content[WSNDDEF] == NULL)
			content[WSNDDEF] = new WadContent(WSNDDEF);
		content[WSNDDEF]->addLump(lump);
		return true;
	}
	if (lname.IsSameAs("SQU-SWE1")) { //Marker for Adventures of Square
		if (content[WOTHER] == NULL)
			content[WOTHER] = new WadContent(WOTHER);
		content[WOTHER]->addLump(lump);
		iwad = IWAD_SQUARE;
		return true;
	}
	return false;
}

void WadStats::processLump(DirEntry* lump, wxInputStream* file)
{
	wxLogVerbose("Lump %s of size %i @ %i", lump->name, lump->size, lump->offset);
	wxString lname(lump->name);
	lname.MakeUpper();
	if (currentMap != NULL) {
		if (currentMap->containsLump(lump->name)) {
			lumpError("Duplicate map lump", lump);
			return;
		}
		if (lname.IsSameAs("LINEDEFS") || lname.IsSameAs("SIDEDEFS") || lname.IsSameAs("VERTEXES") || lname.IsSameAs("SECTORS"))
		{
			if (lump->size < 4)
				lumpError("Empty map lump", lump);
			else
				currentMap->addLump(lump);
			return;
		}
		if (lname.IsSameAs("THINGS") || lname.IsSameAs("SEGS") || lname.IsSameAs("SSECTORS")
			|| lname.IsSameAs("NODES") || lname.IsSameAs("REJECT") || lname.IsSameAs("BLOCKMAP"))
		{
			currentMap->addLump(lump);
			return;
		}
		//TODO: GL_ map marker node (GL_MAP?? or GL_E?M?) can have content
		if (lname.IsSameAs("GL_VERT") || lname.IsSameAs("GL_SEGS") || lname.IsSameAs("GL_SSECT")
			|| lname.IsSameAs("GL_NODES") || lname.IsSameAs("GL_PVS"))
		{
			currentMap->addLump(lump);
			return;
		}
		if (lname.IsSameAs("TEXTMAP") || lname.IsSameAs("DIALOGUE") || lname.IsSameAs("ZNODES")) {
			//UDMF
			currentMap->addLump(lump);
			if (engine<DENG_ZDOOM) engine=DENG_ZDOOM;
			return;
		}
		if ((currentMap->lumps!=NULL) && (currentMap->lumps->size()>0)) {
			if (lname.IsSameAs("BEHAVIOR") || lname.IsSameAs("SCRIPTS")) {
				currentMap->addLump(lump);
				hexenMap = true;
				if (lname.IsSameAs("BEHAVIOR") && lump->size>1) {
					if (content[WBEHAVIOR] == NULL)
						content[WBEHAVIOR] = new WadContent(WBEHAVIOR);
					content[WBEHAVIOR]->addLump(lump);
				}
				return;
			}
		}
		if (lname.IsSameAs("ENDMAP") && (lump->size==0)) {
			currentMap = NULL;
			return;
		}
	} //end map lumps
	if (lname.IsSameAs("LINEDEFS") || lname.IsSameAs("SIDEDEFS") || lname.IsSameAs("VERTEXES") || lname.IsSameAs("SECTORS")
		|| lname.IsSameAs("THINGS") || lname.IsSameAs("SEGS") || lname.IsSameAs("SSECTORS") || lname.IsSameAs("NODES")
		|| lname.IsSameAs("REJECT") || lname.IsSameAs("BLOCKMAP") || lname.IsSameAs("BEHAVIOR"))
	{
		lumpError("Map lump without map marker", lump);
		return;
	}

	if (lump->size == 0) {
		processMarkerLump(lump, lname);
		return;
	}
	//Read first 4 bytes
	char bytes[4];
	file->SeekI(lump->offset, wxFromStart);
	file->Read(bytes, 4);
	bool checkedPicture = false;

	if (checkTexLists(lump, lname, bytes)) return;

	if ((content[WSPRITE]!=NULL) && (content[WSPRITE]->markerStack>0)) {
		if (checkPicture(lump, file, WSPRITE)) return;
		checkedPicture = true;
		if (checkPng(lump, bytes, WSPRITE)) return;
	}
	if ((content[WPATCH]!=NULL) && (content[WPATCH]->markerStack>0)) {
		if (!checkedPicture && checkPicture(lump, file, WPATCH)) return;
		checkedPicture = true;
		if (checkPng(lump, bytes, WPATCH)) return;
	}
	if ((content[WFLAT]!=NULL) && (content[WFLAT]->markerStack>0)) {
		if (lump->size == 4096) {
			content[WFLAT]->addLump(lump);
			return;
		}
		//Other sizes
		if (lump->size == 8192) {
			//Hexen iwad
			content[WFLAT]->addLump(lump);
			//iwad = IWAD_HEXEN;
			return;
		}
		if (lump->size==4160 || lump->size==4) {
			//Heretic iwad
			content[WFLAT]->addLump(lump);
			return;
		}
		if (checkPng(lump, bytes, WFLAT)) return;
	}
	if ((content[WFONT]!=NULL) && (content[WFONT]->markerStack>0)) {
		if (!checkedPicture && checkPicture(lump, file, WFONT)) return;
		checkedPicture = true;
	}

	if (processLumpName(lump, lname)) return;

    if (lname.StartsWith("DS")) {
		if (checkSfx(lump, bytes)) return;
		if (checkWav(lump, bytes)) return;
    }
    if (lname.StartsWith("DP")) {
		if (checkPcs(lump, bytes)) return;
    }
	if (lname.StartsWith("D_")) {
		if (checkMus(lump, bytes)) return;
		if (checkMidi(lump, bytes)) return;
	}
	if ((content[WCOLORMAP]!=NULL) && (content[WCOLORMAP]->markerStack>0)) {
		//Assume this lump is a Boom colormap
		content[WCOLORMAP]->addLump(lump);
		return;
	}
	if ((content[WTX]!=NULL) && (content[WTX]->markerStack>0)) {
		//Assume this lump is a ZDoom texture
		content[WTX]->addLump(lump);
		return;
	}
	if (lname.IsSameAs("HIRESTEX") || lname.IsSameAs("TEXTURES")) {
		if (content[WTEXTURES] == NULL)
			content[WTEXTURES] = new WadContent(WTEXTURES);
		content[WTEXTURES]->bytes += lump->size;
		if (engine<DENG_ZDOOM) engine = DENG_ZDOOM;
		return;
	}
	if (lname.Matches("MAP??") || lname.Matches("E?M?") || lname.Matches("GL_MAP??") || lname.Matches("GL_E?M?")) {
		//Map marker, even though the lump size is not zero
		mapMarker(lump);
		return;
	}
	if (checkPng(lump, bytes, WGFX)) return;
	if (checkMus(lump, bytes)) return;
	if (checkSfx(lump, bytes)) return;
	if (checkWav(lump, bytes)) return;
	if (checkFlac(lump, bytes)) return;
	if (checkPcs(lump, bytes)) return;
	if (checkMidi(lump, bytes)) return;
	if (checkMp3Ogg(lump, bytes)) return;
	if (checkFon12(lump, bytes)) return;
	if (checkAcs(lump, bytes)) return;
	if (!checkedPicture) {
		if (checkPicture(lump, file, WGFX)) return;
		checkedPicture = true;
	}
	if (checkJpg(lump, bytes)) return;
	if (checkBmp(lump, bytes)) return;

	if (processLumpName2(lump, lname)) return;

	if (lump->size == 4096) {
		if (content[WFLAT] == NULL)
			content[WFLAT] = new WadContent(WFLAT);
		content[WFLAT]->addLump(lump);
		return;
	}
	if (lump->size == 64000) {
		//320×200 raw image
		//Heretic/Hexen: TITLE, HELP1, HELP2 and CREDIT
		//Heretic: FINAL1, FINAL2 and E2END
		//Hexen: INTERPIC, FINALE1, FINALE2 and FINALE3
		//Strife: STARTUP0
		if (content[WBITMAP] == NULL)
			content[WBITMAP] = new WadContent(WBITMAP);
		content[WBITMAP]->addLump(lump);
		if (lname.IsSameAs("FINAL1"))
			iwad = IWAD_HERETIC;
		else if (lname.IsSameAs("FINALE1"))
			iwad = IWAD_HEXEN;
		return;
	}
	wxLogVerbose("Unknown lump %s of size %i @ %i", lump->name, lump->size, lump->offset);
	if (content[WUNKNOWN] == NULL)
		content[WUNKNOWN] = new WadContentX(WUNKNOWN, "");
	content[WUNKNOWN]->addLump(lump);
}

bool WadStats::checkPicture(DirEntry* lump, wxInputStream* file, WadContentType t)
{
	int16_t w, h, x, y;
	file->SeekI(lump->offset, wxFromStart);
	file->Read(&w, 2);
	file->Read(&h, 2);
	file->Read(&x, 2);
	file->Read(&y, 2);
	uint32_t columnStart = (w*4) + 8;
	if (lump->size <= columnStart)
		return false;
    uint32_t offset=0;
    for (int i=0; i<w; i++) {
		//Each value is an offset for a column of the image
		file->Read(&offset, 4);
		if ((offset>lump->size) || (offset<columnStart))
			return false;
    }
	//wxLogVerbose("Image header w %i h %i x %i y %i", w, h, x, y);
	if (content[t] == NULL)
		content[t] = new WadContent(t);
	content[t]->addLump(lump);
	return true;
}

bool WadStats::checkTexLists(DirEntry* lump, const wxString& lname, char* bytes)
{
	if (lname.IsSameAs("PNAMES")) {
		if (content[WPNAMES] == NULL) {
			content[WPNAMES] = new WadContent(WPNAMES);
			content[WPNAMES]->bytes = lump->size;
			int32_t num(0);
			std::memcpy(&num, bytes, 4);
			content[WPNAMES]->count = num;
		} else {
			lumpError("Multiple PNAMES lumps", lump);
		}
		return true;
	}
	if (lname.IsSameAs("TEXTURE1") || lname.IsSameAs("TEXTURE2")) {
		if (content[WTEXTURES] == NULL)
			content[WTEXTURES] = new WadContent(WTEXTURES);
		content[WTEXTURES]->bytes += lump->size;
		int32_t num(0);
		std::memcpy(&num, bytes, 4);
		content[WTEXTURES]->count += num;
		return true;
	}
	return false;
}

bool WadStats::checkPng(DirEntry* lump, char* bytes, WadContentType t)
{
	if (bytes[0]==0x89 && bytes[1]==0x50 && bytes[2]==0x4e && bytes[3]==0x47) {
		if (content[t] == NULL)
			content[t] = new WadContent(t);
		content[t]->addLump(lump);
		if (engine<DENG_ZDOOM)
			engine=DENG_ZDOOM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkJpg(DirEntry* lump, char* bytes)
{
	if (bytes[0]==0xff && bytes[1]==0xd8 && bytes[2]==0xff) {
		if (content[WGFX] == NULL)
			content[WGFX] = new WadContent(WGFX);
		content[WGFX]->addLump(lump);
		if (engine<DENG_ZDOOM)
			engine=DENG_ZDOOM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkBmp(DirEntry* lump, char* bytes)
{
	if (bytes[0]=='B' && bytes[1]=='M' && bytes[2]=='6') {
		if (content[WGFX] == NULL)
			content[WGFX] = new WadContent(WGFX);
		content[WGFX]->addLump(lump);
		if (engine<DENG_ZDOOM)
			engine=DENG_ZDOOM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkFon12(DirEntry* lump, char* bytes)
{
	//Starts with "FON1" or "FON2"
	if (bytes[0] != 0x46) return false;
	if (bytes[1] != 0x4f) return false;
	if (bytes[2] != 0x4e) return false;
	if ((bytes[3]!=0x31) && (bytes[3]!=0x32)) return false;
	if (content[WFONT] == NULL)
		content[WFONT] = new WadContent(WFONT);
	content[WFONT]->addLump(lump);
	return true;
}

bool WadStats::checkMus(DirEntry* lump, char* bytes)
{
	//Starts with "MUS" + 0x1A
	if (bytes[0]==0x4d && bytes[1]==0x55 && bytes[2]==0x53 && bytes[3]==0x1a) {
		if (content[WMUS] == NULL)
			content[WMUS] = new WadContent(WMUS);
		content[WMUS]->addLump(lump);
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkMidi(DirEntry* lump, char* bytes)
{
	//Starts with "MThd"
	if (bytes[0]==0x4d && bytes[1]==0x54 && bytes[2]==0x68 && bytes[3]==0x64) {
		if (content[WMIDI] == NULL)
			content[WMIDI] = new WadContent(WMIDI);
		content[WMIDI]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkMp3Ogg(DirEntry* lump, char* bytes)
{
	//Starts with "ID3"
	if (bytes[0]=='I' && bytes[1]=='D' && bytes[2]=='3') {
		if (content[WOTHERMUS] == NULL)
			content[WOTHERMUS] = new WadContent(WOTHERMUS);
		content[WOTHERMUS]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else if (bytes[0]==0xff && (bytes[1]==0xfa || bytes[1]==0xfb || bytes[1]==0xf3)) { //MP3
		if (content[WOTHERMUS] == NULL)
			content[WOTHERMUS] = new WadContent(WOTHERMUS);
		content[WOTHERMUS]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else if (bytes[0]=='O' && bytes[1]=='g' && bytes[2]=='g' && bytes[3]=='S') {
		if (content[WOTHERMUS] == NULL)
			content[WOTHERMUS] = new WadContent(WOTHERMUS);
		content[WOTHERMUS]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkSfx(DirEntry* lump, char* bytes)
{
	uint16_t val(0);
	std::memcpy(&val, bytes, 2);
	if (val != 3) return false;
	std::memcpy(&val, bytes+2, 2);
	if (val < 5500) return false;
	if (content[WSFX] == NULL)
		content[WSFX] = new WadContent(WSFX);
	content[WSFX]->addLump(lump);
	return true;
}

bool WadStats::checkWav(DirEntry* lump, char* bytes)
{
	//Starts with "RIFF"
	if (bytes[0]==0x52 && bytes[1]==0x49 && bytes[2]==0x46 && bytes[3]==0x46) {
		if (content[WWAV] == NULL)
			content[WWAV] = new WadContent(WWAV);
		content[WWAV]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkFlac(DirEntry* lump, char* bytes)
{
	//Starts with "fLaC"
	if (bytes[0]==0x66 && bytes[1]==0x4c && bytes[2]==0x61 && bytes[3]==0x43) {
		if (content[WOTHERSND] == NULL)
			content[WOTHERSND] = new WadContent(WOTHERSND);
		content[WOTHERSND]->addLump(lump);
		if (engine<DENG_LIMITREM)
			engine=DENG_LIMITREM;
		return true;
	} else {
		return false;
	}
}

bool WadStats::checkPcs(DirEntry* lump, char* bytes)
{
	int16_t val(0);
	std::memcpy(&val, bytes, 2);
	if (val != 0) return false;
	std::memcpy(&val, bytes+2, 2);
	if (lump->size != (val+4)) return false;
	if (content[WSPEAKER] == NULL)
		content[WSPEAKER] = new WadContent(WSPEAKER);
	content[WSPEAKER]->addLump(lump);
	return true;
}

bool WadStats::checkAcs(DirEntry* lump, char* bytes)
{
	//Starts with "ACS"
	if (bytes[0]=='A' && bytes[1]=='C' && bytes[2]=='S') {
		if (content[WACS] == NULL)
			content[WACS] = new WadContent(WACS);
		content[WACS]->addLump(lump);
		//ZDoom?
		return true;
	} else {
		return false;
	}
}

void WadStats::findLumpIncludes(WadContentType wct, TaskProgress* progress)
{
	try {
		WadContentX* oldDec = dynamic_cast<WadContentX*>(content[wct]);
		DirEntry* oldDir = oldDec->lumps->at(0);
		IncludeParser* ip = new IncludeParser(wct!=WMAPINFO);
		wxLogVerbose("Parse %s for includes", oldDec->name);
		ip->parseFile(oldDec->name, oldDir->offset, oldDir->size); //throws GuiError
		if ((wct==WDECORATE) && !ip->hasDecorate()) {
			//Don't parse original DECORATE again, since it is just includes
			oldDir->offset = -1;
		}
		vector<wxString>* inclNames = ip->GetIncludes();
		wxLogVerbose("Found %i includes", inclNames->size());
		WadContentX* decHead = findUnknownLumps(inclNames, wct, progress);
		if (decHead != NULL) {
			decHead->mergeContent(oldDec);
			content[wct] = decHead;
		}
		delete inclNames;
		delete ip;
	} catch (GuiError e) {
		//Failed to open the file - just log it
		progress->warnError(e.text1);
	}
}

WadContentX* WadStats::findUnknownLumps(vector<wxString>* inclNames, WadContentType wct, TaskProgress* progress)
{
	WadContentX* unkn = dynamic_cast<WadContentX*>(content[WUNKNOWN]);
	WadContentX* result = NULL;
	WadContentX* newDec;
	DirEntry* dir;
	for (int i=0; i<inclNames->size(); i++) {
		wxString incl = inclNames->at(i);
		dir = NULL;
		for (int i=0; i<unkn->lumps->size(); i++) {
			dir = unkn->lumps->at(i);
			if (incl.IsSameAs(dir->name, false))
				break;
		}
		if (dir != NULL) { //Found in UNKNOWN entries
			wxLogVerbose("Lump %s recognized as known type", dir->name);
			newDec = new WadContentX(wct, filePathName);
			newDec->addLump(dir);
			if (result == NULL) result=newDec;
			else result->mergeContent(newDec);
			dir->size = -1; //To delete
		} else {
			wxLogVerbose("Did not find lump for include %s", incl);
		}
	}
	deleteUnknown(); //Deletes entries with -1
	return result;
}

void WadStats::validateMaps()
{
	WadContentX* wcx = dynamic_cast<WadContentX*>(content[WMAP]);
	WadContentX* last = NULL;
	bool valid;
	while (wcx != NULL) {
		valid=false;
		//First lump is marker (name), must have at least one more lump
		if (wcx->lumps->size()<2) {
			lumpError("Unknown marker lump", new DirEntry(wcx->lumps->at(0)->name));
		} else { //Check for needed lumps
			if (wcx->containsLump("VERTEXES") && wcx->containsLump("LINEDEFS") && wcx->containsLump("SIDEDEFS") && wcx->containsLump("SECTORS"))
				valid=true;
			else if (wcx->containsLump("TEXTMAP"))
				valid=true;
			else
				lumpError("Map marker lump lacking map lumps", new DirEntry(wcx->lumps->at(0)->name));
		}
		if (!valid) {
			//Remove invalid map entry
			if (last==NULL)
				content[WMAP] = wcx->next;
			else
				last->next = wcx->next;
			WadContentX* temp = wcx->next;
			wcx->next = NULL;
			delete wcx;
			wcx = temp;
		} else {
			last = wcx;
			wcx = wcx->next;
		}
	}
}

void WadStats::deleteUnknown()
{
	WadContentX* wcx = dynamic_cast<WadContentX*>(content[WUNKNOWN]);
	vector<DirEntry*>* dirEntries = wcx->lumps;
	int deleted = 0;
	for (int i=0; i<dirEntries->size(); i++) {
		if (dirEntries->at(i)->size == -1)
			deleted++;
	}
	if (deleted > 0) {
		content[WUNKNOWN] = NULL;
		if (deleted < dirEntries->size()) {
			content[WUNKNOWN] = new WadContentX(WUNKNOWN, "");
			for (int i=0; i<dirEntries->size(); i++) {
				if (dirEntries->at(i)->size > -1)
					content[WUNKNOWN]->addLump(dirEntries->at(i));
			}
		}
		delete wcx; //Deletes old content[WUNKNOWN], with old dirEntries
	}
}

void WadStats::checkIwadEngine()
{
	if ((hexenLumps||hexenMap) && iwad!=IWAD_HEXEN) {
		if (engine < DENG_ZDOOM)
			engine = DENG_ZDOOM;
	}
	if (content[WDEHACKED] != NULL) {
		if (iwad==IWAD_HEXEN && engine==DENG_ORIGINAL) {
			iwad = IWAD_DOOM2;
			engine = DENG_ZDOOM;
		}
	}
}

/*
void WadStats::printReport(std::ostream& os)
{
	//Maps
	os << "* MAPS:\n";
	WadContentX* wcx = dynamic_cast<WadContentX*>(content[WMAP]);
	while (wcx != NULL) {
		os << wcx->getString() << '\n';
		wcx = wcx->next;
	}
	os << " -\n";
	for (int j=WSPRITE; j<WERROR; j++)
	{
		if (content[j] != NULL)
			os << content[j]->getString() << '\n';
	}
}*/

void WadStats::printReport(TextReport* reportView)
{
	reportView->writeHeading(fileName);
	reportView->writeLine("Type: "+wadType);
	reportView->writeLine(wxString::Format("File year: %i", year));
	reportView->writeLine(wxString::Format("File size: %i", fileSize));
	MD5 md5Hash;
	for (int i=0; i<16; i++)
		md5Hash.setBytedigest(i, md5Digest[i]);
	reportView->writeLine("MD5 hash: "+md5Hash.hexdigest());
	reportView->writeLine(wxString::Format("Total lumps: %i", numberOfLumps));
	if (content[WWAD]!=NULL)
		reportView->writeLine(wxString::Format("Internal wad files: %i", content[WWAD]->count));
	int mapCount = 0;
	WadContentX* wcx = getMapContent();
	while (wcx != NULL) {
		mapCount++;
		wcx = wcx->next;
	}
	reportView->writeLine(wxString::Format("Maps: %i", mapCount));
	mapCount = (content[WUNKNOWN]==NULL)? 0: content[WUNKNOWN]->count;
	reportView->writeLine(wxString::Format("Unknown lumps: %i", mapCount));
	mapCount = 0;
	if (content[WERROR]!=NULL) {
		wcx = dynamic_cast<WadContentX*>(content[WERROR]);
		while (wcx != NULL) {
			mapCount++;
			wcx = wcx->next;
		}
	}
	reportView->writeLine(wxString::Format("Lump errors: %i", mapCount));

	wxArrayInt tabs;
	tabs.Add(500);tabs.Add(700);
	reportView->setTabs(tabs);
	reportView->writeSubHeading("Graphical lumps\tCount\tBytes");
	//reportView->writeLine(" ");
	bool any = false;
	for (int j=WSPRITE; j<=WCOLORMAP; j++)
	{
		if (content[j] != NULL) {
			reportView->writeLine(content[j]->getString());
			any = true;
		}
	}
	if (!any)
		reportView->writeLine("None");

	reportView->writeSubHeading("Sound/music lumps");
	any = false;
	for (int j=WMUS; j<=WSNDDEF; j++)
	{
		if (content[j] != NULL) {
			reportView->writeLine(content[j]->getString());
			any = true;
		}
	}
	if (!any)
		reportView->writeLine("None");

	reportView->writeSubHeading("Script lumps");
	any = false;
	for (int j=WBEHAVIOR; j<=WMAPINFO; j++)
	{
		if (content[j] != NULL) {
			reportView->writeLine(content[j]->getString());
			any = true;
		}
	}
	if (!any)
		reportView->writeLine("None");

	reportView->writeSubHeading("Various lumps");
	any = false;
	for (int j=WZDEF; j<=WOTHER; j++)
	{
		if (content[j] != NULL) {
			reportView->writeLine(content[j]->getString());
			any = true;
		}
	}
	if (!any)
		reportView->writeLine("None");

	if (content[WUNKNOWN]!=NULL) {
		reportView->writeSubHeading("Unknown lumps");
		wcx = dynamic_cast<WadContentX*>(content[WUNKNOWN]);
		DirEntry* de;
		for (int j=0; j<wcx->lumps->size(); j++) {
			de = wcx->lumps->at(j);
			wxString str(de->name+" - %i bytes");
			reportView->writeLine(wxString::Format(str, de->size));
		}
	}

	if (content[WERROR]!=NULL) {
		reportView->writeSubHeading("Errors");
		wcx = dynamic_cast<WadContentX*>(content[WERROR]);
		while (wcx != NULL) {
			reportView->writeLine(wcx->name+" - "+wcx->lumps->at(0)->name);
			wcx = wcx->next;
		}
	}
}

WadContentX* WadStats::getMapContent()
{
	return dynamic_cast<WadContentX*>(content[WMAP]);
}

WadContentX* WadStats::getContent(WadContentType type)
{
	if (content[type]==NULL)
		return NULL;
	else
		return dynamic_cast<WadContentX*>(content[type]);
}
