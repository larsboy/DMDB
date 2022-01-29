/*!
* \file GuiMapSelect.h
* \author Lars Thomas Boye 2020
*
* Dialog for selecting a map, listing all wads and the maps of the
* selected wad. GuiWadList is the wad list control, and GuiMapSelect
* is the dialog.
*/

#ifndef GUIMAPSELECT_H
#define GUIMAPSELECT_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/listctrl.h>

#include "GuiBase.h"
#include "../data/DataManager.h"

/*!
* List control for selecting wad. It is a wxListCtrl in virtual mode,
* providing items from a list of WadEntries.
*/
class GuiWadList : public wxListCtrl
{
	public:
		GuiWadList(wxWindow* parent, const wxSize& siz);
		virtual ~GuiWadList();

		/*!
		* Set the list of wads to show. List control content is
		* regenerated.
		*/
		void setWadList(ListWrapper<WadEntry*>* wads);

		/*!
		* Mark one wad as selected, also scrolling to make sure this item
		* is visible. Returns false if the wad is not found in the current
		* list.
		*/
		bool setSelected(WadEntry* wad);

		/*!
		* Scroll the list to the first wad with a title starting with prefix.
		*/
		void jumpToWad(wxString prefix);

		/*!
		* Returns the wad at this index in the list.
		*/
		WadEntry* getWad(long index);

	protected:
		//Implementation for virtual list, provoding the cells when needed
		virtual wxString OnGetItemText(long item, long column) const;

	private:
		ListWrapper<WadEntry*>* wadList;
};

/*!
* Dialog for selecting a map. It lists all wads, sorted alphabetically on titles,
* and has a search field for filtering the titles. A second list shows the maps
* in whatever wad is selected. An initial selection can be specified in the
* constructor. The dialog must be shown with ShowModal(), and this returns BUTTON_OK
* if a map was selected AND it is different from the initial selection, otherwise
* it returns BUTTON_CANCEL. Get the selected map with getSelectedMap(). The dialog
* can be used to select a basedOn entry for a map, in which case this map can't be
* selected (a map can't be basedOn itself).
*/
class GuiMapSelect : public wxDialog
{
	public:
		/*!
		* GuiMapSelect needs a DataManager. mapEnt can be null, or used to specify
		* an initial selection (basedOn=false). If basedOn is true, the dialog is
		* used to select a basedOn entry for the given mapEnt. mapEnt may already
		* have a basedOn, which is then the initial selection in the dialog, and
		* mapEnt can't be selected (a map can't be basedOn itself).
		*/
		GuiMapSelect(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, MapEntry* mapEnt, bool basedOn);
		virtual ~GuiMapSelect();

		/*!
		* After showing the dialog, use this to get the selected map. NULL if none.
		*/
		MapEntry* getSelectedMap();

		//Event handlers:
		void onResize(wxSizeEvent& event); //!< Resize lists width on dialog resize
		void wadFocused(wxListEvent& event); //!< Change list index in wad list
		void mapActivated(wxCommandEvent& event); //!< Map item enter/double-click
		void onSearchText(wxCommandEvent& event); //!< Search text change
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onNone(wxCommandEvent& event); //!< Select none
		void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
		int getMapIndex(uint32_t dbid); //!< index of map in currentMap, or -1
		void createWadList(wxString sstr, bool update=false); //!< Create wad list based on search string
		void showMaps(); //!< Populate map list based on currentWad
		void illegalMap(); //!< Dialog on illegal selection
		void selectionDone(int retCode); //!< Closes dialog after making selection

		DataManager* dataBase;
		ListWrapper<WadEntry*>* wadList;
		GuiWadList* wadListCtrl;
		wxListBox* mapListCtrl;
		wxSearchCtrl* searchCtrl;
		wxCheckBox* sameAs;

		uint32_t startMap; //Initial selection, 0 for none
		MapEntry* forBasedOn; //If used to select map forBasedOn->basedOn
		WadEntry* currentWad; //Currently selected wad, or NULL
		int mapIndex; //List index of currently selected map entry in currentWad
		int searchChars;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUIMAPSELECT_H
