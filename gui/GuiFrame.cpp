/*
* GuiFrame implementation
* First the main construction/management, followed by GuiParent-functions,
* event table macro, event handlers and private functions.
*/

#include "GuiFrame.h"
#include "GuiMinorDialogs.h"

#include "GuiPersonDialogs.h"
#include "GuiTagDialogs.h"
#include "GuiAspectDialog.h"
#include "../data/DecorateParser.h"
#include "GuiWadReport.h"
#include "GuiStatistics.h"

#include <limits> //DEBUG


//************************ GuiFrame ************************

//wxFrame constructor sets window style.
GuiFrame::GuiFrame(const wxString& title)
: wxFrame(0, -1, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE),
logText(NULL), logNull(NULL), wadReader(NULL), dataBase(NULL),
wadPanel(NULL), mapPanel(NULL), consoleText(NULL), editMode(false)
{
	//This constructor should create objects for handling settings and child
	//windows, initialize data members and create menubar, toolbar and statusbar.

	//GuiFrame has a pointer to a wxConfigBase-object, used to store application
	//settings. On Windows, the settings are stored in the registry. The object is
	//created here, and deleted by GuiMain.
	configObject = wxConfigBase::Create();
	//configObject->DeleteAll();

	//Creating settings object and loading from configObject:
	appSettings = new ApplicationSettings();
	appSettings->loadConfig(configObject);
	SSHOT_WIDTH = appSettings->getValue(IMG_WIDTH);
	SSHOT_HEIGHT = appSettings->getValue(IMG_HEIGHT);

	//Set the frame icon (icon declared in rc file, MS Windows only):
	#if defined(__WXMSW__)
    	SetIcon(wxICON(top_icon));
    #endif

    //setTitleLine(); //Set text on titlebar

    //Creating menubar:
    menuBar = new GuiMenubarManager();
    menuBar->createMenubar(this);

    //Creating toolbar:
	toolbarMan = new GuiToolbarManager();
	toolbarMan->createToolbar(this, true);

	//GuiFrame is given a status bar, to be shown at the bottom of the window at
	//all times. The array in SetStatusWidths() sets the widths of the fields.
	//A negative number means variable width, -2 means twice as big as -1. Text
	//can then be shown by calling SetStatusText() with the field number from 0
	//and up. Set first field width to 0 to not show tooltips.
	// 0: wxWidgets use this to show help for menus/tools, 0 width to not use
	// 1: Database folder
	// 2: Current view wad/map count
	// 3: Total stats: Wads, Maps, Authors
	CreateStatusBar(4);
	int statusWidths[]={0,-1,-1,-2};
	SetStatusWidths(4, statusWidths);

	SetSizeHints(APP_MINSIZE); //Defines minimum size

    //Set window position and size. Values are taken from the config, or the defaults
	//defined in GuiBase are used.
	bool maxim;
	configObject->Read("/TopFrame/max", &maxim, false);
	if (maxim)
	{
		//Window was maximized. Set default size before maximizing.
		Move(APP_POINT);
		SetClientSize(APP_SIZE);
		Maximize(true);
	}
	else
	{
		//Get old position/size:
		int x = (int) configObject->Read("/TopFrame/x", APP_POINT.x);
		int y = (int) configObject->Read("/TopFrame/y", APP_POINT.y);
		int w = (int) configObject->Read("/TopFrame/w", APP_SIZE.GetWidth());
		int h = (int) configObject->Read("/TopFrame/h", APP_SIZE.GetHeight());
		if ( (w < APP_MINSIZE.GetWidth()) || (h < APP_MINSIZE.GetHeight()) )
		{
			//Too small, reset:
			Move(APP_POINT);
			SetClientSize(APP_SIZE);
		} else {
			Move(x,y);
			SetClientSize(w,h);
		}
	}

	//The main client area is managed by a wxSplitterWindow to allow showing panel on the left side.
	clientArea = new wxSplitterWindow(this, FRAME_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
	//Different sash styles don't work?

	int sashPos = (int)configObject->Read("/TopFrame/left", 170);
	viewSelect = new GuiViewSelect(clientArea, this);
	mainArea = new wxSplitterWindow(clientArea, FRAME_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
	//mainArea->SetSashGravity(0.5);
	clientArea->SplitVertically(viewSelect, mainArea, sashPos);
	clientArea->SetMinimumPaneSize(40);

	bool split = appSettings->getFlag(UI_SIDEPANEL);
	sashPos = (int)configObject->Read("/TopFrame/right", -200);
	centerArea = new wxSplitterWindow(mainArea, FRAME_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
	//centerArea->SetSashGravity(0.5);
	if (split) {
		wadPanel = new GuiWadPanel(mainArea);
		mainArea->SplitVertically(centerArea, wadPanel, sashPos);
	} else {
		mainArea->Initialize(centerArea);
	}
	mainArea->SetMinimumPaneSize(40);
	split = appSettings->getFlag(UI_LOGPANEL);
	sashPos = (int)configObject->Read("/TopFrame/bottom", -100);
	mapList = new GuiEntryList(centerArea, this, appSettings);
	if (split) {
		consoleText = new wxTextCtrl(centerArea, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTE_READONLY | wxTE_MULTILINE | wxTE_BESTWRAP);
		//consoleText->SetMaxLength(20000);
		//consoleText->SetScrollbar(wxVERTICAL, 1, 200, 200, true);
		//clientArea->Initialize(consoleText);
		centerArea->SplitHorizontally(mapList, consoleText, sashPos);
	} else {
		consoleText = NULL;
		centerArea->Initialize(mapList);
	}
	centerArea->SetMinimumPaneSize(40);

	wxLog::SetComponentLevel("dmdb", wxLOG_Info);
	wxLog::SetVerbose(true);
	if (consoleText != NULL) {
		logText = new wxLogTextCtrl(consoleText);
		wxLog::SetActiveTarget(logText);
		logNull = NULL;
	} else {
		logText = NULL;
		logNull = new wxLogNull();
	}
	wxLogVerbose("Application started");

	wadReader = new WadReader();
	wadReader->setThingFile(1, appSettings->getFullPath(TDG_DOOM));
	wadReader->setThingFile(2, appSettings->getFullPath(TDG_ZDOOM));
	wadReader->setThingFile(3, appSettings->getFullPath(TDG_SKULLT));
	wadReader->setThingFile(4, appSettings->getFullPath(TDG_HERET));

	WadStatAspects* aspects = new WadStatAspects();
	configObject->SetPath("/Aspects");
	aspects->wadFile = configObject->ReadBool("wadFile", true);
	aspects->wadFlags = configObject->ReadBool("wadFlags", true);
	aspects->mapMain = configObject->ReadBool("mapMain", true);
	aspects->gameModes = configObject->ReadBool("gameModes", true);
	aspects->mapStats = configObject->ReadBool("mapStats", true);
	aspects->gameStats = configObject->ReadBool("gameStats", true);
	aspects->mapImages = configObject->ReadBool("mapImages", true);
	configObject->SetPath("..");
	wadReader->setAspects(aspects);

	//Set database:
	wxString dbFolder;
	if ( configObject->Read("DB_FOLDER", &dbFolder) ) {
		wxFileName ftest(dbFolder);
		if (ftest.DirExists()) {
			setDatabase(dbFolder);
		} else {
			wxLogVerbose("Could not find %s", dbFolder);
			toolbarMan->enableItems(false);
			menuBar->enableItems(false);
		}
	} else {
		toolbarMan->enableItems(false);
		menuBar->enableItems(false);
		selectDatabase();
	}
}

void GuiFrame::selectDatabase()
{
	wxMessageDialog* dlg = new wxMessageDialog(this,
		"Select a folder to open or create a database. A database is housed in a folder in your file system. If you want to open an existing database, select this folder. If you want to start a new database, create or select an empty folder. You are advised to make backup copies of this folder.",
		"Select database", wxOK|wxCANCEL|wxICON_INFORMATION|wxCENTRE);
	int result = dlg->ShowModal();
	dlg->Destroy();
	if (result != wxID_OK) {
		//No change
		return;
	}

	wxDirDialog* ddlg = new wxDirDialog(this, "Select database folder", "", wxDD_DEFAULT_STYLE);
	if (ddlg->ShowModal() == wxID_OK) {
		wxString dbFolder = ddlg->GetPath();
		ddlg->Destroy();
		configObject->Write("DB_FOLDER", dbFolder);
		if (dataBase != NULL)
			closeDatabase();
		setDatabase(dbFolder);
	} else {
		ddlg->Destroy();
		//No folder selected
	}
}

void GuiFrame::setDatabase(const wxString& folder)
{
	dataBase = new DataManager(this);
	dataBase->setFolder(folder);
	dataBase->setAuthorNaming( AuthorNaming(appSettings->getValue(DB_AUTHORNAME)) );
	SetStatusText(folder, 1);
	menuBar->enableItems(dataBase != NULL);
	toolbarMan->enableItems(dataBase != NULL);

	wadReader->setTempFolder(dataBase->getTempFolder());
	dataBase->load();
	if (wadPanel != NULL)
		wadPanel->setDataManager(dataBase);
	if (mapPanel != NULL)
		mapPanel->setDataManager(dataBase);
	WadMapFields wadSort = WadMapFields(appSettings->getValue(UI_WADSORT));
	WadMapFields mapSort = WadMapFields(appSettings->getValue(UI_MAPSORT));
	viewSelect->populate(dataBase, wadSort, mapSort);
	mapList->setDataManager(dataBase);
	mapList->loadListFields();
	mapList->createList(0);
	//TODO: Make sure wadPanel/mapPanel is updated to reflect new database!
}

void GuiFrame::closeDatabase()
{
	toolbarMan->enableItems(false);
	menuBar->enableItems(false);
	SetStatusText("", 1);
	SetStatusText("", 2);
	SetStatusText("", 3);

	if (wadPanel != NULL)
		wadPanel->setDataManager(NULL);
	if (mapPanel != NULL)
		mapPanel->setDataManager(NULL);
	viewSelect->removeDataManager();
	mapList->saveListFields();
	mapList->setDataManager(NULL);

	dataBase->saveDataFilters();
	if (dataBase->unsavedWadMapChanges()) //own stuff in panels
		dataBase->saveWadsMaps();
	appSettings->setValue(UI_WADSORT, dataBase->getWadFilter()->sortField);
	appSettings->setValue(UI_MAPSORT, dataBase->getMapFilter()->sortField);
	delete dataBase;
	dataBase = NULL;
}

GuiFrame::~GuiFrame()
{
	//Window position and size is saved:
	int x, y, w, h;
	GetPosition(&x, &y);
	GetClientSize(&w, &h);
	bool maxim = IsMaximized();
	configObject->Write("/TopFrame/x", (long) x);
	configObject->Write("/TopFrame/y", (long) y);
	configObject->Write("/TopFrame/w", (long) w);
	configObject->Write("/TopFrame/h", (long) h);
	configObject->Write("/TopFrame/max", maxim);
	configObject->Write("/TopFrame/left", (long)clientArea->GetSashPosition());
	if (mainArea->IsSplit())
		configObject->Write("/TopFrame/right", (long)mainArea->GetSashPosition());
	if (centerArea->IsSplit())
		configObject->Write("/TopFrame/bottom", (long)centerArea->GetSashPosition());

	//Child windows are destroyed automatically by wxWidgets. The config object
	//is deleted by GuiMain.
	//Any other objects should be deleted here.
	delete wadReader;
	delete appSettings;
}

//************************ GuiFrame GuiParent ************************

wxPoint GuiFrame::getDialogPos(int width, int height)
{
	//If zero, use default values for dialog size:
	if (width==0) width=80;
	if (height==0) height=60;
	//Find the position that centers the dialog on the window:
	wxPoint p=GetPosition();
	wxSize s=GetSize();
	wxPoint dialogPoint=wxPoint(0,0);
	dialogPoint.x = p.x + (s.GetWidth()/2) - (width/2);
	if (dialogPoint.x < 0) dialogPoint.x=0;
	dialogPoint.y = p.y + (s.GetHeight()/2) - (height/2);
	if (dialogPoint.y < 0) dialogPoint.y=0;
	return dialogPoint;
}

//************************ GuiFrame DataStatusListener ************************

void GuiFrame::onWadFilter(const wxString& name, unsigned int wadCount)
{
	SetStatusText(name + ": " + wxIntToString(wadCount) + " wads", 2);
}

void GuiFrame::onMapFilter(const wxString& name, unsigned int mapCount)
{
	SetStatusText(name + ": " + wxIntToString(mapCount) + " maps", 2);
}

void GuiFrame::onTotalCounts(long wads, long maps, long auths)
{
	SetStatusText(wxString::Format("Total content: [%i] wads  [%i] maps  [%i] people", wads, maps, auths), 3);
}

//************************ Event table macro ************************
//This connects wxWidgets events with the functions (event handlers) which
//process them. All events processed by the frame are listed here.
BEGIN_EVENT_TABLE(GuiFrame, wxFrame)
	//File menu:
	EVT_MENU(LFILE_OPEN, GuiFrame::onDatabase)
	EVT_MENU(LFILE_MODE, GuiFrame::toggleEditMode)
	//EVT_MENU(LFILE_NEW, GuiFrame::onFileNew)
	EVT_MENU(LFILE_READ, GuiFrame::onFileRead)
    EVT_MENU(LFILE_READALL, GuiFrame::onFolderRead)
	EVT_MENU(LFILE_CLOSE, GuiFrame::onClose)
    EVT_MENU(LFILE_EXIT, GuiFrame::onExit)
    //Data menu:
	EVT_MENU(LDATA_PEOPLE, GuiFrame::onPeople)
	EVT_MENU(LDATA_TAGS, GuiFrame::onTags)
    EVT_MENU(LTHING_NEW, GuiFrame::onNewThing)
	EVT_MENU(LTHING_OPEN, GuiFrame::onLoadThing)
	EVT_MENU(LTHING_DOOM, GuiFrame::onLoadThing)
	EVT_MENU(LTHING_ZDOOM, GuiFrame::onLoadThing)
	EVT_MENU(LTHING_SKULLT, GuiFrame::onLoadThing)
	EVT_MENU(LTHING_HERET, GuiFrame::onLoadThing)
    //Stats menu:
	EVT_MENU(LSTATS_TOTAL, GuiFrame::onStatsTotal)
	EVT_MENU(LSTATS_YEARS, GuiFrame::onStatsYears)
	EVT_MENU(LSTATS_IWAD, GuiFrame::onStatsIwad)
	EVT_MENU(LSTATS_ENGINE, GuiFrame::onStatsEngine)
	EVT_MENU(LSTATS_RATING, GuiFrame::onStatsRating)
	EVT_MENU(LSTATS_DIFFIC, GuiFrame::onStatsDifficulty)
	EVT_MENU(LSTATS_PLAYST, GuiFrame::onStatsPlaystyle)
	EVT_MENU(LSTATS_PEOPLE, GuiFrame::onStatsPeople)
	EVT_MENU(LSTATS_TAGS, GuiFrame::onStatsTags)
	//Options menu:
	EVT_MENU(LOPT_TEST, GuiFrame::onTest)
	EVT_MENU(LOPT_APPSETTINGS, GuiFrame::onAppSettings)
	//EVT_MENU(LOPT_LOG, GuiFrame::onShowLog)
    EVT_MENU(LHELP_ABOUT, GuiFrame::onAbout)
    EVT_TEXT(SEARCH_FIELD, GuiFrame::onSearchText)
    //EVT_SEARCH(SEARCH_FIELD, GuiFrame::onSearch) Not found
    //EVT_SEARCH_CANCEL(SEARCH_FIELD, GuiFrame::onSearchCancel) Not found
    //Other events:

    EVT_CLOSE(GuiFrame::onClose)
    //EVT_SIZE(GuiFrame::onSize)
	//EVT_MENU_HIGHLIGHT(wxID_ANY, GuiFrame::onMenuHighlight) wxFrame shows help text on statusbar
	//EVT_TOOL_ENTER(wxID_ANY, GuiFrame::onToolEnter) wxFrame shows help text on statusbar
	//EVT_SET_FOCUS(GuiFrame::onSetFocus)
	//EVT_NOTEBOOK_PAGE_CHANGED(SIDE_PANELS, GuiFrame::onSidePanel)
	//EVT_SPLITTER_SASH_POS_CHANGED(FRAME_SPLITTER, GuiFrame::onSplitter)
END_EVENT_TABLE()

//************************ GuiFrame Event-handlers ************************

void GuiFrame::toggleEditMode(wxCommandEvent& event)
{
	editMode = !editMode;
	menuBar->setItemState(LFILE_MODE, editMode);
	toolbarMan->setToolState(LFILE_MODE, editMode);
}

void GuiFrame::onDatabase(wxCommandEvent& event)
{
	selectDatabase();
}

void GuiFrame::onFileNew(wxCommandEvent& event)
{
	//Create new database
}

void GuiFrame::onFolderRead(wxCommandEvent& event)
{
	//Get folder from dialog
	wxString folder;
	wxDirDialog* ddlg = new wxDirDialog(this, "Select folder", "", wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST|wxDD_CHANGE_DIR);
	if (ddlg->ShowModal() == wxID_OK) {
		folder = ddlg->GetPath();
		ddlg->Destroy();
	} else {
		ddlg->Destroy();
		return; //No folder selected
	}
	wxArrayString* files = new wxArrayString();
	int fcount = wxDir::GetAllFiles(folder, files, wxEmptyString, wxDIR_FILES);
	wxFileName fname;
	wxString ext;
	for (int i=fcount-1; i>=0; i--){
		fname = wxFileName((*files)[i]);
		ext = fname.GetExt();
		if (ext.IsSameAs("wad", false) || ext.IsSameAs("zip", false) || ext.IsSameAs("pk3"))
			;//Supported file
		else
			files->RemoveAt(i,1);
	}
	fcount = files->GetCount();
	if (fcount == 0) {
		wxMessageDialog* dlg = new wxMessageDialog(this, "No relevant files found (zip/wad/pk3).",
			"No files found", wxOK | wxICON_WARNING);
		dlg->ShowModal();
		dlg->Destroy();
		delete files;
		return;
	}
	bool replExisting = false;
	MapEntry* mapTemp = NULL;

	//Ask iwad/engine, to override best guess
	GuiWadType* typeDialog = new GuiWadType(this, getDialogPos(200,200),
		IWAD_NONE, DENG_NONE, fcount);
	int result = typeDialog->ShowModal();
	IwadType iwad = (IwadType)typeDialog->getIwad();
	EngineType engine = (EngineType)typeDialog->getEngine();
	typeDialog->Destroy();
	if (result==BUTTON_CANCEL) {
		delete files;
		return;
	}
	if (result == BUTTON_EDIT) {
		//Ask which aspects to set/update, and whether to replace existing entries
		WadStatAspects* aspects = wadReader->getAspects();
		GuiAspectDialog* aspectDialog = new GuiAspectDialog(this, getDialogPos(300,300), aspects,
			"Aspects to set", "Set which fields to fill in when creating or updating database entries:", true);
		result = aspectDialog->ShowModal();
		replExisting = aspectDialog->getReplaceFlag();
		aspectDialog->Destroy();
	}
	if (result==BUTTON_CANCEL) {
		delete files;
		return;
	}
	if (result == BUTTON_EDIT) {
		//Dialog to set common map properties
		mapTemp = new MapEntry(0);
		mapTemp->singlePlayer = 4;
		mapTemp->cooperative = 4;
		mapTemp->deathmatch = 4;
		mapTemp->played = 5;
		GuiMapCommon* mapDialog = new GuiMapCommon(this, getDialogPos(300,400),
			mapTemp, dataBase, true);
		result = mapDialog->ShowModal();
		mapDialog->Destroy();
		if (result != BUTTON_OK) {
			delete mapTemp;
			mapTemp = NULL;
		}
	}
	if (result==BUTTON_CANCEL) {
		delete files;
		return;
	}

	//Loop over each file found (zip, pk3, wad)
	int wadsAdded = 0;
	int mapsAdded = 0;
	int wadsUpdated = 0;
	WadProgress* progress = new WadProgress("Process files in folder");
	GuiProgress* progDialog = new GuiProgress(this, getDialogPos(400,600), wxSize(400,600), true);
	progDialog->Show();
	progress->setDialog(progDialog);
	progress->startCount(fcount*2);
	WadEntry* wadEntry;
	wxString imgFolder = dataBase->getMapImgFolder();
	for (int i=0; i<fcount; i++) {
		fname = wxFileName((*files)[i]);
		//wxLogVerbose("Process file %s", (*files)[i]);
		TaskProgress* fileProg = new TaskProgress(fname.GetFullName(), progress);
		wadReader->initReader((*files)[i]);
		wadReader->processWads(fileProg); //Creates WadStats for each wad/pk3, best-guess iwad and engine
		if (!fileProg->hasFailed()) {
			if (iwad != IWAD_NONE)
				wadReader->setIwad(iwad);
			if (engine != DENG_NONE)
				wadReader->setEngine(engine);
			wadReader->findThingDefs(fileProg); //Set correct thingDefs, process decorate and dehacked
		}
		progress->incrCount();
		if (!fileProg->hasFailed()) {
			unsigned char* chptr = wadReader->getMainDigest();
			wadEntry = dataBase->findWad(chptr); //If DB has match, skip or update existing?
			//wadReader->getMainFileName(); //If DB has match, add anyway or update existing?
			if (wadEntry == NULL) {
				wadEntry = wadReader->createEntries(imgFolder, fileProg, mapTemp); //Create new entry
				if (!fileProg->hasFailed()) {
					wadsAdded++;
					mapsAdded += wadEntry->numberOfMaps;
					dataBase->addWad(wadEntry);
				}
			} else if (replExisting) {
				bool deleted = wadReader->updateEntries(wadEntry, imgFolder, fileProg); //Update entry
				if (!fileProg->hasFailed()) {
					wadsUpdated++;
					if (deleted)
						deleteRemovedMaps(wadEntry);
					dataBase->wadModified(wadEntry);
				}
			} else {
				//Skipping existing wads - log with warnError
				fileProg->warnError("Existing entry found, skipping");
			}
		}
		delete fileProg;
		wadReader->clearState();
		progress->incrCount();
	}
	progress->completeCount();
	progDialog->logLine(wxString::Format("Added %i wad entries with %i maps",wadsAdded,mapsAdded));
	if (wadsUpdated > 0)
		progDialog->logLine(wxString::Format("Updated %i wad entries",wadsUpdated));
	progDialog->finished();
	delete files;
	delete progress;
	mapList->refreshList();
	dataBase->saveWadsMaps();
	//TODO: Go to new wad list?
}

void GuiFrame::deleteRemovedMaps(WadEntry* wadEntry)
{
	MapEntry* mapEntry = wadEntry->mapPointers[wadEntry->numberOfMaps - 1];
	while (mapEntry->ownFlags & OFLG_DELETE) {
		dataBase->deleteMap(mapEntry);
		mapEntry = wadEntry->mapPointers[wadEntry->numberOfMaps - 1];
	}
}

void GuiFrame::onFileRead(wxCommandEvent& event)
{
	//Get filename from dialog
	wxFileDialog* fdlg = new wxFileDialog(this, "Open file", "", "",
		"Content files (zip/wad/pk3)|*.zip;*.wad;*.pk3|Zip files (*.zip)|*.zip|Wad files (*.wad)|*.wad|Pk3 files (*.pk3)|*.pk3|All files (*.*)|*.*",
		wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
    fdlg->SetFilterIndex(appSettings->getValue(UI_FILETYPE));
	if (fdlg->ShowModal() == wxID_OK) {
        appSettings->setValue(UI_FILETYPE, fdlg->GetFilterIndex());
		wadReader->initReader(fdlg->GetPath());
		fdlg->Destroy();
	} else {
		fdlg->Destroy();
		return; //No file selected
	}
	//Parse wadStats from file
	WadProgress* progress = new WadProgress("Process file");
	int result = BUTTON_CANCEL;
	GuiProgress* progDialog = new GuiProgress(this, getDialogPos(320,300));
	progDialog->Show();
	progress->setDialog(progDialog);
	wadReader->processWads(progress); //Try to load file(s)
	progDialog->Close(true);
	progDialog->Destroy();
	progress->setDialog(NULL);
	if (progress->hasFailed()) {
		wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
			"File analysis failed", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		GuiWadType* typeDialog = new GuiWadType(this, getDialogPos(200,200),
			wadReader->getIwad(), wadReader->getEngine());
		if (typeDialog->ShowModal() == BUTTON_OK) {
			wadReader->setIwad((IwadType)typeDialog->getIwad());
			wadReader->setEngine((EngineType)typeDialog->getEngine());
		}
		typeDialog->Destroy();

		//Set/parse thingDefs
		wxBeginBusyCursor(wxHOURGLASS_CURSOR);
		wadReader->findThingDefs(progress);
		wxEndBusyCursor();
		if (progress->hasFailed()) {
			wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
				"File error", wxOK | wxICON_ERROR);
			dlg->ShowModal();
			dlg->Destroy();
		} else {
			GuiWadReport* wadDialog = new GuiWadReport(this, getDialogPos(300,400), wadReader);
			result = wadDialog->ShowModal();
			wadDialog->Destroy();
		}
	}
	delete progress;

	if (result == BUTTON_OK) {
		//Check if we have WadEntry with same MD5 hash
		unsigned char* chptr = wadReader->getMainDigest();
		WadEntry* replaceWad = dataBase->findWad(chptr);
		bool changeHash = false;
		if (replaceWad != NULL) {
			wxString mes = "There is already a Wad entry for this file (matching file hash), named "
				+ replaceWad->getName() + ". A duplicate entry is not allowed. Do you wish to "
				+ "update the existing entry?";
			wxMessageDialog* dlg = new wxMessageDialog(this, mes,
				"Matching Wad entry found", wxYES_NO | wxICON_WARNING);
			int result = dlg->ShowModal();
			dlg->Destroy();
			if (result == wxID_NO) {
				wadReader->clearState();
				return;
			}
		} else {
			//Check if we have WadEntry with same name
			replaceWad = dataBase->findWad(wadReader->getMainFileName());
			if (replaceWad != NULL) {
				wxString mes = "There is already a Wad entry for this filename, named "
					+ replaceWad->getName() + ", but the file is different (file hash does "
					+ "not match). Do you want to update this entry? Select No to create "
					+ "a new Wad entry.";
				wxMessageDialog* dlg = new wxMessageDialog(this, mes,
					"Matching filename found", wxYES_NO);
				int result = dlg->ShowModal();
				dlg->Destroy();
				if (result == wxID_NO)
					replaceWad = NULL;
				else
					changeHash = true;
			}
		}

		wxString imgFolder = dataBase->getMapImgFolder();
		if (replaceWad != NULL) {
			//Show aspects dialog:
			WadStatAspects* aspects = wadReader->getAspects();
			GuiAspectDialog* aspectDialog = new GuiAspectDialog(this, getDialogPos(200,200), aspects,
				"Aspects to update", "Select which aspects to update:");
			int result = aspectDialog->ShowModal();
			aspectDialog->Destroy();
            if (result == BUTTON_OK) {
                //Replace WadEntry
                GuiProgress* progDialog = new GuiProgress(this, getDialogPos(320,300));
				progDialog->Show();
				progress = new WadProgress("Process maps");
				progress->setDialog(progDialog);
				if (changeHash) { //Must rename files identified by wad hash
					MD5 md5Hash;
					for (int i=0; i<16; i++)
						md5Hash.setBytedigest(i, chptr[i]);
					dataBase->renameFiles(replaceWad, md5Hash.hexdigest());
				}
                bool deleted = wadReader->updateEntries(replaceWad, imgFolder, progress, changeHash);
                progress->setDialog(NULL);
				progDialog->Close(true);
				progDialog->Destroy();
				if (progress->hasFailed()) {
					wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
						"Database update failed", wxOK | wxICON_ERROR);
					dlg->ShowModal();
					dlg->Destroy();
				} else {
					if (deleted)
						deleteRemovedMaps(replaceWad);
					dataBase->wadModified(replaceWad);
					mapList->refreshList();
					//TODO: Show wad/map dialog? Mark map?
					dataBase->saveWadsMaps();
					wxMessageDialog* dlg = new wxMessageDialog(this, "Wad/Map entries updated.",
						"Finished", wxOK);
					dlg->ShowModal();
					dlg->Destroy();
				}
                delete progress;
            }

		} else {
			MapEntry* mapTemp = NULL;
			if (wadReader->numberOfMaps() > 1) {
				//Dialog to set common map properties
				mapTemp = new MapEntry(0);
				mapTemp->singlePlayer = 4;
				mapTemp->cooperative = 4;
				mapTemp->deathmatch = 4;
				mapTemp->played = 5;
				GuiMapCommon* mapDialog = new GuiMapCommon(this, getDialogPos(300,400),
					mapTemp, dataBase);
				int result = mapDialog->ShowModal();
				mapDialog->Destroy();
				if (result != BUTTON_OK) {
					delete mapTemp;
					mapTemp = NULL;
				}
			}

			//Create WadEntry
			GuiProgress* progDialog = new GuiProgress(this, getDialogPos(320,300));
			progDialog->Show();
			progress = new WadProgress("Process maps");
			progress->setDialog(progDialog);
			WadEntry* wadEntry = wadReader->createEntries(imgFolder, progress, mapTemp);
			//Map images have been created for the new WadEntry
			progress->setDialog(NULL);
			progDialog->Close(true);
			progDialog->Destroy();
			if (mapTemp != NULL) delete mapTemp;
			mapTemp = NULL;

			if (progress->hasFailed()) {
				if (wadEntry != NULL) delete wadEntry;
				wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
					"Database entry failed", wxOK | wxICON_ERROR);
				dlg->ShowModal();
				dlg->Destroy();
			} else {
				int x = (int) configObject->Read("/WadEntry/x", 300);
				int y = (int) configObject->Read("/WadEntry/y", 200);
				int w = (int) configObject->Read("/WadEntry/w", 900);
				int h = (int) configObject->Read("/WadEntry/h", 600);
				ListWrapper<WadEntry*>* wrapper = new ListWrapper<WadEntry*>();
				wrapper->add(wadEntry);
				wrapper->reset();
				GuiWadEntry* wadDialog = new GuiWadEntry(this, wxPoint(x,y), wxSize(w,h),
					wrapper, dataBase, DDMODE_NEW, true);
				int result = wadDialog->ShowModal();
				wadDialog->savePlacement(configObject);
				wadDialog->Destroy();
				delete wrapper;
				if (result == BUTTON_OK) {
					dataBase->addWad(wadEntry);
					mapList->refreshList();
					//TODO: Mark new map?
					dataBase->saveWadsMaps();
				} else {
					//Must delete images created for this wad
					dataBase->deleteWadFiles(wadEntry);
					delete wadEntry;
				}
			}
			delete progress;
		}
	}
	wadReader->clearState();
}

/* Not used
void GuiFrame::onFileSave(wxCommandEvent& event)
{
	if (event.GetId()==LFILE_SAVE) //Save to file with current name
		saveFile();
	else //LFILE_SAVEAS, Save to file with new name
		saveFile(true);
}*/

void GuiFrame::onPeople(wxCommandEvent& event)
{
	DoomDialogMode m = (editMode)? DDMODE_EDIT: DDMODE_VIEW;
	dataBase->openAuthorTextFile();
	GuiPersonList* personDialog = new GuiPersonList(this, getDialogPos(300,400), dataBase, m);
	personDialog->ShowModal();
	personDialog->Destroy();
	dataBase->closeAuthorTextFile();
}

void GuiFrame::onTags(wxCommandEvent& event)
{
    DoomDialogMode m = (editMode)? DDMODE_EDIT: DDMODE_VIEW;
	GuiTagList* tagDialog = new GuiTagList(this, getDialogPos(300,400), dataBase, m);
	tagDialog->ShowModal();
	tagDialog->Destroy();
}

void GuiFrame::onClose(wxCommandEvent& event)
{
	if (dataBase != NULL)
		closeDatabase();
	appSettings->saveConfig(configObject);
}

void GuiFrame::onExit(wxCommandEvent& event)
{
	Close(false); //Calls onClose, FALSE means can veto (cancel)
}

void GuiFrame::onStatsTotal(wxCommandEvent& event)
{
	MapStatistics* stats = dataBase->getStatistics(mapList->getCurrentType());
	StatisticsReport* report = new StatisticsReport(this, getDialogPos(300,400), stats);
	int result = report->ShowModal();
	report->Destroy();
	delete stats;
}

void GuiFrame::onStatsYears(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getYearStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsIwad(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getIwadStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsEngine(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getEngineStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsRating(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getRatingStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}


void GuiFrame::onStatsDifficulty(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getDifficultyStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsPlaystyle(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getPlaystyleStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsPeople(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getAuthorStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onStatsTags(wxCommandEvent& event)
{
	StatisticSet* stats = dataBase->getTagStats(mapList->getCurrentType());
	GuiStatistics* dialog = new GuiStatistics(this, getDialogPos(300,400), stats);
	int result = dialog->ShowModal();
	dialog->Destroy();
	delete stats;
}

void GuiFrame::onNewThing(wxCommandEvent& event)
{
	wxString folder;
	wxDirDialog* ddlg = new wxDirDialog(this, "Select Decorate folder", "", wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST);
	if (ddlg->ShowModal() == wxID_OK) {
		folder = ddlg->GetPath();
		ddlg->Destroy();
	} else {
		ddlg->Destroy();
		return; //No folder selected
	}
	DecorateParser* parser = new DecorateParser();
	parser->parseFiles(folder);
	ThingDefList* things = parser->extractThingDefs();
	things->setModified(false);
	wxLogVerbose("Got list of %i thing definitions", things->size());
	delete parser;
	thingDialog(things);
}

void GuiFrame::thingDialog(ThingDefList* things)
{
	GuiThingDialog* dialog = new GuiThingDialog(this, getDialogPos(300,400), things);
	int result = dialog->ShowModal();
	dialog->Destroy();
	if (result == BUTTON_OK) {
		//Save to file
		wxFileDialog* fdlg=new wxFileDialog(this, "Save Thing definition", "", "",
			"Thing files (*.thg)|*.thg", wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
		if (fdlg->ShowModal()==wxID_OK)
		{
			wxString fileName=fdlg->GetPath();
			try {
				things->saveDefs(fileName);
			} catch (GuiError e) {
				wxMessageDialog* dlg = new wxMessageDialog(this, e.text1,
					"File error", wxOK | wxICON_ERROR);
				dlg->ShowModal();
				dlg->Destroy();
			}
			fdlg->Destroy();
		} else {
			fdlg->Destroy();
		}
	}
	delete things;
}

void GuiFrame::onLoadThing(wxCommandEvent& event)
{
	wxString fileName="";
	if (event.GetId() == LTHING_DOOM)
		fileName = appSettings->getFullPath(TDG_DOOM);
	else if (event.GetId() == LTHING_ZDOOM)
		fileName = appSettings->getFullPath(TDG_ZDOOM);
	else if (event.GetId() == LTHING_SKULLT)
		fileName = appSettings->getFullPath(TDG_SKULLT);
	else if (event.GetId() == LTHING_HERET)
		fileName = appSettings->getFullPath(TDG_HERET);

	if (fileName.Length() == 0) {
		//Get filename from dialog:
		bool fileOK=false;
		wxFileDialog* fdlg = new wxFileDialog(this, "Open Thing definition", "", "",
			"Thing files (*.thg)|*.thg|All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
		if (fdlg->ShowModal()==wxID_OK)
		{
			fileName=fdlg->GetPath();
			fileOK=true;
		}
		fdlg->Destroy();
		if (!fileOK) return; //No file selected
	}

	ThingDefList* things = new ThingDefList();
	things->loadDefs(fileName);
	thingDialog(things);
}

void GuiFrame::onTest(wxCommandEvent& event)
{
	wxLogVerbose("Show Test dialog");
	wxString testStr = "Size of int: " + wxIntToString(sizeof(int)) +
		"\nMaximum values: " + wxIntToString(numeric_limits<int>::max());
	wxMessageDialog* dlg = new wxMessageDialog(this, testStr, "Test", wxOK);
	dlg->ShowModal();
	dlg->Destroy();
}

void GuiFrame::onNothing(wxCommandEvent& event)
{
	vector<int>* vectorPointer = new vector<int>(10);
	for (int i=0; i<10; i++)
		vectorPointer->at(i) = i;
	wxString testStr = "Value 5 is " + wxIntToString(vectorPointer->at(5));
	wxMessageDialog* dlg = new wxMessageDialog(this, testStr, "Test", wxOK);
	dlg->ShowModal();
	dlg->Destroy();
}

void GuiFrame::onAppSettings(wxCommandEvent& event)
{
	if (dataBase != NULL)
		mapList->saveListFields();
	//Create a dialog to set application settings. getDialogPos() is given an approximate
	//dialog size to find a good position.
	GuiSettingsDialog* dlg = new GuiSettingsDialog(this, getDialogPos(300,400), appSettings);
	int result=dlg->ShowModal();
	dlg->Destroy();
	if (result) //User chose 'Ok'
	{
		appSettings->saveConfig(configObject);
		if (dataBase != NULL) {
			dataBase->setAuthorNaming( AuthorNaming(appSettings->getValue(DB_AUTHORNAME)) );
			mapList->loadListFields(); //Update list columns
		}
		if (wadReader != NULL) {
			wadReader->setThingFile(1, appSettings->getFullPath(TDG_DOOM));
			wadReader->setThingFile(2, appSettings->getFullPath(TDG_ZDOOM));
			wadReader->setThingFile(3, appSettings->getFullPath(TDG_SKULLT));
			wadReader->setThingFile(4, appSettings->getFullPath(TDG_HERET));
		}
		updateSplitters();
		SSHOT_WIDTH = appSettings->getValue(IMG_WIDTH);
		SSHOT_HEIGHT = appSettings->getValue(IMG_HEIGHT);
	} else {
		//Reload old settings:
		appSettings->loadConfig(configObject);
	}
}

void GuiFrame::updateSplitters()
{
	int sashPos;
	bool split = appSettings->getFlag(UI_SIDEPANEL);
	if (split && (wadPanel==NULL) && (mapPanel==NULL)) { //Show sidepanel
		sashPos = (int)configObject->Read("/TopFrame/right", -200);
		if (mapList->getCurrentType() == 1) {
			mapPanel = new GuiMapPanel(mainArea);
			mapPanel->setDataManager(dataBase);
			mainArea->SplitVertically(centerArea, mapPanel, sashPos);
		} else {
			wadPanel = new GuiWadPanel(mainArea);
			wadPanel->setDataManager(dataBase);
			mainArea->SplitVertically(centerArea, wadPanel, sashPos);
		}
	} else if (!split && (wadPanel!=NULL)) { //Remove sidepanel
		wadPanel->readChanges();
		mainArea->Unsplit(wadPanel);
		delete wadPanel;
		wadPanel = NULL;
	} else if (!split && (mapPanel!=NULL)) { //Remove sidepanel
		mapPanel->readChanges();
		mainArea->Unsplit(mapPanel);
		delete mapPanel;
		mapPanel = NULL;
	}

	split = appSettings->getFlag(UI_LOGPANEL);
	if (split && (consoleText==NULL)) { //Show log
		sashPos = (int)configObject->Read("/TopFrame/bottom", -100);
		consoleText = new wxTextCtrl(centerArea, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 | wxTE_READONLY | wxTE_MULTILINE | wxTE_BESTWRAP);
		centerArea->SplitHorizontally(mapList, consoleText, sashPos);
		logText = new wxLogTextCtrl(consoleText);
		wxLog::SetActiveTarget(logText);
		if (logNull != NULL)
			delete logNull;
		logNull = NULL;
	} else if (!split && (consoleText!=NULL)) { //Remove log
		centerArea->Unsplit(consoleText);
		logNull = new wxLogNull();
		wxLog::SetActiveTarget(NULL);
		if (logText != NULL)
			delete logText;
		delete consoleText;
		consoleText = NULL;
	}
}

/*
void GuiFrame::onControl(wxCommandEvent& event)
{
	//Show dialog listing controls:
	ControlDialog* dlg = new ControlDialog(this, getDialogPos(200,400), dialogAtStart);
	//Has checkbox to toggle showing at startup
	dialogAtStart = bool(dlg->ShowModal());
	dlg->Destroy();
	configObject->Write("AppSettings/StartupDialog", dialogAtStart);
}*/

void GuiFrame::onAbout(wxCommandEvent& event)
{
	//Show an About-box:
	AboutDialog* dlg = new AboutDialog(this, getDialogPos(140,160));
	dlg->ShowModal();
	dlg->Destroy();
}

void GuiFrame::onSearchText(wxCommandEvent& event)
{
	if (dataBase != NULL)
		mapList->onSearchText(toolbarMan->getSearchString());
}

/*
void GuiFrame::onSearchCancel(wxCommandEvent& event)
{
    wxLogVerbose("Search cancelled");
}*/

void GuiFrame::onViewSelected(DataFilter* dataView)
{
	int type = (dataView->type < FILTER_MAP)? 0: 1; //Wad or map
	if (mapPanel != NULL) { //Currently showing map
		mapPanel->readChanges();
		if (type == 0) { //Switch to wadPanel
			int sashPos = mainArea->GetSashPosition();
			mainArea->Unsplit(mapPanel);
			delete mapPanel;
			mapPanel = NULL;
			wadPanel = new GuiWadPanel(mainArea);
			wadPanel->setDataManager(dataBase);
			mainArea->SplitVertically(centerArea, wadPanel, sashPos);
		} else {
			mapPanel->clearMap();
		}
	} else if (wadPanel != NULL) { //Currently showing wad
		wadPanel->readChanges();
		if (type == 1) { //Switch to mapPanel
			int sashPos = mainArea->GetSashPosition();
			mainArea->Unsplit(wadPanel);
			delete wadPanel;
			wadPanel = NULL;
			mapPanel = new GuiMapPanel(mainArea);
			mapPanel->setDataManager(dataBase);
			mainArea->SplitVertically(centerArea, mapPanel, sashPos);
		} else {
			wadPanel->clearWad();
		}
	}
	mapList->createList(type);
}

void GuiFrame::onWadActivated(long index, int dialogType)
{
	if (wadPanel!=NULL) wadPanel->readChanges();
	if (mapPanel!=NULL) mapPanel->readChanges();
	DoomDialogMode m = (editMode)? DDMODE_EDIT: DDMODE_VIEW;
	bool combo = false;
	if (dialogType == 0)
		combo = appSettings->getFlag(UI_COMBOWAD);
	else if (dialogType == 2)
		combo = true;

	if (combo) {
		int x = (int) configObject->Read("/ComboEntry/x", 200);
		int y = (int) configObject->Read("/ComboEntry/y", 100);
		int w = (int) configObject->Read("/ComboEntry/w", 1150);
		int h = (int) configObject->Read("/ComboEntry/h", 600);
		ListWrapper<WadEntry*>* wadLst = dataBase->getWadList();
		wadLst->setIndex(index);
		GuiComboDialog* comboDialog = new GuiComboDialog(this, wxPoint(x,y), wxSize(w,h), wadLst, dataBase, m);
		int result = comboDialog->ShowModal();
		comboDialog->savePlacement(configObject);
		comboDialog->Destroy();

	} else {
		int x = (int) configObject->Read("/WadEntry/x", 300);
		int y = (int) configObject->Read("/WadEntry/y", 200);
		int w = (int) configObject->Read("/WadEntry/w", 900);
		int h = (int) configObject->Read("/WadEntry/h", 600);
		ListWrapper<WadEntry*>* wadLst = dataBase->getWadList();
		wadLst->setIndex(index);
		GuiWadEntry* wadDialog = new GuiWadEntry(this, wxPoint(x,y), wxSize(w,h), wadLst, dataBase, m, true);
		int result = wadDialog->ShowModal();
		wadDialog->savePlacement(configObject);
		wadDialog->Destroy();
	}
	if (dataBase->unsavedWadMapChanges()) {
		mapList->refreshList();
		if (wadPanel!=NULL) wadPanel->refresh();
		if (mapPanel!=NULL) mapPanel->refresh();
		dataBase->saveWadsMaps();
	}
}

void GuiFrame::onWadFocused(long index)
{
    if (wadPanel != NULL)
	{
		if (index==-1) {
			wadPanel->clearWad();
		} else {
			wadPanel->readChanges();
			WadEntry* wad = dataBase->getWad(index);
			wadPanel->setWad(wad);
		}
    }
}

void GuiFrame::onMapActivated(long index, int dialogType)
{
	if (wadPanel!=NULL) wadPanel->readChanges();
	if (mapPanel!=NULL) mapPanel->readChanges();
	DoomDialogMode m = (editMode)? DDMODE_EDIT: DDMODE_VIEW;
	bool combo = false;
	if (dialogType == 0)
		combo = appSettings->getFlag(UI_COMBOMAP);
	else if (dialogType == 2)
		combo = true;

	if (combo) {
		int x = (int) configObject->Read("/ComboEntry/x", 200);
		int y = (int) configObject->Read("/ComboEntry/y", 100);
		int w = (int) configObject->Read("/ComboEntry/w", 1400);
		int h = (int) configObject->Read("/ComboEntry/h", 550);
		MapEntry* me = dataBase->getMap(index);
		WadEntry* we = me->wadPointer;
		int mapIndex = we->getMapIndex(me);
		ListWrapper<WadEntry*>* wrapper = new ListWrapper<WadEntry*>();
		wrapper->add(we);
		wrapper->reset();
		GuiComboDialog* comboDialog = new GuiComboDialog(this, wxPoint(x,y), wxSize(w,h), wrapper, dataBase, m, mapIndex);
		int result = comboDialog->ShowModal();
		comboDialog->savePlacement(configObject);
		comboDialog->Destroy();
		delete wrapper;
	} else {
		int x = (int) configObject->Read("/MapEntry/x", 300);
		int y = (int) configObject->Read("/MapEntry/y", 200);
		int w = (int) configObject->Read("/MapEntry/w", 800);
		int h = (int) configObject->Read("/MapEntry/h", 550);
		ListWrapper<MapEntry*>* mapLst = dataBase->getMapList();
		mapLst->setIndex(index);
		GuiMapEntry* mapDialog = new GuiMapEntry(this, wxPoint(x,y), wxSize(w,h), mapLst, dataBase, m, true);
		int result = mapDialog->ShowModal();
		mapDialog->savePlacement(configObject);
		mapDialog->Destroy();
	}
	if (dataBase->unsavedWadMapChanges()) {
		mapList->refreshList();
		if (wadPanel!=NULL) wadPanel->refresh();
		if (mapPanel!=NULL) mapPanel->refresh();
		dataBase->saveWadsMaps();
	}
}

void GuiFrame::onMapFocused(long index)
{
	if (mapPanel != NULL)
	{
		if (index==-1) {
			mapPanel->clearMap();
		} else {
			mapPanel->readChanges();
			MapEntry* me = dataBase->getMap(index);
			mapPanel->setMap(me);
		}
    }
}

/* Unused event handlers
void GuiFrame::onSize(wxSizeEvent& event)
{
	//The frame can have a custom size event handler, called when the size of
	//the frame changes. It can get the needed new size with GetClientSize(),
	//and set the new sizes/positions of members with SetSize(x, y, size.x, size.y).
}

void GuiFrame::onMenuHighlight(wxMenuEvent& event)
{}

void GuiFrame::onToolEnter(wxCommandEvent& event)
{}
*/

void GuiFrame::onClose(wxCloseEvent& event)
{
	//User is trying to close the application
	bool destroy = true;
	//canvas->Show(false);

	//Check if ok to close:
	/*
	if (event.CanVeto())
	{
		//Find out if any documents are unsaved?
		bool unsaved=false;
		...
		if (unsaved)

		wxMessageDialog* dlg = new wxMessageDialog(this, "Are you sure you wish to exit?",
			"Exit wxOpenGL?", wxYES_NO|wxICON_QUESTION); //Can't set position
		int result=dlg->ShowModal();
		dlg->Destroy();
		if (result==wxID_NO) //Don't exit
		{
			event.Veto();
			destroy=false;
		}
	}
	*/

	//Destroy the frame:
	if (destroy)
	{
		if (dataBase != NULL)
			closeDatabase();
		appSettings->saveConfig(configObject);
		//Persist WadStatAspects:
		WadStatAspects* aspects = wadReader->getAspects();
		configObject->SetPath("/Aspects");
		configObject->Write("wadFile", aspects->wadFile);
		configObject->Write("wadFlags", aspects->wadFlags);
		configObject->Write("mapMain", aspects->mapMain);
		configObject->Write("gameModes", aspects->gameModes);
		configObject->Write("mapStats", aspects->mapStats);
		configObject->Write("gameStats", aspects->gameStats);
		configObject->Write("mapImages", aspects->mapImages);
		configObject->SetPath("..");
		//Do cleanup?
		Destroy();
	}
}

void GuiFrame::saveFile(bool saveas)
{
	//Get filename, either from object or from a file dialog:
	//object* obj=handler.getEntry(id).object;
	//wxString oldName=obj->getwxFileName();
	//if (oldName=="") saveas=true; //Must choose name
	if (saveas) //Get filename from dialog:
	{
		wxFileDialog* fdlg=new wxFileDialog(this, "Save document", "", "",
		"App files (*.nam)|*.nam", wxFD_SAVE|wxFD_OVERWRITE_PROMPT|wxFD_CHANGE_DIR);
		if (fdlg->ShowModal()==wxID_OK)
		{
			//obj->setwxFileName(fdlg->GetPath());
			fdlg->Destroy();
		} else {
			fdlg->Destroy();
			return; //Cancel action
		}
	}
	//Save the document:
	/*
	try {
		obj->saveToFile();
	} catch (Error e) {
		//Couldn't save document, show error message:
		wxString errorLine1=wxString(e.text1.c_str());
		wxString errorLine2=wxString(e.text2.c_str());
		wxMessageDialog* dlg=new wxMessageDialog(this,
			"The attempted operation returned an error:\n"+errorLine1+"\n"+errorLine2,
			"Couldn't save file!",wxOK | wxICON_ERROR); //Can't set dialog position
		dlg->ShowModal();
		dlg->Destroy();
		if (saveas) obj->setwxFileName(oldName); //Revert to original name
		return;
	} */
	if (saveas) //Update title of frame(s):
	{

	}
	//If saving should cause updating of subframes, this should be done here.
}
