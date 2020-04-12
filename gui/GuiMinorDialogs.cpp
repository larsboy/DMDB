/*
* GuiMinorDialogs implementation
*/

#include "GuiMinorDialogs.h"

//Include icon for About box
#include "../bitmaps/dmdb64.XPM"


//************************ ControlDialog ************************

/*
ControlDialog::ControlDialog(wxWindow* parent, const wxPoint& pos, bool showStart)
: wxDialog(parent, -1, "Controls", pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE), showStartup(showStart)
{
	//Sizers for layout:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	//The text:
	topSizer->Add(new wxStaticText(this,-1,"Controls"),0,wxALL|wxALIGN_LEFT,8);
	//...
	//Checkbox and button:
	topSizer->AddSpacer(8);
	wxCheckBox* cb = new wxCheckBox(this, LCHK_DIALOG, "Show this at startup");
	cb->SetValue(showStartup);
	topSizer->Add(cb,0,wxALL|wxALIGN_LEFT,10);
	topSizer->AddSpacer(8);
	topSizer->Add(new wxButton(this,BUTTON_OK,"Ok"),0,wxALL|wxALIGN_CENTER,10);

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->Fit(this); //Resize window to match topSizer minimal size
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
}

BEGIN_EVENT_TABLE(ControlDialog, wxDialog)
	EVT_CHECKBOX(LCHK_DIALOG, ControlDialog::toggleStartup)
    EVT_BUTTON(BUTTON_OK, ControlDialog::onOk)
    EVT_CLOSE(ControlDialog::onClose)
END_EVENT_TABLE()

void ControlDialog::toggleStartup(wxCommandEvent& event)
{
	showStartup=int(event.IsChecked());
}

void ControlDialog::onOk(wxCommandEvent& event)
{
	EndModal(showStartup);
}

void ControlDialog::onClose(wxCloseEvent& event)
{
	EndModal(showStartup);
}*/

//************************ AboutDialog ************************

AboutDialog::AboutDialog(wxWindow* parent, const wxPoint& pos)
: wxDialog(parent, -1, "About DMDB", pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	//Sizers for layout:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	topSizer->Add(new wxStaticText(this,-1,APP_TITLE),0,wxALL|wxALIGN_CENTER,8);
	wxStaticBitmap* aboutIcon = new wxStaticBitmap(this,-1,wxBitmap(_dmdb));
	topSizer->Add(aboutIcon,0,wxALL|wxALIGN_CENTER,10);
	topSizer->Add(new wxStaticText(this,-1,VERSIONSTRING+" (2020)"),0,wxALL|wxALIGN_CENTER,8);
	topSizer->Add(new wxStaticText(this,-1,"Coded in C++ using wxWidgets"),0,wxALL|wxALIGN_CENTER,10);
	topSizer->Add(new wxStaticText(this,-1,"By Lars Thomas Boye"),0,wxALL|wxALIGN_CENTER,10);
	topSizer->Add(new wxStaticText(this,-1,"Dedicated to"),0,wxTop|wxALIGN_CENTER,10);
	topSizer->Add(new wxStaticText(this,-1,"My lovely wife Meiji and son MM"),0,wxLeft|wxRight|wxALIGN_CENTER,25);
	topSizer->Add(20,20);
	topSizer->Add(new wxButton(this,BUTTON_OK,"Ok"),0,wxALL|wxALIGN_CENTER,10);

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->Fit(this); //Resize window to match topSizer minimal size
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
}

BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
    EVT_BUTTON(BUTTON_OK, AboutDialog::onOk)
    EVT_CLOSE(AboutDialog::onClose)
END_EVENT_TABLE()

void AboutDialog::onOk(wxCommandEvent& event)
{
	EndModal(0);
}

void AboutDialog::onClose(wxCloseEvent& event)
{
	EndModal(0);
}
