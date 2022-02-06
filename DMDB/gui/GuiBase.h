/*!
* \file GuiBase.h
* \author Lars Thomas Boye 2007
*
* GuiBase contains some basic definitions, classes and functions used by the wx
* gui classes of the application, located in one place to easily find and change
* them. The menus and toolbars of the main window are defined here.
*
* First of all, there are constants for the application as a whole, like title
* and version strings. And there are functions for converting numbers to
* wxStrings, and an exception struct to be used if the gui classes need to
* throw exceptions.
*
* All event ids for menu/toolbar commands and any other gui events are defined
* here. Each gui class must define an event table and functions for processing
* events, with all events it needs to handle. For frames, this is mainly
* menu/tool-events. A GuiMenubarManager creates and manages the menus of the
* main frame, and a GuiToolbarManager handles the toolbar.
*
* The virtual class GuiParent is an interface implemented by GuiFrame. Needed to
* avoid circular dependencies?
*/

#ifndef GUIBASE_H
#define GUIBASE_H

//The std namespace is put in the global scope to freely use the standard library?
using namespace std;

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/srchctrl.h>

//Use native toolbar, not generic:
#define USE_GENERIC_TBAR 0

//Log name
#define wxLOG_COMPONENT "dmdb"

//Any application-specific declarations needed should be included here...


//Application strings:
const wxString VENDOR_NAME = "LTB"; //!< Maker of application, as seen by OS
const wxString APP_NAME = "Doom Map Database"; //!< Name of application, as seen by OS
const wxString VERSIONSTRING = "Version 1.04"; //!< Current version

//Default parameters for the main application window (pos/size is stored in
//registry, so these values are only used the first time):
const wxString APP_TITLE = "Doom Map Database"; //!< Title of main window
const wxPoint APP_POINT(20,20); //!< Initial position of main window
const wxSize APP_SIZE(640,480); //!< Initial size of main window
const wxSize APP_MINSIZE(580,400); //!< Window size lower limit
//const int PANELSIZE = 180; //!< Width of panel
//Application-specific sizes...

/*! Maximum width of screenshots, resize if larger. */
extern int SSHOT_WIDTH; //= 640;

/*! Maximum height of screenshots, resize if larger. */
extern int SSHOT_HEIGHT; // = 480;

//Some useful functions for converting numbers to wxStrings:
wxString wxIntToString(int i); //!< Converts int to wxString
wxString wxDoubleToString(double d, int decimals=5); //!< Converts floating-point number to wxString

/*!
* An error class to throw as exceptions, with two strings to hold information
* on the error that can be shown in a dialog.
*/
struct GuiError
{
	wxString text1, text2;
	GuiError(wxString s1, wxString s2 = "") { text1=s1; text2=s2; }
};

//Macros for custom event types:
//DECLARE_EVENT_TYPE(wxEVT_MYEVENT, -1)

/*!
* Modes for dialogs, controlling which operations are available.
*/
enum DoomDialogMode {
	DDMODE_VIEW, //!< Read-only mode
	DDMODE_EDIT, //!< Edit mode (can modify)
	DDMODE_NEW //!< Creating new entry, not yet in DB (can modify)
};

/*!
* The following enum contains the constants defining all event ids for menus
* and toolbar buttons, plus some other commands. This is a complete list of
* menu-events, with info on each.
*/
enum ApplicationMenuEvents {
	//Buttons:
	BUTTON_OK,
	BUTTON_CANCEL,
	BUTTON_DEFAULT,
	BUTTON_NEW,
	BUTTON_EDIT,
	BUTTON_UPDATE,
	BUTTON_DELETE,
	BUTTON_MERGE,
	BUTTON_ADDFILE,
	BUTTON_WAD,
	BUTTON_MAP,
	BUTTON_AUTHOR1,
	BUTTON_AUTHOR2,
	BUTTON_GROUP,
	BUTTON_TAGADD,
	BUTTON_TAGDEL,
	BUTTON_PREVMAP,
	BUTTON_NEXTMAP,
	//File menu:
	LFILE_NEW,		//!< Create empty database (not currently used)
	LFILE_OPEN,		//!< Open a database (select folder)
	LFILE_MODE,     //!< Toggle between edit and read-only mode
	LFILE_READ,		//!< Read wad (Doom content file), possibly adding to database
	LFILE_READALL,  //!< Read and add all content in folder
	LFILE_CLOSE,	//!< Close database (not currently used)
	LFILE_EXIT,		//!< Close application
	//Data menu:
	LDATA_PEOPLE,   //!< People dialog
	LDATA_TAGS,     //!< Tag dialog
	LTHING_NEW,		//!< Create new thing defs
	LTHING_OPEN,	//!< Load and edit thing defs
	LTHING_DOOM,	//!< Open Doom thing defs
	LTHING_ZDOOM,	//!< Open ZDoom thing defs
	LTHING_SKULLT,	//!< Open Skulltag thing defs
	LTHING_HERET,	//!< Open Heretic thing defs
	LTHING_DOOM64,	//!< Open Doom64 thing defs
	//Statistics menu:
	LSTATS_TOTAL,	//!< Uncategorized statistics
	LSTATS_YEARS,	//!< Categorized by year
	LSTATS_IWAD,	//!< Categorized by iwad
	LSTATS_ENGINE,	//!< Categorized by engine
	LSTATS_RATING,	//!< Categorized by rating
	LSTATS_DIFFIC,	//!< Categorized by difficulty
	LSTATS_PLAYST,	//!< Categorized by playstyle
	LSTATS_PEOPLE,	//!< Categorized by author
	LSTATS_TAGS,	//!< Categorized by style tags
	LSTATS_WTOTAL,	//!< Uncategorized wad statistics
	LSTATS_WYEARS,
	LSTATS_WIWAD,
	LSTATS_WENGINE,
	LSTATS_WRATING,
	//Options menu:
	LOPT_TEST,		//!< Debug only
	LOPT_APPSETTINGS, //!< Show dialog to set application settings
	LOPT_LOG,       //!< Show log window
	LHELP_ABOUT,	//!< Show about dialog

	//Dialog lists:
	LPERS_LIST,
	LWAD_LIST,
	LMAP_LIST
};

//Some other event ids:
//(values must start above the number of elements in ApplicationMenuEvents)
const int LCHK_DIALOG = 201;	//!< Dialog checkbox
const int FRAME_SPLITTER = 202;	//!< Main frame splitter
const int LVIEW_LIST = 203;	//!< List
const int LVIEW_TREE = 204;	//!< Tree
const int SEARCH_FIELD = 205; //!< Search control


/*!
* GuiMenubarManager handles creating and updating the menu system for the main
* application window. The menus are placed in the calling window, which must
* handle menu events. Event ids are defined in ApplicationMenuEvents. In
* addition to creating the menus, this class handles enabling/disabling items.
* Having all menu management located in one class makes it easy to find and
* change, and if the application needs to have several different menu bars and
* swap them based on settings and modes, this can easily be added to this class
* by having several menubar-creating functions that handle the swap. The
* implementation of this class defines menu bar composition and when each item
* is present/enabled.
*/
class GuiMenubarManager
{
	public:
	GuiMenubarManager();
	~GuiMenubarManager();

	//! This creates the menubar, and places it in the given wxFrame. So far the class
	//! only handles a single menubar, and so the function doesn't check if a menubar
	//! already exists and should only be used once.
	wxMenuBar* createMenubar(wxFrame* parent);

	//! Enables/disables toolbar items based on whether a database is loaded.
	void enableItems(bool hasDB);

	//! Call with true when we have a wad list, false when we have a map list.
	void enableWadItems(bool wadList);

	void setItemState(ApplicationMenuEvents toolId, bool newState); //!< Sets the state of a toggle item
	bool getItemState(ApplicationMenuEvents toolId); //!< Gets the on/off state of a toggle item

	private:
	wxMenuBar* currentMenubar;
};

/*!
* GuiToolbarManager handles creating and updating the toolbar for the main
* application window. The toolbar is placed in the calling window, which must
* handle toolbar events. Event ids are defined in ApplicationMenuEvents. In
* addition to creating the toolbar, this class handles enabling/disabling items
* and setting their states. Having all toolbar management located in one class
* makes it easy to find and change, and if the application needs to have several
* different toolbars and swap them based on settings and modes, this can easily
* be added to this class by having several toolbar-creating functions that
* handle the swap. The implementation of this class defines toolbar composition
* and when each item is present/enabled.
*/
class GuiToolbarManager
{
	public:
	GuiToolbarManager();
	~GuiToolbarManager() {}

	//! This creates the toolbar. The toolbar is placed and showed in the parent frame,
	//! the only other thing the calling code needs to do is give toolbar items the
	//! wanted states with other functions in this class. So far the class only handles
	//! a single toolbar, and so the function doesn't check if a toolbar already exists
	//! and should only be used once. The buttons can have text, or only icons.
	wxToolBar* createToolbar(wxFrame* parent, bool showText=true);

	//! Enables/disables toolbar items based on whether a database is loaded.
	void enableItems(bool hasDB);

	//The rest of the functions access specific items:
	void setToolState(ApplicationMenuEvents toolId, bool newState); //!< Sets the state of a toggle tool
	bool getToolState(ApplicationMenuEvents toolId); //!< Gets the on/off state of a toggle tool
	wxString getSearchString(); //!< Get the search input

	private:
	//Bitmaps are kept in an array:
	enum ToolbarBitmaps { BM_ADD, BM_DIRADD, BM_EDIT, BM_READ,
		BM_PEOPLE, BM_TAGS, BM_SETTINGS, BM_MAX };
	wxBitmap bitmapArray[BM_MAX];

	//The class keeps a pointer to the current toolbar:
	wxToolBar* currentToolbar;
	wxSearchCtrl* txtCtrl;
};

/*!
* GuiParent is a virtual class implemented by GuiFrame, and used as an interface
* to that frame by child windows.
*/
class GuiParent
{
    public:
	//! getDialogPos() finds a good point to use as position for a dialog-frame,
	//! so it is centered on the application window. Dialog dimensions can be
	//! passed, or defualt dimensions are used.
	virtual wxPoint getDialogPos(int width=0, int height=0) = 0;

	//! Send a string to frame.
	//virtual void giveInfo(wxString infoString) = 0;
};

#endif // GUIBASE_H
