/*!
* \file GuiPersonDialogs.h
* \author Lars Thomas Boye 2019
*
* Dialogs dealing with author entries. GuiPersonList lists all entries,
* both people and groups. It can be used to select an entry and/or allow
* creating and editing entries. GuiPersonEdit is used to create/edit an
* entry for a single person. GuiGroupEdit is used to create/edit a group
* entry, and operates together with GuiPersonList which is used to select
* entries for the list.
*/

#ifndef GUIPERSONDIALOGS_H //Avoid problems with multiple includes
#define GUIPERSONDIALOGS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/listctrl.h>

#include "GuiBase.h"
#include "../data/DataManager.h"

const int PERSONLABELS = 4; //!< Number of text fields in GuiPersonList dialog

class GuiGroupEdit;

/*!
* Dialog listing all person entries in the database. A search field is included,
* to filter on a search string. Properties are shown for the currently selected
* entry. The dialog can allow editing or not, based on the DoomDialogMode.
* DDMODE_EDIT or DDMODE_NEW allows creating new person and groupd entries and
* editing existing entries, using the GuiPersonEdit and GuiGroupEdit dialogs.
* The dialog can be used to select a person entry. Create with dbid > -1 and show
* with ShowModal. This returns BUTTON_OK or BUTTON_CANCEL, and if OK use
* getSelectedPerson() to get the entry.
*/
class GuiPersonList : public wxDialog
{
	public:
	/*!
	* If dbid is more than the default -1, the dialog is used to select
	* a person. If 0, there is no initial selection. If larger, the entry
	* with this dbid will start selected.
	*/
	GuiPersonList(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, DoomDialogMode m, int dbid=-1);
	virtual ~GuiPersonList();

	/*! Populate entry-specific fields with data from currently selected entry. */
	void showPerson();

	/*!
	* Populates the list control with the person names. Returns the index of
	* the last item, -1 if the list is empty.
	*/
	long refreshList();

	/*! Get the currently selected entry in the list. */
	AuthorEntry* getSelectedPerson();

	//Event handlers:
	void onResize(wxSizeEvent& event); //!< Resize list width on dialog resize
	void itemActivated(wxListEvent& event); //!< Item enter/double-click
	void itemFocused(wxListEvent& event); //!< Change author index
	void rightClick(wxListEvent& event); //!< Right-click menu with extra options
	void onSearchText(wxCommandEvent& event); //!< Search text change
	void onNew(wxCommandEvent& event); //!< Button to create new entry
	void onNewGroup(wxCommandEvent& event); //!< Button to create new group entry
	void groupEditDone(int changed); //!< GuiGroupEdit dialog finished
	void onEdit(wxCommandEvent& event); //!< Button to edit entry
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onNone(wxCommandEvent& event); //!< Select none (selection mode)
	void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
	void selectPerson(long index); //!< Set index in list, highlighting an entry
	void closeGroupDialog(); //!< Removes GuiGroupEdit dialog, re-enabling buttons
	void editDialog(); //!< Show GuiPersonEdit or GuiGroupEdit to edit entry
	void onMerge(); //!< Merge the currently selected author entry with another
	void onDelete(); //!< Delete the currently selected author entry

	DataManager* dataBase;
	wxListCtrl* listCtrl;
	wxSearchCtrl* searchCtrl;
	DoomDialogMode mode;
	bool selectMode; //true to return selection
	long authorIndex; //List index of currently selected entry
	int searchChars; //Length of current search string
	wxStaticText* personLabels[PERSONLABELS]; //Text output for selected entry
	wxTextCtrl* personDescription;

	wxButton* buttons[5];
	GuiGroupEdit* groupDialog; //Dialog shown when non-NULL

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Dialog for creating/editing an AuthorEntry representing a single person.
* It has fields for the names and text notes. Either a name or an alias
* must be specified to make a valid entry. Show with ShowModal, and it returns
* a result: ENTRY_CORE and/or ENTRY_TEXT for changes, or 0.
*/
class GuiPersonEdit : public wxDialog
{
	public:
	/*!
	* Created with pointers to an AuthorEntry object and a wxString for notes.
	* ShowModal's return value indicates what has been modified. Can also be
	* in read-only mode (show=true).
	*/
	GuiPersonEdit(wxWindow* parent, const wxPoint& pos, AuthorEntry* person, wxString* notes, bool show=false);
	~GuiPersonEdit() {}

	//Event handlers:
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onCancel(wxCommandEvent& event); //!< Ending with cancel
	void onClose(wxCloseEvent& event); //!< Dialog closed (cancel)

	private:
	AuthorEntry* author;
	wxString* authorNotes;
	wxString inputs[4];
	wxString noteInput;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Dialog for creating/editing an AuthorGroupEntry. It should be shown from
* GuiPersonList, which is used to select entries to add to the group. It
* has fields for name and notes, and a list showing the current entries.
* Entries can be removed, and added from the GuiPersonList.
*/
class GuiGroupEdit : public wxDialog
{
	public:
	/*!
	* Created with pointers to an AuthorGroupEntry object and a wxString for
	* notes. Also needs pointers to the DataManager, and the owning
	* GuiPersonList to be able to add list entries.
	*/
	GuiGroupEdit(wxWindow* parent, const wxPoint& pos, AuthorGroupEntry* group,
				wxString notes, DataManager* dataMan, GuiPersonList* ownr = NULL);
	~GuiGroupEdit() {}

	/*! Add entry to group. */
	void addPerson(AuthorEntry* person, wxString name);

	/*! Returns the AuthorGroupEntry. */
	AuthorGroupEntry* getAuthorGroup() { return authorGroup; }

	/*! Return the notes string. */
	wxString getAuthorNotes() { return authorNotes; }

	//Event handlers:
	void onEntryRemove(wxCommandEvent& event); //!< Clicked list item to remove
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onCancel(wxCommandEvent& event); //!< Ending with cancel
	void onClose(wxCloseEvent& event); //!< Dialog closed (cancel)

	private:
	GuiPersonList* owner;
	AuthorGroupEntry* authorGroup;
	wxString authorNotes;
	wxString nameInput;
	wxString noteInput;
	wxListBox* listBox;
	bool listChange;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif
