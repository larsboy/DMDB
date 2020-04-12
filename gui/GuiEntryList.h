/*!
* \file GuiEntryList.h
* \author Lars Thomas Boye 2018
*
* The GuiEntryList shows a wad or map list in table view, with
* sortable columns. Columns and their properties are specified with
* an EntryListFields object from ApplicationSettings. The client
* code implements GuiEntryListener to be notified of entry selection.
*/

#ifndef GUIENTRYLIST_H
#define GUIENTRYLIST_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/dnd.h>
#include "GuiBase.h"
#include "GuiSettings.h"
#include "../data/DataModel.h"
#include "../data/DataManager.h"

/*!
* Listener for events from the GuiEntryList (callbacks
* on the selection of entries).
*/
class GuiEntryListener
{
	public:
		/*!
		* Called when a wad is activated (double-clicked).
		* Gives index in sorted list. Optionally specifies
		* a dialog type (0=default, 1=wad, 2=wad+maps).
		*/
		virtual void onWadActivated(long index, int dialogType=0) = 0;

		/*!
		* Called when a wad is focused (marked as selected). Gives
		* index in sorted list, or -1 to clear the current selection.
		*/
		virtual void onWadFocused(long index) = 0;

		/*!
		* Called when a map is activated (double-clicked).
		* Gives index in sorted list. Optionally specifies
		* a dialog type (0=default, 1=map, 2=wad+maps).
		*/
		virtual void onMapActivated(long index, int dialogType=0) = 0;

		/*!
		* Called when a map is focused (marked as selected). Gives
		* index in sorted list, or -1 to clear the current selection.
		*/
		virtual void onMapFocused(long index) = 0;
};

/*!
* The big table of wad or map entries. It is a wxListCtrl, used in virtual mode
* and with report view showing the data in columns. It can show either wads
* (type 0) or maps (type 1), set with createList. The data comes from the
* DataManager, which always has one sorted and filtered list of each type,
* so the data is controlled by whatever filter is in effect there.
*
* Which columns to show for each list type is completely configurable, with
* ApplicationSettings storing which columns to show and their widths.
* Column widths and order can be changed directly in the list interface.
* Currently only widths are persistent.
*/
class GuiEntryList : public wxListCtrl
{
	public:
		/*! Created with a GuiEntryListener, and ApplicationSettings for column configurations. */
		GuiEntryList(wxWindow* parent, GuiEntryListener* l, ApplicationSettings* as);
		virtual ~GuiEntryList();

		/*!
		* Connect the GuiEntryList to a database (needed to show content).
		* Set to NULL to remove connection to database, also removing the
		* current list.
		*/
		void setDataManager(DataManager* dataMan);

		/*! Read column configurations from settings. */
		void loadListFields();

		/*! Write any modified column configurations to settings. */
		void saveListFields();

		/*!
		* Create list of the given type (0 for wad, 1 for map).
		* List content for the given type is retrieved from the DataManager.
		*/
		void createList(int type);

		/*! Returns current list type - 0 for wad, 1 for map. */
		int getCurrentType() { return dataType; }

		/*! Update list content from database (when content may have changed). */
		void refreshList();

		/*!
		* Apply a search string to the list, going to the first matching entry
		* or filtering out all non-matching entries.
		*/
		void onSearchText(wxString sstr);

		void itemActivated(wxListEvent& event); //!< Item activated - calls listener
		void itemFocused(wxListEvent& event); //!< Item focused - calls listener
		void rightClick(wxListEvent& event); //!< Right-click - show item menu
		void colClicked(wxListEvent& event); //!< Clicked header, for sorting
		//void colDragged(wxListEvent& event);
		void beginDrag(wxListEvent& event); //!< Drag'n'drop entry from list

	protected:
		//Implementation for virtual list, provoding the cells when needed
		virtual wxString OnGetItemText(long item, long column) const;
		virtual wxListItemAttr* OnGetItemAttr(long item) const;

	private:
		void createList(); //!< Create list of current dataType
		void readFields(int type); //!< Read current column properties from UI

		ApplicationSettings* settings;
		DataManager* dataBase;
		GuiEntryListener* listener;
		EntryListFields* fields[2];
		int dataType; //Index in fields, -1 for none
		int searchChars; //Length of search string

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUIENTRYLIST_H
