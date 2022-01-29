/*!
* \file GuiWadEntry.h
* \author Lars Thomas Boye 2018
*
* Window for displaying or editing all fields of wad entries.
*/
#ifndef GUIWADENTRY_H
#define GUIWADENTRY_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/config.h"
#include "GuiWadManager.h"

/*!
* A dialog based on GuiWadManager, featuring the complete set of GUI items
* for wad entries. The dialog is given a list of WadEntries, and has
* previous/next buttons to navigate the list (if there are multiple entries).
* Fields are editable or not based on the DoomDialogMode (personal fields are
* always editable). It includes a list of maps, and may open a map in its own
* dialog (GuiMapEntry with the list of maps in this wad).
*/
class GuiWadEntry : public wxDialog
{
	public:
		/*!
		* The GuiWadEntry shows WadEntries from the ListWrapper, starting at the
		* current position in this list. Put a single WadEntry in a ListWrapper to
		* show a single entry. In addition to controlling editability to fields,
		* there are some small adaptations to the controls based on DoomDialogMode.
		* openMaps controls whether or not this dialog can open new windows to
		* show maps.
		*/
		GuiWadEntry(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
				ListWrapper<WadEntry*>* wl, DataManager* dm, DoomDialogMode m, bool openMaps);

		virtual ~GuiWadEntry();

		/*!
		* Call after closing the dialog, to save position and size.
		*/
		void savePlacement(wxConfigBase* config);

		//Event handlers:
		void onMapSelected(wxCommandEvent& event); //!< Selected map in map list
		void onRemoveMap(wxCommandEvent& event); //!< Delete a map
		void onEditMaps(wxCommandEvent& event); //!< Edit common map fields

		void onPrevWad(wxCommandEvent& event); //!< Previous wad in list
		void onNextWad(wxCommandEvent& event); //!< Next wad in list
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		void showWad(); //!< Update dialog with current wad in list

		GuiWadManager* wadManager;
		ListWrapper<WadEntry*>* wadList;

		bool openMap; //Can open map entries from list?
		wxButton* commonButton;
		wxButton* prevButton;
		wxButton* nextButton;

	DECLARE_EVENT_TABLE()
};

#endif // GUIWADENTRY_H
