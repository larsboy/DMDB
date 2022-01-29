/*!
* \file GuiWadEntry.h
* \author Lars Thomas Boye 2020
*
* Window for displaying or editing a wad entry and its map entries.
*/
#ifndef GUICOMBODIALOG_H
#define GUICOMBODIALOG_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/config.h"
#include "GuiWadManager.h"
#include "GuiMapManager.h"

/*!
* A big all-in-one dialog for showing wads with maps. It basically combines
* GuiWadEntry and GuiMapEntry into one dialog, using both GuiWadManager and
* GuiMapManager to create and manage its content. It is given a list of
* WadEntries, initially showing the the entry at the current position. The
* wad fields are on the left, and there are previous/next buttons to navigate
* the wad list. Then follows a list of the maps in the wad, to select a map.
* The right side of the dialog shows the fields and images of the current map.
* Fields are editable or not based on the DoomDialogMode (personal fields are
* always editable). A few UI fields are removed for single-map wads, where
* the map property is the same as the wad property.
*/
class GuiComboDialog : public wxDialog
{
	public:
		/*!
		* The GuiComboDialog shows WadEntries from the ListWrapper, starting at
		* the current position in this list. Put a single WadEntry in a ListWrapper
		* to show a single entry. In addition to controlling editability to fields,
		* there are some small adaptations to the controls based on DoomDialogMode.
		* The initial map to show can be specified as an index in the map list of
		* the initial wad.
		*/
		GuiComboDialog(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
				ListWrapper<WadEntry*>* wl, DataManager* dm, DoomDialogMode m, int mapIndex=0);

		virtual ~GuiComboDialog();

		/*!
		* Call after closing the dialog, to save position and size.
		*/
		void savePlacement(wxConfigBase* config);

		//Event handlers:
		void onMapSelected(wxCommandEvent& event); //!< Selected map in map list
		void onRemoveMap(wxCommandEvent& event); //!< Delete a map
		void onEditMaps(wxCommandEvent& event); //!< Edit common map fields
		void onAuthorButton(wxCommandEvent& event); //!< Select author
		void onBasedButton(wxCommandEvent& event); //!< Select/view map this is based on
		void onAddTag(wxCommandEvent& event); //!< Select tag to add to map
		void onRemoveTag(wxCommandEvent& event); //!< Remove currently selected tag

		void onPrevWad(wxCommandEvent& event); //!< Previous wad in list
		void onNextWad(wxCommandEvent& event); //!< Next wad in list
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		void showWad(int mapIndex=0); //!< Update dialog with current wad in list
		void refreshMap(); //!< Show current map in dialog

		GuiWadManager* wadManager; //Handles wad part of GUI
		GuiMapManager* mapManager; //Handles map part of GUI
		ListWrapper<WadEntry*>* wadList;

		wxBoxSizer* dialogSizer; //Top-level sizer
		wxButton* prevButton;
		wxButton* nextButton;
		wxSplitterWindow* imageSplitter;

		wxBoxSizer* mapSizer; //To hide map descr. input
		wxStaticText* noteLabel; //To hide map descr. input
		wxTextCtrl* noteCtrl; //To hide map descr. input

	DECLARE_EVENT_TABLE()
};

#endif // GUICOMBODIALOG_H
