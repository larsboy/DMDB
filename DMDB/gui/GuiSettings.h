/*!
* \file GuiSettings.h
* \author Lars Thomas Boye 2019
*
* All settings for the application which we may want to make available through
* the UI are kept in a single object of type ApplicationSettings. To automate
* the managing of a possibly large amount of settings, settings of common types
* (boolean, integer and strings) are managed by indexing arrays of these types.
* The settings also includes columns for wad and map lists, with the width of
* each column (0 to not show). The EntryListFields struct is used to exchande
* these settings with the list code. The settings object can save its values
* using a wxConfigBase-object.
*
* ApplicationSettings is closely linked with the dialog used to set the values,
* represented by the class GuiSettingsDialog.
*/

#ifndef GUISETTINGS_H //Avoid problems with multiple includes
#define GUISETTINGS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/config.h"
#include "wx/notebook.h"
#include "wx/clrpicker.h"

#include "GuiBase.h"
#include "../data/DataModel.h"


/*! An enum provides the indices for all boolean flags. */
enum boolSetElems {
	UI_SIDEPANEL,		//!< Show side panel
	UI_LOGPANEL,        //!< Show log panel
	UI_COMBOWAD,		//!< Use combined wad+map dialog for wad entries
	UI_COMBOMAP,		//!< Use combined wad+map dialog for map entries
	DB_EDITMODE,		//!< Can edit core DB entries
	ENDOF_BOOLSETELEMS
};

/*! An enum for all integer settings. */
enum intSetElems {
	DB_AUTHORNAME,		//!< Naming preference - AuthorNaming enum (0,1,2)
	UI_FILETYPE,        //!< File type index in read wad file selection
	UI_WADSORT,			//!< Sort column for wad lists
	UI_MAPSORT,			//!< Sort column for map lists
	IMG_WIDTH,			//!< Screenshot width
	IMG_HEIGHT,			//!< Screenshot height
	ENDOF_INTSETELEMS
};

/*! Enum for file path settings. */
enum pathSetElems {
    TDG_DOOM,           //!< ThigDef files?
    TDG_ZDOOM,
    TDG_SKULLT,
    TDG_HERET,
    ENDOF_PATHSETELEMS
};

/*!
* Map columns are indexed with the WadMapFields enum, which starts
* with wad fields. We include some of the wad fields as map columns
* and skip the rest, using this offset to skip to MAP_* entries.
*/
const int MAPWIDTHS_MAPOFFSET = 18;

const int ENDOF_MAPWIDTHS = MAP_END - MAPWIDTHS_MAPOFFSET; //!< Map columns (45)

const int SETTINGPAGES = 4; //!< Number of notebook pages in dialog


/*!
* Defines the columns of a map or wad list, with the fields to show
* and the width of each. Fields are specified with their WadMapFields
* enum values. Column order is included, but not persisted by settings.
*/
struct EntryListFields
{
	WadMapFields fields[30]; //!< Fields to show, in this order
	uint16_t width[30]; //!< Column width of each field
	unsigned char fieldCount; //!< Number of fields
	wxArrayInt order; //!< For column order
	bool modified; //!< true after fields/width changed

	/*! Initializes an empty list of fields. Add with addField. */
	EntryListFields();

	/*! Adds one field at the back of the list. */
	void addField(WadMapFields field, uint16_t w);

	/*! Returns array index for this field, or -1. */
	int findIndex(WadMapFields field);

	/*! Sets modified=true if different from old value. */
	void setWidth(int index, uint16_t w);

	/*! true if both objects have the same fields with the same widths. */
	bool isSame(EntryListFields* other);
};

/*!
* ApplicationSettings manages the settings. Boolean flags, integer values and
* strings are managed by sets of arrays, with one for the values themselves and
* others for attributes for each setting. All attributes are set in the
* constructor, so this is where all such settings are defined.
*/
class ApplicationSettings
{
	/*! The dialog needs full access to this class. */
	friend class GuiSettingsDialog;

	public:
	/*!
	* The constructor is where all boolean and floating-point settings are
	* defined. All settings are given default values.
	*/
	ApplicationSettings();

	/*! Resetting all values, using defaults. */
	void setDefaults();

	void debug();

	/*! Loading values using wxConfigBase. */
	void loadConfig(wxConfigBase* conf);

	/*! Saving values using wxConfigBase. */
	void saveConfig(wxConfigBase* conf);

	//Access to members:

	/*! Get boolean values. */
	bool getFlag(boolSetElems index) const { return flags[index]; }

	/*! Set boolean values. */
	void setFlag(boolSetElems index, bool val) { flags[index]=val; }

	/*! Get integer values. */
	int getValue(intSetElems index) const { return values[index]; }

	/*! Get integer values. Checks if val is within limits. */
	void setValue(intSetElems index, int val);

	/*! Get path strings. */
	wxString getPath(pathSetElems index) const { return paths[index]; }

	/*!
	* Get path string. Default paths are relative to application
	* directory, so if the requested path has its default value,
	* it is appended to this. In any case, the full path is returned.
	*/
	wxString getFullPath(pathSetElems index);

	/*! Set path strings. */
	void setPath(pathSetElems index, wxString val) { paths[index]=val; }

	/*! true if wad column width is more than zero. */
	bool hasWadWidth(int index) { return wadWidths[index]>0; }

	/*!
	* Set wad list column on/off by setting width value. 0 width if show=false,
	* else old/default non-zero width.
	*/
	void setWadWidth(int index, bool show);

	/*! Get wad list column properties from current settings, to set up the list. */
	EntryListFields* getWadFields();

	/*! Set wad list column widths from the EntryListFields object. */
	void setWadFields(EntryListFields* elf);

	/*! true if map column width is more than zero. */
	int hasMapWidth(int index) { return mapWidths[index]>0; }

	/*!
	* Set map list column on/off by setting width value. 0 width if show=false,
	* else old/default non-zero width.
	*/
	void setMapWidth(int index, bool show);

	/*! Get map list column properties from current settings, to set up the list. */
	EntryListFields* getMapFields();

	/*! Set map list column widths from the EntryListFields object. */
	void setMapFields(EntryListFields* elf);

	//wxColour getColour(colourSetElems index) const { return colours[index]; } //!< Get colour members

	//void setColour(colourSetElems index, wxColour col) { colours[index]=col; } //!< Set colour members

	// For colours, the RGB values can be returned as floats in the range 0.0 to 1.0.
	// The rgb index is 0 for red, 1 for green and 2 for blue.
	//float getColourFloat(colourSetElems index, int rgb);

	/*!
	* The integer values can be set from strings. If the process fails, a
	* GuiError will be thrown with information on what was wrong.
	*/
	void setValueFromString(intSetElems index, wxString s); //throws GuiError

	private:
	wxString appDir; //Application location (starting working dir)
	//Boolean settings:
	bool flags[ENDOF_BOOLSETELEMS]; //The settings themselves
	bool defaultFlags[ENDOF_BOOLSETELEMS]; //Defaults to use as start values
	wxString flagRegKeys[ENDOF_BOOLSETELEMS]; //Name used for storing with wxConfigBase
	wxString flagDescriptions[ENDOF_BOOLSETELEMS]; //Text to show in dialogs
	//Numerical settings:
	int values[ENDOF_INTSETELEMS]; //The settings themselves
	int defaultValues[ENDOF_INTSETELEMS]; //Defaults to use as start values
	int minValues[ENDOF_INTSETELEMS]; //Lower limit
	int maxValues[ENDOF_INTSETELEMS]; //Upper limit
	wxString intRegKeys[ENDOF_INTSETELEMS]; //Name used for storing with wxConfigBase
	wxString intDescriptions[ENDOF_INTSETELEMS]; //Text to show in dialogs
	//Path settings:
	wxString paths[ENDOF_PATHSETELEMS]; //The settings themselves
	wxString defaultPaths[ENDOF_PATHSETELEMS]; //Defaults to use as start values
	wxString pathRegKeys[ENDOF_PATHSETELEMS]; //Name used for storing with wxConfigBase
	wxString pathDescriptions[ENDOF_PATHSETELEMS]; //Text to show in dialogs
	//Wad list fields - based on enum WadMapFields:
	int wadWidths[WAD_END_MAP_START];
	int wadWidthDefaults[WAD_END_MAP_START];
	wxString wadWidthKeys[WAD_END_MAP_START]; //Name used for storing with wxConfigBase
	//Descriptions from wadMapLabels
	//Map list fields - based on enum WadMapFields:
	int mapWidths[ENDOF_MAPWIDTHS];
	int mapWidthDefaults[ENDOF_MAPWIDTHS];
	wxString mapWidthKeys[ENDOF_MAPWIDTHS]; //Name used for storing with wxConfigBase
	//MAP_DBID first, then WAD_FILENAME(1) to WAD_PLAYSTYLE(10), then MAP_NAME(28) to MAP_PLAYTIME(59)
	//Descriptions from wadMapLabels
};

//! Names of level schemes (AuthorNaming)
const wxString authorNamingSchemes[] = {
	"Real (First Last)",
	"Real (Last, First)",
	"Alias"
};

/*!
* GuiSettingsDialog is the dialog used for changing the values of the members of
* the ApplicationSettings object. It is constructed with a pointer to this
* object, and uses the definitions in its private arrays to make labels for the
* input fields. The dialog should be shown with ShowModal(). This will return 1
* if the user chose Ok, and 0 if the user cancelled. The ApplicationSettings
* object might have changed. If 1 was returned, the object should be saved. If 0
* was returned, the user didn't want to make changes, but this class does not
* guarantee that the settings are unchanged, so they should be reloaded (in case
* it's possible to make invalid changes, get an error message after other
* settings were changed, and then cancel).
* Validators are used on input fields for numeric values to limit input to
* numbers.
*/
class GuiSettingsDialog : public wxDialog
{
	public:
	/*!
	* The constructor needs a pointer to an ApplicationSettings-object. The current
	* values will be used as start values, and changes will be made to this object.
	*/
	GuiSettingsDialog(wxWindow* parent, const wxPoint& pos, ApplicationSettings* appSet);

	/*!
	* setValues is called if the values in input fields must be reset from the
	* settings object.
	*/
	void setValues();

	//Event handlers:
	void onPathChange(wxCommandEvent& event); //!< Change a path setting
	void onDefault(wxCommandEvent& event); //!< "Default" button event
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onCancel(wxCommandEvent& event); //!< Dialog cancelled
	void onClose(wxCloseEvent& event); //!< Dialog cancelled

	private:
	wxNotebook* notebook; //Input controls are managed by a notebook
	wxPanel* notebookPages[SETTINGPAGES]; //Panels with input controls
	wxCheckBox* flagBoxes[ENDOF_BOOLSETELEMS]; //Check-boxes
	//wxString intInputs[ENDOF_INTSETELEMS]; //The text in the numerical input fields
	wxChoice* intChoice; //Dropdown list
	wxString pathFields[ENDOF_PATHSETELEMS]; //Output of each path
	wxCheckBox* wadBoxes[WAD_END_MAP_START]; //Check-boxes for wad list columns
	wxCheckBox* mapBoxes[ENDOF_MAPWIDTHS]; //Check-boxes for map list columns
	//wxColourPickerCtrl* colourPickers[ENDOF_COLOURSETELEMS]; //Colour picker controls

	ApplicationSettings* appSettings; //The object with the values to change

	DECLARE_EVENT_TABLE()
};

#endif
