/*!
* \file GuiFrame.h
* \author Lars Thomas Boye 2019
*
* GuiFrame is the main application window of the program. It is a wxFrame
* - a basic application window - containing panels, menu system, tool bar
* and status bar.
*/

#ifndef GUIFRAME_H //Avoid problems with multiple includes
#define GUIFRAME_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/config.h"
#include "wx/log.h"
#include "wx/splitter.h" //To split main area in several panels

#include "../data/WadReader.h"
#include "../data/DataManager.h"
#include "GuiViewSelect.h"
#include "GuiEntryList.h"
#include "GuiSettings.h" //Includes GuiBase.h
#include "GuiThingDef.h"
#include "GuiWadEntry.h"
#include "GuiComboDialog.h"
#include "GuiMapCommon.h"
#include "GuiWadPanel.h"
#include "GuiMapPanel.h"


/*!
* GuiFrame is the main window of the application. It contains the menu system and
* toolbar, and implements event handlers for all their items. It contains the main
* entry list, and view select, detail and log panels. It shows the selected wad or
* map dialog. And it has a status bar, updating it based on the DataStatusListener
* interface. It manages the configuration options. It orchestrates the processing
* of wads and the database itself, with the WadReader and DataManager objects.
* Most of the implementation is made up of event handler methods.
*/
class GuiFrame : public wxFrame, public GuiViewListener, public GuiEntryListener,
public GuiParent, public DataStatusListener
{
	public:
    /*! The constructor creates menu/toolbars and other components of the window. */
	GuiFrame(const wxString& title);
	~GuiFrame();

	/*!
	* To be able to open a file on startup through command line arguments, the
	* frame can be given the name of a file to open.
	*/
	//void loadFile(wxString fileName);

	//************** GuiParent functions **************

	//These are called from contained objects to give information to the frame.
	wxPoint getDialogPos(int width = 0, int height = 0); //!< Find position for dialogs

	//************** DataStatusListener **************
	void onWadFilter(const wxString& name, unsigned int wadCount);
	void onMapFilter(const wxString& name, unsigned int mapCount);
	void onTotalCounts(long wads, long maps, long auths);

	//************** Event handlers **************

	void toggleEditMode(wxCommandEvent& event); //!< Change mode (read-only/edit)
	void onDatabase(wxCommandEvent& event); //!< Select database folder
	void onFileNew(wxCommandEvent& event); //!< Create new database
	void onFileRead(wxCommandEvent& event); //!< Read and process wad file
    void onFolderRead(wxCommandEvent& event); //!< Read and process all wads in a folder
	//void onFileSave(wxCommandEvent& event); //!< Save file
	void onPeople(wxCommandEvent& event); //!< Show people dialog
	void onTags(wxCommandEvent& event); //!< Show tag dialog
	void onClose(wxCommandEvent& event); //!< Close database
	void onExit(wxCommandEvent& event); //!< Exit menu item

	void onStatsTotal(wxCommandEvent& event); //!< Show total statistics
	void onStatsYears(wxCommandEvent& event); //!< Show statistics pr year
	void onStatsIwad(wxCommandEvent& event); //!< Show statistics pr iwad
	void onStatsEngine(wxCommandEvent& event); //!< Show statistics pr engine
	void onStatsRating(wxCommandEvent& event); //!< Show statistics pr rating
	void onStatsDifficulty(wxCommandEvent& event); //!< Show statistics pr difficulty
	void onStatsPlaystyle(wxCommandEvent& event); //!< Show statistics pr playstyle
	void onStatsPeople(wxCommandEvent& event); //!< Show statistics pr designer
	void onStatsTags(wxCommandEvent& event); //!< Show statistics pr tag
	void onWadStatsTotal(wxCommandEvent& event); //!< Show total wad statistics
	void onWadStatsYears(wxCommandEvent& event); //!< Show wad statistics pr year
	void onWadStatsIwad(wxCommandEvent& event); //!< Show wad statistics pr iwad
	void onWadStatsEngine(wxCommandEvent& event); //!< Show wad statistics pr engine
	void onWadStatsRating(wxCommandEvent& event); //!< Show wad statistics pr rating

	void onNewThing(wxCommandEvent& event); //!< Create thingDefs from decorate files
	void onLoadThing(wxCommandEvent& event); //!< Show/edit existing thingDef file

	void onTest(wxCommandEvent& event); //!< Debug
	void onNothing(wxCommandEvent& event); //!< Placeholder
	void onAppSettings(wxCommandEvent& event); //!< Dialog to set application settings
	void onAbout(wxCommandEvent& event); //!< Show about dialog

	void onSearchText(wxCommandEvent& event); //!< Search text changed
	//void onSearch(wxCommandEvent& event); //!< Search input
	//void onSearchCancel(wxCommandEvent& event); //!< Search input

	/*!
	* From GuiViewListener - sets the selected DataFilter on the entry list.
	*/
	virtual void onViewSelected(DataFilter* dataView);

	//************** GuiEntryListener **************

	virtual void onWadActivated(long index, int dialogType=0); //!< Wad clicked - open dialog
	virtual void onWadFocused(long index); //!< Wad selected - show in side panel
	virtual void onMapActivated(long index, int dialogType=0); //!< Map clicked - open dialog
	virtual void onMapFocused(long index); //!< Map selected - show in side panel

	//void onSize(wxSizeEvent& event);
	//void onMenuHighlight(wxMenuEvent& event);
	//void onToolEnter(wxCommandEvent& event);

	void onClose(wxCloseEvent& event); //!< User is trying to close the application

	private:
	//************** Private functions **************

	void selectDatabase(); //Select database folder
	void setDatabase(const wxString& folder); //Open database folder
	void closeDatabase(); //Delete current database object

	void deleteRemovedMaps(WadEntry* wadEntry); //Delete maps flagged for deletion after updating wadEntry
	void thingDialog(ThingDefList* things); //Show dialog with list of thingDefs
	void saveFile(bool saveas = false); //Save document to file
	void updateSplitters(); //Update main window composition from settings

	//************** Data members **************
	//Data objects:
	wxConfigBase* configObject; //Saves settings to registry
	ApplicationSettings* appSettings; //Settings
	GuiMenubarManager* menuBar; //Manages menubar
	GuiToolbarManager* toolbarMan; //Manages toolbar
	wxLogTextCtrl* logText; //For logging to log panel (consoleText)
	wxLogNull* logNull; //To suppress logging

	WadReader* wadReader; //Load and process wads
	DataManager* dataBase; //The current database

	wxSplitterWindow* clientArea; //Manages the main frame split with side panels
	wxSplitterWindow* mainArea; //Main part of clientArea, with right panel and centerArea
	wxSplitterWindow* centerArea; //Center part of window, with mapList and optional consoleText
	GuiWadPanel* wadPanel;
	GuiMapPanel* mapPanel;
	GuiViewSelect* viewSelect;
	GuiEntryList* mapList;
	wxTextCtrl* consoleText; //For log output

	//State:
	bool editMode; //Can edit core entries?

	//The macro for the event table
    DECLARE_EVENT_TABLE()
};

#endif
