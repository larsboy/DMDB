/*!
* \file GuiWadManager.h
* \author Lars Thomas Boye 2020
*
* GuiWadManager creates and manages the user interface elements for
* a WadEntry, for showing WadEntry contents and allowing them to be
* edited.
*/

#ifndef GUIWADMANAGER_H
#define GUIWADMANAGER_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiBase.h"
#include "GuiMapEntry.h"
#include "../data/DataModel.h"
#include "../data/DataManager.h"

/*!
* Defines the possible sections we can include in the wad GUI.
*/
enum GuiWadParts {
	GWP_MAIN = 1, //!< Title, engine, etc.
	GWP_FILE = 2, //!< File details
	GWP_OWN= 4, //!< Personal info
	GWP_CONTENT = 8, //!< Content flags
	GWP_NOTES = 16, //!< Notes & links
	GWP_TEXT = 32, //!< Description
	GWP_MAPS = 64 //!< Map list
};

/*!
* List of wad fields which needs string input/output.
*/
enum WadStringFields {
	WADSTR_FILENAME,
	WADSTR_FILESIZE,
	WADSTR_MD5,
	WADSTR_XFILES,
	WADSTR_IDGAMES,
	WADSTR_TITLE,
	WADSTR_YEAR,
	WADSTR_IWAD,
	WADSTR_ENGINE,
	WADSTR_PLAYSTYLE,
	WADSTR_RATING,
	WADSTR_PLAYTIME,
	WADSTR_RESOURCES,
	WADSTR_PORTNOTES,
	WADSTR_LINKS,
	WADSTR_END
};

/*!
* The GuiWadManager is used to build a user interface, such as dialog or panel,
* for the Wad entries of the database. It connects the data with the user
* interface, creating the necessary user interface elements with the create*
* methods and transferring data between the UI elements and the WadEntry with
* writeEntry and readEntry. The WadEntry is set with setWadEntry. The wad
* elements are grouped in a set of sections which can be created and placed
* individually. Most sections are organized in two columns, with labels in the
* first and the data in the second. The DoomDialogMode controls whether fields
* are editable and other details.
*
* For single-map wads, the wad and map gui managers implement the following rules:
* - map.title = wad.title, and only editable on the wad.
* - wad.rating = map.rating, and only editable on the map.
* - wad.ownRating = map.ownRating, and only editable on the map.
* - For textual description, there is only the wad description. It can be edited
*   on both wad and map.
* The fields which are not used for single-map wads can be hidden by setting
* hideUnused when constructing the manager. Note that if hideUnused is true,
* section layouts can change in size after writing a new wad (setWadEntry +
* writeEntry), so you may need to do a Layout()-call on the containing sizer.
*/
class GuiWadManager
{
	public:
		/*!
		* The GuiWadManager needs a DataManager. Set hideUnused to true to
		* hide unused fields for single-map wads.
		*/
		GuiWadManager(DoomDialogMode m, DataManager* dm = NULL, bool hideUnused=false);

		virtual ~GuiWadManager();

		/*!
		* Change the DataManager used by this GuiWadManager. It can be set to NULL
		* if the previous DataManager is being deleted. Any current WadEntry is
		* cleared.
		*/
		void setDataManager(DataManager* dm);

		/*!
		* Creates the "main" section of fields, which includes key fields such
		* as title, iwad, engine and year. Specify a heading text for this
		* section.
		*/
		wxSizer* createMain(wxWindow* parent, const wxString& label);

		/*!
		* Creates the two sections "file" (info about the wad and associated files)
		* and "own" (personal info, such as own rating). These are created together
		* to be aligned in a common layout, but you select which to include.
		* Personal fields can be edited no matter what the DoomDialogMode, but rating
		* can only be set on wad if it has multiple maps.
		*/
		wxSizer* createFileOwn(wxWindow* parent, bool file=true, bool own=true);

		/*!
		* Creates the "content" section, which are all flags for what content is
		* included in the wad, rendered with check boxes.
		*/
		wxSizer* createContent(wxWindow* parent);

		/*!
		* Creates a section with the free text fields not part of the core wad
		* entry, except for the description (resources, port notes, web links).
		*/
		wxSizer* createNotes(wxWindow* parent);

		/*!
		* Creates a multi-line text field for the free description.
		*/
		wxTextCtrl* createText(wxWindow* parent);

		/*!
		* Creates a list of maps in the wad. Depending on the mode, the user can
		* interact with this list in two ways. The list itself has id LMAP_LIST,
		* and has events, such as for double-click to select a map. In edit modes,
		* a delete button is added, to delete the currently selected map. It has
		* id BUTTON_DELETE. If editCommon is true, a button is added to edit common
		* values for all maps in the wad, with id BUTTON_EDIT (only enabled for
		* multi-map wads). To handle events for the map list, add event handlers
		* to the event table macro of the owning window, such as:
		* EVT_LISTBOX(LMAP_LIST, <function>)
		* EVT_LISTBOX_DCLICK(LMAP_LIST, <function>)
		* EVT_BUTTON(BUTTON_DELETE, <function>)
		* EVT_BUTTON(BUTTON_EDIT, <function>)
		* The GuiWadManager has methods useful for handling these events.
		*/
		wxBoxSizer* createMaps(wxWindow* parent, bool editCommon=false, bool deleteMaps=false);

		/*!
		* Connect a WadEntry to the UI. Call writeEntry to put its data
		* into the UI elements.
		*/
		void setWadEntry(WadEntry* we);

		/*!
		* true if the manager currently has a WadEntry.
		*/
		bool hasEntry() { return wadEntry != NULL; }

		/*!
		* Write data from the current WadEntry into the UI elements. This should
		* be followed by a call to the owning window's TransferDataToWindow.
		*/
		void writeEntry();

		/*!
		* Read data from the UI elements into the current WadEntry. Before calling
		* this, call TransferDataFromWindow on the owning window, to make sure data
		* is ready.
		*/
		void readEntry();

		/*!
		* Removes the current WadEntry, removing all data from the UI.
		*/
		void clearEntry();

		/*!
		* Shows the map dialog for the maps in the current wad. This is a modal
		* dialog, and the user can scroll through the maps in the wad (if there are
		* more than one), starting at mapIndex.
		*/
		void modalMapDialog(wxWindow* parent, int mapIndex=0);

		/*!
		* Shows a dialog to edit common properties for all maps in the wad, allowing
		* to set common values for all maps at once.
		*/
		void editMapCommon(wxWindow* parent);

		/*!
		* Delete a map from the wad. The selected map in the map list is the one
		* to delete. Needs a window pointer to show dialogs. Available in DDMODE_NEW
		* (removed the map from the WadEntry in memory, which is not yet persisted)
		* and in DDMODE_EDIT (deletes the map permanently from the database, after
		* asking the user to acknowledge). There must be more than one map in the
		* WadEntry to delete one. Returns true if a map was deleted.
		*/
		bool deleteSelectedMap(wxWindow* parent);

		/*!
		* Set the selected item in the map list.
		*/
		void setSelectedMap(int index);

		/*!
		* The DoomDialogMode the GuiWadManager was constructed with.
		*/
		const DoomDialogMode mode;

	private:
		DataManager* dataBase;
		bool hide; //Hide unused fields?
		unsigned char parts; //Which parts have been created
		wxSize sizeLong;
		wxSize sizeMed;
		bool canEdit; //Based on mode
		long textStyle; //Based on mode

		WadEntry* wadEntry; //Current WadEntry
		WadText* wadText;

		wxString wadStrFields[WADSTR_END];
		wxCheckBox* wadFlagBoxes[12]; //WadFlags + OF_HAVEFILE
		wxChoice* wadChoice[4];
		wxListBox* listBox;
		wxButton* commonButton;
		wxButton* deleteButton;
		wxString noteInput;
		wxStaticText* orLabel; //Label for ownRating, to hide for single-map wads
		wxFlexGridSizer* fileOwnSizer; //To hide rating for single-map wads
};

#endif // GUIWADMANAGER_H
