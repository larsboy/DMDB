/*!
* \file GuiWadPanel.h
* \author Lars Thomas Boye 2020
*
* Panel displaying select fields for a WadEntry.
*/
#ifndef GUIWADPANEL_H
#define GUIWADPANEL_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiWadManager.h"

/*!
* Panel to show a WadEntry. It uses GuiWadManager to create its
* content, but does not include all possible fields. It is laid
* laid out in a vertical column, meant for use along the side of
* the main window. It includes a list of maps, to open a map
* dialog. The fields are not editable, except for personal fields
* (rating).
*/
class GuiWadPanel : public wxPanel
{
	public:
		GuiWadPanel(wxWindow* parent);
		virtual ~GuiWadPanel();

		/*!
		* Connect the panel to data, or remove it by setting NULL.
		* Must be set before calling other methods.
		*/
		void setDataManager(DataManager* dm);

		/*!
		* Show this WadEntry in the panel.
		*/
		void setWad(WadEntry* wad);

		/*!
		* Reshow the current WadEntry in the panel.
		*/
		void refresh();

		/*!
		* Remove the current WadEntry, clearing the content.
		*/
		void clearWad();

		/*!
		* Before changing WadEntry or removing the panel, call this to
		* update the currently shown WadEntry based on UI state.
		*/
		void readChanges();

		/*! Selected map in map list. */
		void onMapSelected(wxCommandEvent& event);

	protected:

	private:
		GuiWadManager* wadManager;

	DECLARE_EVENT_TABLE()
};

#endif // GUIWADPANEL_H
