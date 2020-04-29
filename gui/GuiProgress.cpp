#include "GuiProgress.h"

GuiProgress::GuiProgress(wxWindow* parent, const wxPoint& pos, const wxSize& siz, bool log)
: wxDialog(parent, -1, "Progress", pos, siz, wxCAPTION)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	bar = new wxGauge(this,-1,100,wxDefaultPosition,wxDefaultSize,wxGA_HORIZONTAL|wxGA_PROGRESS);
	topSizer->Add(bar, 0, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER, 20);
	label = new wxStaticText(this,-1,"Starting up...",wxDefaultPosition,wxSize(200,40),
			wxALIGN_CENTRE_HORIZONTAL|wxST_NO_AUTORESIZE);
	topSizer->Add(label, 0, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER, 20);

	if (log) {
		logView = new wxRichTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
			wxDefaultSize, wxRE_MULTILINE|wxRE_READONLY);
		topSizer->Add(logView, 1, wxEXPAND|wxALL, 8);
		wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
		closeButton = new wxButton(this,BUTTON_OK,"Close");
		closeButton->Enable(false);
		bottomSizer->Add(closeButton, 0, wxALL, 10);
		//bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Done"), 0, wxALL, 10);
		topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);
	} else {
		logView = NULL;
		closeButton = NULL;
	}

	SetSizer(topSizer);
	if (!log) {
		topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
		topSizer->Fit(this); //Resize window to match topSizer minimal size
	}
}

GuiProgress::~GuiProgress()
{
}

void GuiProgress::setLabel(wxString name)
{
	label->SetLabel(name);
	Update();
}

void GuiProgress::setRange(int range, int pos)
{
	bar->SetRange(range);
	bar->SetValue(pos);
	Update();
}

void GuiProgress::setProgress(int pos)
{
	bar->SetValue(pos);
	Update();
}

void GuiProgress::logLine(wxString line)
{
	//TODO: Add colors?
	if (logView != NULL) {
		logView->WriteText(line);
		logView->Newline();
		logView->ShowPosition(logView->GetInsertionPoint());
	}
}

void GuiProgress::finished()
{
	if (closeButton == NULL) {
		Close(true);
	} else {
		closeButton->Enable(true);
	}
}

//Event table:
BEGIN_EVENT_TABLE(GuiProgress, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiProgress::onOk)
    //EVT_CLOSE(GuiProgress::onClose)
END_EVENT_TABLE()

void GuiProgress::onOk(wxCommandEvent& event)
{
	Close(true);
	Destroy();
}

//void GuiProgress::onClose(wxCloseEvent& event)
