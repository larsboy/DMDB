/*!
* \file GuiMapReport.h
* \author Lars Thomas Boye 2018
*
* Defines a dialog for showing the textual report of a map.
* The TextReport interface is used to write reports, and the
* GuiReport class implements this with a dialog, to be used
* as sub-class for such dialogs. GuiMapReport sub-classes it
* for reporting the MapStats class.
*/

#ifndef GUIMAPREPORT_H
#define GUIMAPREPORT_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/richtext/richtextctrl.h>
#include "GuiBase.h"
#include "../TextReport.h"
#include "../data/MapStats.h"

/*!
* GUI dialog implementing the TextReport interface, writing
* formatted text to a rich text control. Serves as a base class
* for dialogs with text report.
*/
class GuiReport : public wxDialog, public TextReport
{
	public:
	/*! wxDialog constructor arguments. */
	GuiReport(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size);
	
	virtual ~GuiReport() {}
	
	//TextReport implementation:
	virtual void writeHeading(wxString text);
	virtual void writeSubHeading(wxString text);
	virtual void writeText(wxString text);
	virtual void writeLine(wxString text);
	virtual void setTabs(const wxArrayInt &tabs);
	
	protected:
	wxRichTextCtrl* reportView;
	bool paragraph;
};

/*!
* The GuiMapReport is a simple report dialog, to output the
* report of a MapStats object. It only consists of the text
* report and a button to close itself.
*/
class GuiMapReport : public GuiReport
{
	public:
	GuiMapReport(wxWindow* parent, const wxPoint& pos, MapStats* mapSt);
	virtual ~GuiMapReport() {}
	
	//Event handlers:
	virtual void onOk(wxCommandEvent& event); //!< Ending with ok
	virtual void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
		
	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUIMAPREPORT_H
