/*!
* \file GuiMapPanel.h
* \author Lars Thomas Boye 2020
*
* Panel displaying select fields for a MapEntry.
*/
#ifndef GUIMAPPANEL_H
#define GUIMAPPANEL_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiMapManager.h"

/*!
* Panel to show a MapEntry. It uses GuiMapManager to create its
* content, but does not include all possible fields. It is laid
* laid out in a vertical column, meant for use along the side of
* the main window. The fields are not editable, except for personal
* fields (rating).
*/
class GuiMapPanel : public wxPanel
{
	public:
		GuiMapPanel(wxWindow* parent);
		virtual ~GuiMapPanel();

		/*!
		* Connect the panel to data, or remove it by setting NULL.
		* Must be set before calling other methods.
		*/
		void setDataManager(DataManager* dm);

		/*!
		* Show this MapEntry in the panel.
		*/
		void setMap(MapEntry* mapEnt);

		/*!
		* Reshow the current MapEntry in the panel.
		*/
		void refresh();

		/*!
		* Remove the current MapEntry, clearing the content.
		*/
		void clearMap();

		/*!
		* Before changing MapEntry or removing the panel, call this to
		* update the currently shown MapEntry based on UI state.
		*/
		void readChanges();

	protected:

	private:
		GuiMapManager* mapManager;

	//DECLARE_EVENT_TABLE()
};

#endif // GUIMAPPANEL_H
