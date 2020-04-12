/*!
* \file GuiTagDialogs.h
* \author Lars Thomas Boye 2019
*
* Dialogs dealing with style tags. GuiTagList lists all tags, to view or select
* tags. GuiTagEdit is for entering/editing the name of a tag, and is activated
* from GuiTagList.
*/

#ifndef GUITAGLIST_H
#define GUITAGLIST_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/listctrl.h>

#include "GuiBase.h"
#include "../data/DataManager.h"

/*!
* Dialog listing all the style tags in the database. A search field is included,
* to filter on a search string. The dialog can allow editing or not, based on the
* DoomDialogMode. DDMODE_EDIT or DDMODE_NEW allows adding new tags and editing
* existing tags, using the GuiTagEdit dialog. The dialog can be used to select a
* tag. Create with dbid > -1 and show with ShowModal. The database ID of the
* selected tag is returned, otherwise 0.
*/
class GuiTagList : public wxDialog
{
	public:
		/*!
		* If dbid is more than the default -1, the dialog is used to select
        * a tag. If 0, there is no initial selection. If larger, the entry
        * with this dbid will start selected.
		*/
		GuiTagList(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, DoomDialogMode m, int dbid=-1);
		virtual ~GuiTagList();

		/*!
		* Populates the list control with the tag names. Returns the index of
		* the last item, -1 if the list is empty.
		*/
		int refreshList();

		//Event handlers:
		void onResize(wxSizeEvent& event); //!< Resize list width on dialog resize
		void itemActivated(wxListEvent& event); //!< Tag enter/double-click
		void itemFocused(wxListEvent& event); //!< Change tag index
		void onSearchText(wxCommandEvent& event); //!< Search text change
		void onNew(wxCommandEvent& event); //!< Button to create new entry
		void onEdit(wxCommandEvent& event); //!< Button to edit entry
		void onMerge(wxCommandEvent& event); //!< Button to merge entry
		void onDelete(wxCommandEvent& event); //!< Button to delete entry
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
		void selectTag(int index); //!< Set index in list, highlighting a tag
		void editDialog(); //!< Show dialog to edit current tag

		DataManager* dataBase;
		wxListCtrl* listCtrl;
		wxSearchCtrl* searchCtrl;
		DoomDialogMode mode;
		bool selectMode; //true to return selection
		int tagIndex; //List index of currently selected entry
		int searchChars; //Length of current search string

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Small dialog to enter/edit tag string. Just an input field and
* OK/Cancel buttons. A note about changing an existing tag is shown
* if the tag is named already. ShowModal returns ENTRY_CORE if the
* tag is modified, 0 otherwise.
*/
class GuiTagEdit : public wxDialog
{
	public:
    /*! Constructed with reference to new or existing TagEntry. */
	GuiTagEdit(wxWindow* parent, const wxPoint& pos, TagEntry* tag);
	~GuiTagEdit() {}

	//Event handlers:
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onCancel(wxCommandEvent& event); //!< Ending with cancel
	void onClose(wxCloseEvent& event); //!< Dialog closed (cancel)

	private:
	TagEntry* tagPtr; //Tag to edit
	wxString input;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUITAGLIST_H
