/*!
* \file GuiThingDef.h
* \author Lars Thomas Boye 2018
*
* GUI dialog for listing and editing ThingDef objects. Each ThingDef
* represents one type of "thing" found in Doom-engine maps, with a
* number of attributes used to analyze maps. The dialog lists these
* in a table, and allows editing attributes and adding and deleting
* items.
*/

#ifndef GUITHINGDEF_H
#define GUITHINGDEF_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>
#include <wx/listctrl.h>
#include "GuiBase.h"
#include "../data/DecorateParser.h"


/*!
* Defines the columns of the table, based on the ThingDef class.
*/
enum thingAttrs {
	THA_ID,
	THA_DEH,
	THA_NAME,
	THA_CAT,
	THA_HP,
	THA_SPAWN,
	THA_HEALTH,
	THA_AMMO,
	THA_ARMOR,
	ENDOF_THA
};

/*!
* Labels for the attributes defined by thingAttrs.
*/
const wxString thingAttrStr[] = {
	"ID",
	"Dehacked",
	"Name",
	"Category",
	"Hit points",
	"Spawner",
	"Health",
	"Ammo",
	"Armor"
};

/*!
* GuiThingDialog is a wxDialog meant to be shown in modal mode. It
* is created with a ThingDefList housing the ThingDef objects. It
* has a list control displaying a table view of the ThingDefs. It
* has controls for adding and deleting items, and items can be
* clicked to open the GuiThingDef dialog to edit the item.
* Modified items are highlighted in the list. The dialog is closed
* with either an OK or CANCEL event. The ThingDefList will in any
* case reflect the changes done in the dialog.
*/
class GuiThingDialog : public wxDialog
{
	public:
		/*!
		* Creating the dialog with a ThingDefList housing the ThingDef
		* objects (can be empty). Changes done in the dialog will be
		* reflected in the ThingDefList. If onlyDone is true there is
		* only a "Done" button to close the dialog, otherwise "Save"
		* and "Cancel".
		*/
		GuiThingDialog(wxWindow* parent, const wxPoint& pos, ThingDefList* things, bool onlyDone=false);

		virtual ~GuiThingDialog();

		//Event handlers:
		void itemActivated(wxListEvent& event); //!< Clicked in list
		void itemFocused(wxListEvent& event); //!< Selected in list
		void colClicked(wxListEvent& event); //!< Sorting on column

		/*! Creating a ThingDef object based on values in input fields. */
		void addThing(wxCommandEvent& event);

		/*! Delete the selected item. */
		void removeThing(wxCommandEvent& event);

		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		/*! Creates a column of the table with specific width. */
		void createColumn(thingAttrs ta, int w);

		/*! Creates list contents from thingList. */
		void showList();

		/*! Find ThingDef based on its id property. */
		ThingDef* findThing(uint16_t id);

		/*! Find ThingDef based on index in list. */
		ThingDef* thingFromIndex(int index);

		/*! Find index in the list of ThingDef. */
		int indexFromThing(ThingDef* tf);

		ThingDefList* masterList;
		list<ThingDef*>* thingList; //Exported from masterList to show
		int thingIndex; //Index of selected item, -1 for none
		ThingDef* currentThing; //Selected in list
		bool (*thingComp)(const ThingDef*, const ThingDef*);
		wxListCtrl* listCtrl;
		wxButton* delButton;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

class GuiThingDef : public wxDialog
{
	public:
		GuiThingDef(wxWindow* parent, const wxPoint& pos, ThingDef* td);
		virtual ~GuiThingDef();

		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
		/*! Update input fields with values from currentThing. */
		void writeThing();

		/*! Update ThingDef with values from input fields. */
		void readThing();

		ThingDef* currentThing;

		wxChoice* catChoice;
		wxCheckBox* spawnerCheck;
		wxString attrInput[ENDOF_THA];

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUITHINGDEF_H
