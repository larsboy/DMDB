/*!
* \file GuiMapCommon.h
* \author Lars Thomas Boye 2020
*
* Dialog to set common properties for all maps in a wad.
*/
#ifndef GUIMAPCOMMON_H
#define GUIMAPCOMMON_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiMapManager.h"

/*!
* The GuiMapCommon dialog is made to edit/specify common values for
* MapEntry properties for a set of maps, such as for all maps in a wad.
* It uses the GuiMapManager for its content, but includes only those
* parts which may be common for a set of maps:
* - References: author1, author2
* - Modes: singlePlayer, cooperative, deathmatch, otherMode
* - Tags
* - Personal: ownRating, played, difficulty, playTime
*
* It works on a single MapEntry object, which is just a container for the
* input and not an actual entry in the database. The values set in the
* relevant fields of this MapEntry can then be applied to all the actual
* MapEntries to be edited.
*/
class GuiMapCommon : public wxDialog
{
	public:
		/*!
		* Created with pointers to a MapEntry to edit and the DataManager.
		*/
		GuiMapCommon(wxWindow* parent, const wxPoint& pos, MapEntry* me, DataManager* dm, bool dir=false);

		virtual ~GuiMapCommon();

		//Event handlers:
		void onAuthorButton(wxCommandEvent& event); //!< Select author
		void onAddTag(wxCommandEvent& event); //!< Select tag to add to map
		void onRemoveTag(wxCommandEvent& event); //!< Remove currently selected tag
		void onOk(wxCommandEvent& event); //!< Ending with ok
		void onCancel(wxCommandEvent& event); //!< Ending with cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		GuiMapManager* mapManager; //Creates and manages GUI

	DECLARE_EVENT_TABLE()
};

#endif // GUIMAPCOMMON_H
