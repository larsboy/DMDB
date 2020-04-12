/*!
* \file GuiMinorDialogs.h
* \author Lars Thomas Boye 2007
*
* A place for simple auxiliary dialogs, such as to show information.
* AboutDialog is an "About" box.
*/

#ifndef GUIMINORDIALOGS_H //Avoid problems with multiple includes
#define GUIMINORDIALOGS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiBase.h"

/*
* ControlDialog is an information frame telling the user how to interact with
* the application. It can be shown each time the application starts, and has
* a checkbox to toggle this behavior.
*
class ControlDialog : public wxDialog
{
	public:
	//! After construction, it is shown with ShowModal(), and can be destroyed
	//! when this returns. It returns the showStartup value.
	ControlDialog(wxWindow* parent, const wxPoint& pos, bool showStart);

	//Event handlers:
	void toggleStartup(wxCommandEvent& event); //!< Checkbox event
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
	int showStartup; //0=False, 1=True

	DECLARE_EVENT_TABLE()
};*/

/*!
* AboutDialog is a simple information frame to be shown when the user selects
* 'About' from the menus. It provides a custom-made alternative to
* wxMessageDialog, to show icons in addition to text and control text layout.
* It handles its own events.
*/
class AboutDialog : public wxDialog
{
	public:
	//! After construction, it is shown with ShowModal(), and can be destroyed
	//! when this returns.
	AboutDialog(wxWindow* parent, const wxPoint& pos);

    //Event handlers:
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onClose(wxCloseEvent& event); //!< Dialog closed

	DECLARE_EVENT_TABLE()
};

#endif
