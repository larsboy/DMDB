/*
* GuiBase implementation
* The wxString functions simply use LtbUtils functions for the conversion of
* numbers to strings. GuiMenubarManager defines the menus of the main window.
* GuiToolbarManager defines the toolbar of the main window.
*/

#include "GuiBase.h"
#include "../LtbUtils.h" //For string conversion functions

//Toolbar icons:
#include "../bitmaps/add.XPM"
#include "../bitmaps/diradd.XPM"
#include "../bitmaps/closed.XPM"
#include "../bitmaps/cog.XPM"
#include "../bitmaps/open.XPM"
#include "../bitmaps/people.XPM"
#include "../bitmaps/tags.XPM"

//Macros for custom event types?
//DEFINE_EVENT_TYPE(wxEVT_MODELUPDT)

int SSHOT_WIDTH;

int SSHOT_HEIGHT;

//************************ wxString functions ************************

wxString wxIntToString(int i)
{
	string str=LtbUtils::intToString(i);
	return wxString(str.c_str());
}

wxString wxDoubleToString(double d, int decimals)
{
	string str=LtbUtils::doubleToString(d,decimals);
	return wxString(str.c_str());
}

//************************ GuiMenubarManager ************************

GuiMenubarManager::GuiMenubarManager()
{
	currentMenubar=0;
}

wxMenuBar* GuiMenubarManager::createMenubar(wxFrame* parent)
{
	//Creating menubar:
	//Menus can be accessed with Alt+letter if the letter is marked by '&' in the
    //menu string: "&File"
    //Menu items can also have Alt+letter access by including '&' in the string.
	//In addition, menu itmes can be assigned a direct shortcut like Ctrl+letter
	//by adding it at the end.
	//Ex: "&Copy\tCtrl+C"
	//A help text can be included, to be shown in the statusbar:
	//Append(ID, menustring, helpstring)

	wxMenuBar* menuBar=new wxMenuBar();

	//Database menu:
    wxMenu* fileMenu=new wxMenu();
    fileMenu->Append(LFILE_OPEN, "Open database...\tCtrl+O");
    //fileMenu->Append(LFILE_NEW, "New database\tCtrl+N");
    fileMenu->Append(LFILE_READ, "Read wad...\tCtrl+R");
    fileMenu->Append(LFILE_READALL, "Read folder...");
    fileMenu->AppendCheckItem(LFILE_MODE, "Edit mode");
    fileMenu->AppendSeparator();
    fileMenu->Append(LDATA_PEOPLE, "People");
	fileMenu->Append(LDATA_TAGS, "Tags");
	fileMenu->AppendSeparator();
	fileMenu->Append(LFILE_CLOSE, "Close database");
    fileMenu->Append(LFILE_EXIT, "Exit");

    //Stats menu:
	wxMenu* statsMenu=new wxMenu();
	statsMenu->Append(LSTATS_TOTAL, "Map statistics");
	statsMenu->Append(LSTATS_YEARS, "Statistics by year");
	statsMenu->Append(LSTATS_IWAD, "Statistics by Iwad");
	statsMenu->Append(LSTATS_ENGINE, "Statistics by engine");
	statsMenu->Append(LSTATS_RATING, "Statistics by rating");
	statsMenu->Append(LSTATS_DIFFIC, "Statistics by difficulty");
	statsMenu->Append(LSTATS_PLAYST, "Statistics by playstyle");
	statsMenu->Append(LSTATS_PEOPLE, "Statistics by authors");
	statsMenu->Append(LSTATS_TAGS, "Statistics by tags");
	statsMenu->AppendSeparator();
	statsMenu->Append(LSTATS_WTOTAL, "Wad statistics");
	statsMenu->Append(LSTATS_WYEARS, "Statistics by year");
	statsMenu->Append(LSTATS_WIWAD, "Statistics by Iwad");
	statsMenu->Append(LSTATS_WENGINE, "Statistics by engine");
	statsMenu->Append(LSTATS_WRATING, "Statistics by rating");

    //ThingDef menu:
	wxMenu* toolsMenu=new wxMenu();
	toolsMenu->Append(LTHING_NEW, "Parse Decorate folder...\tF4");
	toolsMenu->Append(LTHING_OPEN, "Open ThingDef...\tF5");
	toolsMenu->AppendSeparator();
	toolsMenu->Append(LTHING_DOOM, "Doom (1/2, Boom)");
	toolsMenu->Append(LTHING_ZDOOM, "Doom 1/2 in ZDoom");
	toolsMenu->Append(LTHING_SKULLT, "Skulltag");
	toolsMenu->Append(LTHING_HERET, "Heretic");

	//Options menu:
	wxMenu* optionMenu=new wxMenu();
	optionMenu->Append(LOPT_APPSETTINGS, "Settings...");
    optionMenu->AppendSeparator();
    optionMenu->Append(LHELP_ABOUT, "About...");

    //Append menus to menuBar:
	menuBar->Append(fileMenu, "&Database");
	menuBar->Append(statsMenu, "&Statistics");
	menuBar->Append(toolsMenu, "&ThingDefs");
	menuBar->Append(optionMenu, "&Options");

    parent->SetMenuBar(menuBar);
    currentMenubar=menuBar;
    return menuBar;
}

void GuiMenubarManager::enableItems(bool hasDB)
{
	currentMenubar->Enable(LFILE_READ, hasDB);
	currentMenubar->Enable(LFILE_READALL, hasDB);
	currentMenubar->Enable(LFILE_CLOSE, hasDB);
	currentMenubar->Enable(LDATA_PEOPLE, hasDB);
	currentMenubar->Enable(LDATA_TAGS, hasDB);
	currentMenubar->Enable(LSTATS_TOTAL, hasDB);
	currentMenubar->Enable(LSTATS_YEARS, hasDB);
	currentMenubar->Enable(LSTATS_IWAD, hasDB);
	currentMenubar->Enable(LSTATS_ENGINE, hasDB);
	currentMenubar->Enable(LSTATS_RATING, hasDB);
	currentMenubar->Enable(LSTATS_DIFFIC, hasDB);
	currentMenubar->Enable(LSTATS_PLAYST, hasDB);
	currentMenubar->Enable(LSTATS_PEOPLE, hasDB);
	currentMenubar->Enable(LSTATS_TAGS, hasDB);
	currentMenubar->Enable(LSTATS_WTOTAL, hasDB);
	currentMenubar->Enable(LSTATS_WYEARS, hasDB);
	currentMenubar->Enable(LSTATS_WIWAD, hasDB);
	currentMenubar->Enable(LSTATS_WENGINE, hasDB);
	currentMenubar->Enable(LSTATS_WRATING, hasDB);
}

void GuiMenubarManager::enableWadItems(bool wadList)
{
	currentMenubar->Enable(LSTATS_WTOTAL, wadList);
	currentMenubar->Enable(LSTATS_WYEARS, wadList);
	currentMenubar->Enable(LSTATS_WIWAD, wadList);
	currentMenubar->Enable(LSTATS_WENGINE, wadList);
	currentMenubar->Enable(LSTATS_WRATING, wadList);
}

void GuiMenubarManager::setItemState(ApplicationMenuEvents toolId, bool newState)
{
    currentMenubar->Check(toolId, newState);
}

bool GuiMenubarManager::getItemState(ApplicationMenuEvents toolId)
{
    return currentMenubar->IsChecked(toolId);
}

//************************ GuiToolbarManager ************************

GuiToolbarManager::GuiToolbarManager()
{
	//The constructor sets up the array of bitmaps:
	bitmapArray[BM_ADD] = wxBitmap(_add);
	bitmapArray[BM_DIRADD] = wxBitmap(_diradd);
	bitmapArray[BM_EDIT] = wxBitmap(_openxx);
	bitmapArray[BM_READ] = wxBitmap(_closed);
	bitmapArray[BM_PEOPLE] = wxBitmap(_people);
	bitmapArray[BM_TAGS] = wxBitmap(_tags);
	bitmapArray[BM_SETTINGS] = wxBitmap(_cog);

	currentToolbar=0;
}

wxToolBar* GuiToolbarManager::createToolbar(wxFrame* parent, bool showText)
{
	//Check if there already is a toolbar?

	//The style fields in the toolbar constructor controls the look of the toolbar.
	long style = wxTB_FLAT|wxTB_HORIZONTAL;
	if (showText) style |= wxTB_TEXT;
	wxToolBar* theToolbar = new wxToolBar(parent, -1, wxDefaultPosition, wxDefaultSize, style);

	//The toolbar gets different colours in different parts on Windows with the default
	//style, so I set a colour manually.
	#if defined(__WXMSW__)
    	theToolbar->SetBackgroundStyle(wxBG_STYLE_COLOUR); //wxBG_STYLE_SYSTEM
		theToolbar->SetBackgroundColour(wxColour(224,223,227));
    #endif

	//Create edit toolbar buttons:
	theToolbar->AddTool(LFILE_READ, "Read wad", bitmapArray[BM_ADD], "Process game content file");
	theToolbar->AddTool(LFILE_READALL, "Read folder", bitmapArray[BM_DIRADD], "Process all game content files in a folder");
	theToolbar->AddCheckTool(LFILE_MODE, "Edit mode", bitmapArray[BM_READ], bitmapArray[BM_READ], "Edit core contents?");
	theToolbar->AddTool(LDATA_PEOPLE, "People", bitmapArray[BM_PEOPLE], "Person entries");
	theToolbar->AddTool(LDATA_TAGS, "Tags", bitmapArray[BM_TAGS], "Tag entries");
	theToolbar->AddSeparator();

	//wxStaticText* label = new wxStaticText(theToolbar, -1, " Search");
	//theToolbar->AddControl(label);
	txtCtrl = new wxSearchCtrl(theToolbar,SEARCH_FIELD,"",wxDefaultPosition,wxDefaultSize);//wxSize(100,20)
	txtCtrl->ShowCancelButton(true);
    theToolbar->AddControl(txtCtrl);

    //Must call TransferDataFromWindow(); to get text into searchRef

    theToolbar->AddSeparator();
    theToolbar->AddTool(LOPT_APPSETTINGS, "Settings", bitmapArray[BM_SETTINGS], "Application settings");

	//Show the toolbar:
	theToolbar->Realize();
	parent->SetToolBar(theToolbar);
	//theToolbar->EnableTool(LSLD_ZOOM, true);

	//Delete old toolbar:
	//if (currentToolbar!=0) delete currentToolbar;

	currentToolbar=theToolbar;
	return theToolbar;
}

void GuiToolbarManager::enableItems(bool hasDB)
{
	currentToolbar->EnableTool(LFILE_READ, hasDB);
	currentToolbar->EnableTool(LFILE_READALL, hasDB);
	currentToolbar->EnableTool(LDATA_PEOPLE, hasDB);
	currentToolbar->EnableTool(LDATA_TAGS, hasDB);
}

void GuiToolbarManager::setToolState(ApplicationMenuEvents toolId, bool newState)
{
    if (toolId == LFILE_MODE) {
        if (newState) currentToolbar->SetToolNormalBitmap(toolId, bitmapArray[BM_EDIT]);
        else currentToolbar->SetToolNormalBitmap(toolId, bitmapArray[BM_READ]);
    }
	currentToolbar->ToggleTool(toolId, newState);
}

bool GuiToolbarManager::getToolState(ApplicationMenuEvents toolId)
{
	return currentToolbar->GetToolState(toolId);
}

wxString GuiToolbarManager::getSearchString()
{
    //currentToolbar->TransferDataFromWindow();
    return txtCtrl->GetValue();
}
