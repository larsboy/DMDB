#include "GuiMapReport.h"

//***********************************************************
//************************ GuiReport ************************
//***********************************************************

GuiReport::GuiReport(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size)
: wxDialog(parent, -1, title, pos, size, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	reportView = new wxRichTextCtrl(this, wxID_ANY, "", wxDefaultPosition,
			wxDefaultSize, wxRE_MULTILINE|wxRE_READONLY);
	paragraph = false;
}

void GuiReport::writeHeading(wxString text)
{
	if (paragraph) {
		reportView->BeginLeftIndent(0);
		reportView->Newline();
	}
	reportView->BeginFontSize(14);
	reportView->BeginUnderline();
	reportView->WriteText(text);
	reportView->EndStyle();
	reportView->Newline();
	paragraph = false;
}

void GuiReport::writeSubHeading(wxString text)
{
	if (paragraph) {
		reportView->BeginLeftIndent(0);
		reportView->Newline();
	}
	reportView->BeginFontSize(12);
	reportView->BeginBold();
	reportView->WriteText(text);
	reportView->Newline();
	reportView->EndStyle();
	paragraph = false;
}

void GuiReport::writeText(wxString text)
{
	if (!paragraph) {
		reportView->BeginLeftIndent(20);
		reportView->BeginFontSize(10);
		paragraph = true;
	}
	reportView->WriteText(text);
}

void GuiReport::writeLine(wxString text)
{
	if (!paragraph) {
		reportView->BeginLeftIndent(20);
		reportView->BeginFontSize(10);
		paragraph = true;
	}
	if (text.Length()>0)
		reportView->WriteText(text);
	reportView->Newline();
}

void GuiReport::setTabs(const wxArrayInt &tabs)
{
	wxTextAttr attr;
	attr.SetFlags(wxTEXT_ATTR_TABS);
	attr.SetTabs(tabs);
	reportView->SetDefaultStyle(attr);
}


//**************************************************************
//************************ GuiMapReport ************************
//**************************************************************

GuiMapReport::GuiMapReport(wxWindow* parent, const wxPoint& pos, MapStats* mapSt)
: GuiReport(parent, "Map report", pos, wxSize(500,400))
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	//Report view
	topSizer->Add(reportView, 1, wxEXPAND|wxALL, 8);

	//Buttons (Ok/Cancel) is the last part:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"OK"), 0, wxALL, 10);
	//bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Done"), 0, wxALL, 10);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size
    
    mapSt->printReport(this);
}

//Event table:
BEGIN_EVENT_TABLE(GuiMapReport, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiMapReport::onOk)
    EVT_CLOSE(GuiMapReport::onClose)
END_EVENT_TABLE()

void GuiMapReport::onOk(wxCommandEvent& event)
{
	EndModal(BUTTON_OK);
}

void GuiMapReport::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_OK);
}
