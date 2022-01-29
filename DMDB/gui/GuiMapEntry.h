/*!
* \file GuiMapEntry.h
* \author Lars Thomas Boye 2018
*
* Window for displaying or editing all fields of map entries.
*/
#ifndef GUIMAPENTRY_H
#define GUIMAPENTRY_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/splitter.h"
#include "wx/config.h"
#include "GuiMapManager.h"

/*!
* A dialog based on GuiMapManager, featuring the complete set of GUI items
* for map entries. The dialog is given a list of MapEntries, and has
* previous/next-buttons to navigate the list (if there are multiple entries).
* Fields are editable or not based on the DoomDialogMode (personal fields are
* always editable). It includes text description, map image and screenshot.
* The image panels occupy the right part of the window, with moveable splitter
* between them, and the screenshot panel can be used to import a screenshot.
* Fields for wad, authors and basedOn map have buttons to open (or select)
* these entries. Style tags can be entered and removed.
*/
class GuiMapEntry : public wxDialog
{
	public:
		/*!
		* The GuiMapEntry shows MapEntries from the ListWrapper, starting at the
		* current position in this list. Put a single MapEntry in a ListWrapper to
		* show a single entry. In addition to controlling editability to fields,
		* there are some small adaptations to the controls based on DoomDialogMode.
		* openWads controls whether or not this dialog can open new windows to
		* show wads.
		*/
		GuiMapEntry(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
					ListWrapper<MapEntry*>* ml, DataManager* dm, DoomDialogMode m, bool openWads);

		virtual ~GuiMapEntry();

		/*!
		* Call after closing the dialog, to save position and size.
		*/
		void savePlacement(wxConfigBase* config);

		//Event handlers:
		void onAuthorButton(wxCommandEvent& event); //!< Select author
		void onBasedButton(wxCommandEvent& event); //!< Select/view map this is based on
		void onWadButton(wxCommandEvent& event); //!< Show wad dialog
		void onAddTag(wxCommandEvent& event); //!< Select tag to add to map
		void onRemoveTag(wxCommandEvent& event); //!< Remove currently selected tag

		void onPrevMap(wxCommandEvent& event); //!< Previous map in list
		void onNextMap(wxCommandEvent& event); //!< Next map in list
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		void refreshAll(); //!< Update all parts of dialog with current map

		GuiMapManager* mapManager;
		ListWrapper<MapEntry*>* mapList;

		bool openStuff; //Can open map and wad entries from dialog?
		wxSplitterWindow* imageSplitter;
		wxButton* prevButton;
		wxButton* nextButton;

	DECLARE_EVENT_TABLE()
};

#endif // GUIMAPENTRY_H
