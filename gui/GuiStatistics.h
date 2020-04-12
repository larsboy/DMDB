/*!
* \file GuiStatistics.h
* \author Lars Thomas Boye 2018
*
* Defines GUI dialogs for showing statistics. StatisticsReport
* shows the contents of a MapStatistics object as a textual
* report. GuiStatsList is a list control for listing the
* MapStatistics of a StatisticSet. The GuiStatistics dialog
* houses such a list control. StatisticsReport dialog is shown
* when selecting a row in the list.
*/

#ifndef GUISTATISTICS_H
#define GUISTATISTICS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/listctrl.h>
#include "GuiBase.h"
#include "GuiMapReport.h"
#include "../data/StatisticSet.h"

/*!
* Dialog to show the contents of a MapStatistics object -
* aggregated statistics from processing a set of maps. It
* inherits from GuiReport, which implements the TextReport
* interface.
*/
class StatisticsReport : public GuiReport
{
	public:
	/*! Created with a MapStatistics object to show. */
	StatisticsReport(wxWindow* parent, const wxPoint& pos, MapStatistics* mapSt);
	
	virtual ~StatisticsReport() {}
	
	//Event handlers:
	virtual void onOk(wxCommandEvent& event); //!< Ending with ok
	virtual void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
		
	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* List control showing the table specified by a StatisticSet.
* The StatisticSet has a set of MapStatistics objects, with
* each of these shown as a row in the table. It also specifies
* which fields to use as columns and the column widths.
* Clicking a column header sorts the list on this column, and
* clicking a row opens a StatisticsReport dialog to show all
* the details of the selected MapStatistics.
*/
class GuiStatsList : public wxListCtrl
{
	public:
		/*! Created with a StatisticSet object to show. */
		GuiStatsList(wxWindow* parent, StatisticSet* stats);
		
		virtual ~GuiStatsList();
		
		/*! Refreshes the list. Only needed if the StatisticSet changes. */
		void updateList();
		
		void itemActivated(wxListEvent& event); //!< Row clicked
		void colClicked(wxListEvent& event); //!< Column header clicked
		
	protected:
		/*! Implementation of wxListCtrl. */
		virtual wxString OnGetItemText(long item, long column) const;
		
	private:
		StatisticSet* statSet;
		list<MapStatistics*>* statList; //MapStatistics from StatisticSet
		mutable list<MapStatistics*>::iterator iter; //Iterator in list
		mutable long iterIndex; //List index of iterator
		
	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Dialog with a GuiStatsList, to show a StatisticSet in
* a table.
*/
class GuiStatistics : public wxDialog
{
	public:
		/*! Created with a StatisticSet object to show. */
		GuiStatistics(wxWindow* parent, const wxPoint& pos, StatisticSet* stats);
		
		virtual ~GuiStatistics();
		
		//Event handlers:
		virtual void onOk(wxCommandEvent& event); //!< Ending with ok
		virtual void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUISTATISTICS_H
