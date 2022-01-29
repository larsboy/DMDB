/*!
* \file GuiViewSelect.h
* \author Lars Thomas Boye 2019
*
* UI control for selecting a data view - a filter or list specifying
* a set of wad or map entries. The selection is represented with a
* DataFilter, which can be a wad or map filter with selection criteria
* or a wad or map list naming specific entries. The views are
* organized in a tree. Entries can be added to lists with drag-and-drop.
*/

#ifndef GUIVIEWSELECT_H
#define GUIVIEWSELECT_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/treectrl.h>
#include <wx/dnd.h>
#include <wx/datetime.h>
#include "GuiBase.h"
#include "../data/DataModel.h"
#include "../data/DataManager.h"

/*!
* Interface with events from the view selection, implemented by the
* GuiViewSelect client.
*/
class GuiViewListener
{
	public:
	    /*! Called when a DataFilter is selected. */
		virtual void onViewSelected(DataFilter* dataView) = 0;
};

/*!
* The UI control for selecting a data view, subclassing wxTreeCtrl.
* A data view is of type DataFilter and is used to specify a subset
* of wad or map entries. There are two main types. One is based on
* conditions, selecting entries matching the conditions. The other
* is an explicit list of entries. List views are stored as part of
* the database, and retrieved from the DataManager. New lists can be
* created here with the right-click menu, and lists can be renamed.
* All the condition filters are created and kept by the GuiViewSelect.
* A number of filters for various properties are created for both wads
* and maps, as well as special ones like "all" and "new". When a view
* is selected in the tree, the corresponding DataFilter is set in the
* DataManager, and the listener is called to notify of the update.
*/
class GuiViewSelect : public wxTreeCtrl
{
	public:
	    /*! Constructed with reference the listener to call when views are selected. */
		GuiViewSelect(wxWindow* parent, GuiViewListener* l);

		virtual ~GuiViewSelect();

		/*!
		* Initiates the control, connecting it to a database and creating the tree
		* structure of filters and lists. "All wads" and "All maps" are set as initial
		* filters on the database, with the wad and map sorting fields specified.
		*/
		void populate(DataManager* dataMan, WadMapFields wadSort, WadMapFields mapSort);

		/*!
		* Remove the connection to a database. Call this if the DataManager is being
		* deleted. All content will be deleted from the UI.
		*/
		void removeDataManager();

		void onSelection(wxTreeEvent& event); //!< Item selection
		void onMenu(wxTreeEvent& event); //!< Right-click for menu on item
		void onBeginEdit(wxTreeEvent& event); //!< Start edit list name
		void onEndEdit(wxTreeEvent& event); //!< End edit list name
		bool onDragDropped(wxCoord x, wxCoord y, const wxString &data); //!< Add entry to list
	protected:

	private:
		/*! Handles the details of adding one DataFilter to the TreeCtrl. */
		void addFilter(const wxTreeItemId& parent, DataFilter* dv, wxString name);

		/*! Creates a group of IwadFilters, for either wads or maps. */
		void iwadFilters(const wxTreeItemId& root, unsigned char type);

		/*! Creates a group of EngineFilters, for either wads or maps. */
		void engineFilters(const wxTreeItemId& root, unsigned char type);

		/*! Creates a group of PlayStyleFilters, for either wads or maps. */
		void playStyleFilters(const wxTreeItemId& root, unsigned char type);

		/*! Creates a group of YearFilters, for either wads or maps. */
		void yearFilters(const wxTreeItemId& root, unsigned char type);

		/*! Creates a group of WadFlagsFilters, for either wads or maps. */
		void contentFilters(const wxTreeItemId& root, unsigned char type);

		/*! Creates a group of filters for game modes - only for maps. */
		void gameModeFilters(const wxTreeItemId& root);

		/*! Creates a group of filters for personal fields. */
		void ownStuffFilters(const wxTreeItemId& root, unsigned char type);

		/*! Filters for various wad properties. */
		void varWadFilters(const wxTreeItemId& root);

		/*! Filters for various map properties. */
		void varMapFilters(const wxTreeItemId& root);

		DataManager* dataBase;
		GuiViewListener* listener;
		map<wxTreeItemId, DataFilter*>* dataViews; //Maps UI tree items to DataFilters
		wxTreeItemId defaultView;
		wxTreeItemId mapAuthor; //Tree item for author filter
		wxTreeItemId mapTag; //Tree item for tag filter
		wxTreeItemId wadLists; //Tree item holding wad lists
		wxTreeItemId mapLists; //Tree item holding map lists

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Class for receiving drag-and-dropped text for the GuiViewSelect, to
* add entries to a list. The string must be either "WAD:<dbid>" or "MAP:<dbid>",
* where <dbid> is the numerical ID of the wad or map entry. The string is
* processed by GuiViewSelect::onDragDropped.
*/
class GuiViewDropTarget : public wxTextDropTarget
{
	public:
	    /*! Created by the owning GuiViewSelect, with reference to it. */
		GuiViewDropTarget(GuiViewSelect* owner);
		virtual ~GuiViewDropTarget();
		virtual bool OnDropText(wxCoord x, wxCoord y, const wxString &data);

	private:
		GuiViewSelect* owner; //!< Event is passed here
};

#endif // GUIVIEWSELECT_H
