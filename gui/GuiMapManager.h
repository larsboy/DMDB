/*!
* \file GuiMapManager.h
* \author Lars Thomas Boye 2020
*
* GuiMapManager creates and manages the user interface elements for
* a MapEntry, for showing MapEntry contents and allowing them to be
* edited.
*/

#ifndef GUIMAPMANAGER_H
#define GUIMAPMANAGER_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiBase.h"
#include "GuiPersonDialogs.h"
#include "GuiTagDialogs.h"
#include "GuiMapSelect.h"
#include "GuiImagePanel.h"
#include "../data/DataModel.h"
#include "../data/DataManager.h"

/*!
* Defines the possible sections we can include in the map GUI.
*/
enum GuiMapParts {
	GMP_TITLE = 1, //!< Title and name
	GMP_REFS = 2, //!< Wad, author and basedOn references
	GMP_MODES = 4, //!< Game modes
	GMP_TAGS = 8, //!< Style tag list
	GMP_STATS = 16, //!< Map stats
	GMP_OWN = 32, //!< Personal stuff
	GMP_GAMEPLAY = 64, //!< Gameplay stats
	GMP_TEXT = 128 //!< Text
};

/*!
* Flags for which buttons to include in the map GUI.
*/
enum GuiMapButtons {
	GMB_WAD = 1, //!< Go to wad entry for the map
	GMB_AUTHORS = 2, //!< View/edit authors for the map
	GMB_BASEDON = 4 //!< View/edit the basedOn field (other map this map is based on)
};

/*!
* List of map fields which needs string input/output.
*/
enum MapStringFields {
	MAPSTR_NAME,
	MAPSTR_TITLE,
	MAPSTR_BASEDON,
	MAPSTR_WAD,
	MAPSTR_AUTHOR1,
	MAPSTR_AUTHOR2,
	MAPSTR_SINGLE,
	MAPSTR_COOP,
	MAPSTR_DM,
	MAPSTR_MODE,
	MAPSTR_LINEDEFS,
	MAPSTR_SECTORS,
	MAPSTR_THINGS,
	MAPSTR_SECRETS,
	MAPSTR_ENEMIES,
	MAPSTR_TOTALHP,
	MAPSTR_HEALTHRAT,
	MAPSTR_ARMORRAT,
	MAPSTR_AMMORAT,
	MAPSTR_AREA,
	MAPSTR_PLAYTIME,
	MAPSTR_LINEDENS,
	MAPSTR_ENEMYDENS,
	MAPSTR_HPDENS,
	MAPSTR_END
};

/*!
* The GuiMapManager is used to build a user interface, such as dialog or panel,
* for the Map entries of the database. It connects the data with the user
* interface, creating the necessary user interface elements with the create*
* methods and transferring data between the UI elements and the MapEntry with
* writeEntry and readEntry. The MapEntry is set with setMapEntry. The map
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
* section layouts can change in size after writing a new map (setMapEntry +
* writeEntry), so you may need to do a Layout()-call on the containing sizer.
*/
class GuiMapManager
{
	public:
		/*!
		* The GuiMapManager needs a DataManager. Set hideUnused to true to
		* hide unused fields for single-map wads.
		*/
		GuiMapManager(DoomDialogMode m, DataManager* dm=NULL, bool hideUnused=false);

		virtual ~GuiMapManager();

		/*!
		* Change the DataManager used by this GuiMapManager. It can be set to NULL
		* if the previous DataManager is being deleted. Any current MapEntry is
		* cleared.
		*/
		void setDataManager(DataManager* dm);

		/*!
		* Creates the title section of fields - map title and lump name, plus
		* "non-level" flag. Lump name is not editable, and title is only
		* editable if there are multiple maps in the wad, with it otherwise
		* hidden if hideUnused is true. Specify a heading text for this section.
		*/
		wxSizer* createTitle(wxWindow* parent, const wxString& label);

		/*!
		* Creates a section with references to other entries - wad, authors
		* and basedOn map. Each field can have a button or not, controlled
		* with buttonFlags. Button ids are BUTTON_WAD, BUTTON_AUTHOR1,
		* BUTTON_AUTHOR2 and BUTTON_MAP. Events from included buttons should
		* be handled with EVT_BUTTON macros. In read-only mode (DDMODE_VIEW),
		* buttons are disabled when there is no entry, as they are meant to
		* be used to view the referenced entries. The whole wad field is
		* dropped if the button is not included and hideUnused is true.
		*/
		wxSizer* createRefs(wxWindow* parent, char buttonFlags);

		/*!
		* Creates section with game modes. unspecified=true allows selecting
		* an extra value 4 with an empty string as label for single/coop/dm.
		*/
		wxSizer* createModes(wxWindow* parent, bool unspecified=false);

		/*!
		* Creates field for showing/setting style tags. It has a small list
		* box for listing tags, with id LVIEW_LIST. If in an edit mode,
		* there are buttons to add (BUTTON_TAGADD) and delete (BUTTON_TAGDEL)
		* tags. The button events should be handled by the owning window,
		* which can call addTag and removeSelectedTag.
		*/
		wxBoxSizer* createTags(wxWindow* parent);

		/*!
		* Creates section with map stats (not gameplay) - numerical inputs
		* and some check boxes for flags.
		*/
		wxSizer* createStats(wxWindow* parent);

		/*!
		* Creates section with perosnal fields, such as rating and difficulty.
		* Note that these can be edited no matter the DoomDialogMode. Some
		* have an additiona blank choice if unspecified is true.
		*/
		wxSizer* createOwn(wxWindow* parent, bool unspecified=false);

		/*!
		* Creates section with gameplay (skill 3) stats.
		*/
		wxSizer* createGameplay(wxWindow* parent);

		/*!
		* Creates a multi-line text field for the free description.
		*/
		wxTextCtrl* createText(wxWindow* parent);

		/*!
		* Creates image-pane for map image, showing it if the file exists.
		*/
		GuiImagePanel* createMapPane(wxWindow* parent);

		/*!
		* Creates image-pane for screenshot, showing it if the file exists
		* and allowing setting an image/file (no matter the DoomDialogMode).
		*/
		GuiImagePanel* createShotPane(wxWindow* parent);

		/*!
		* Connect a MapEntry to the UI. Call writeEntry to put its data
		* into the UI elements, and additionally writeTagList and showImages
		* if such sections are included.
		*/
		void setMapEntry(MapEntry* me);

		/*!
		* true if the manager currently has a MapEntry.
		*/
		bool hasEntry() { return mapEntry != NULL; }

		/*!
		* Populates the tag list with the style tags of the current map entry.
		*/
		void writeTagList();

		/*!
		* Write data from the current MapEntry into the UI elements. This should
		* be followed by a call to the owning window's TransferDataToWindow.
		*/
		void writeEntry();

		/*!
		* Read data from the UI elements into the current MapEntry. Before calling
		* this, call TransferDataFromWindow on the owning window, to make sure data
		* is ready.
		*/
		void readEntry();

		/*!
		* Loads images in the map and shot panes, if included. They are given file
		* paths corresponding to the current map.
		*/
		void showImages();

		/*!
		* Removes the current MapEntry, removing all data from the UI.
		*/
		void clearEntry();

		/*!
		* Open the wad of the current map. This opens a modal dialog, with
		* the same DoomDialogMode as the MapManager (so it may allow the wad
		* entry to be edited).
		*/
		void openWad(wxWindow* parent);

		/*!
		* View an author entry (first or second). Opens a dialog showing the
		* person or group (view only).
		*/
		void showAuthor(wxWindow* parent, bool second);

		/*!
		* Edit one of the author entries of the current map. Opens a dialog
		* for author selection. The map entry can be modified by this, so it
		* reads from GUI first and writes back to GUI afterwards.
		*/
		void editAuthor(wxWindow* parent, bool second);

		/*!
		* If the current map had a basedOn reference to another map, this is
		* opened in a modal map dialog. Useful for BUTTON_MAP in VIEW mode.
		*/
		void showBasedOn(wxWindow* parent);

		/*!
		* Select a map for the basedOn entry of the current map. This opens a
		* dialog listing wads and maps, modifying the current map if a selection
		* is made. Useful for BUTTON_MAP in edit mode.
		*/
		void editBasedOn(wxWindow* parent);

		/*!
		* Allow the user to add a style tag to the current map. This opens a
		* dialog for selecting a tag. Useful for BUTTON_TAGADD (shown in edit
		* modes).
		*/
		void addTag(wxWindow* parent);

		/*!
		* Removes the currently selected style tag from the map. Useful for
		* BUTTON_TAGDEL (shown in edit modes).
		*/
		void removeSelectedTag();

		/*!
		* The DoomDialogMode the GuiMapManager was constructed with.
		*/
		DoomDialogMode mode;

	protected:

	private:
		/*! Creates a string representation of 'base' as a base for 'me'. */
		string basedOnString(MapEntry* me, MapEntry* base);

		DataManager* dataBase;
		bool hide; //Hide unused fields?
		unsigned char parts; //Which parts have been created
		wxSize sizeMed;
		wxSize sizeSmall;
		bool canEdit; //Based on mode
		long textStyle; //Based on mode

		MapEntry* mapEntry; //Current MapEntry
		WadText* wadText;

		wxString mapStrFields[MAPSTR_END];
		wxFlexGridSizer* titleSizer; //To hide title for single-map wad
		wxStaticText* titleLabel; //Can be hidden
		wxTextCtrl* titleInput;
		wxTextCtrl* wadTextCtrl; //NULL if wad row not included
		wxTextCtrl* playtimeInput;
		wxCheckBox* mapFlagBoxes[7]; //MapFlags
		wxChoice* mapChoice[7];
		wxListBox* listBox;
		wxString noteInput;
		GuiImagePanel* mapPane;
		GuiImagePanel* shotPane;

		wxButton* auth1Button;
		wxButton* auth2Button;
		wxButton* basedButton;
		wxStaticText* basedLabel;
};

#endif // GUIMAPMANAGER_H
