#include "GuiAspectDialog.h"

GuiAspectDialog::GuiAspectDialog(wxWindow* parent, const wxPoint& pos, WadStatAspects* asp, const wxString& title, const wxString& msg, bool repl)
: wxDialog(parent, -1, title, pos, wxSize(320,370), wxDEFAULT_DIALOG_STYLE), replaceFlag(false)
{
    aspects = asp;
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(new wxStaticText(this,-1,msg), 0, wxTOP|wxLEFT|wxRIGHT, 16);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    checks[0] = new wxCheckBox(this, -1, "Wad file data (name, size, year, engine, ...)");
    mainSizer->Add(checks[0], 0, wxALL, 2);

    checks[1] = new wxCheckBox(this, -1, "Wad content flags");
    mainSizer->Add(checks[1], 0, wxALL, 2);

    checks[2] = new wxCheckBox(this, -1, "Map titles");
    mainSizer->Add(checks[2], 0, wxALL, 2);

    checks[3] = new wxCheckBox(this, -1, "Map game mode fields (sp/coop/dm)");
    mainSizer->Add(checks[3], 0, wxALL, 2);

    checks[4] = new wxCheckBox(this, -1, "Map counts and flags (not gameplay stats)");
    mainSizer->Add(checks[4], 0, wxALL, 2);

    checks[5] = new wxCheckBox(this, -1, "Gameplay stats (hp, ratios)");
    mainSizer->Add(checks[5], 0, wxALL, 2);

    checks[6] = new wxCheckBox(this, -1, "Create map drawings");
    mainSizer->Add(checks[6], 0, wxALL, 2);

    mainSizer->AddSpacer(10);
    checks[7] = new wxCheckBox(this, -1, "Store zips we can't extract in failed folder");
    mainSizer->Add(checks[7], 0, wxALL, 2);

    if (repl) {
		checks[8] = new wxCheckBox(this, -1, "Overwrite existing entries on matching MD5 hash");
		mainSizer->Add(checks[8], 0, wxALL, 2);
    } else {
		checks[8] = NULL;
    }
    topSizer->Add(mainSizer, 0, wxALL, 16);

    if (repl) {
		wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Start"), 0, wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_EDIT,"More..."), 0, wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
		topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);
	} else {
		topSizer->Add(new wxButton(this,BUTTON_OK,"OK"), 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 16);
	}

    SetSizer(topSizer);
	topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

    writeData();
}

GuiAspectDialog::~GuiAspectDialog()
{
}

void GuiAspectDialog::writeData()
{
    checks[0]->SetValue(aspects->wadFile);
    checks[1]->SetValue(aspects->wadFlags);
    checks[2]->SetValue(aspects->mapMain);
    checks[3]->SetValue(aspects->gameModes);
    checks[4]->SetValue(aspects->mapStats);
    checks[5]->SetValue(aspects->gameStats);
    checks[6]->SetValue(aspects->mapImages);
    checks[7]->SetValue(aspects->copyFailedFiles);
    if (checks[8] != NULL)
		checks[8]->SetValue(replaceFlag);
    TransferDataToWindow();
}

void GuiAspectDialog::readData()
{
    TransferDataFromWindow();
    aspects->wadFile = checks[0]->GetValue();
    aspects->wadFlags = checks[1]->GetValue();
    aspects->mapMain = checks[2]->GetValue();
    aspects->gameModes = checks[3]->GetValue();
    aspects->mapStats = checks[4]->GetValue();
    aspects->gameStats = checks[5]->GetValue();
    aspects->mapImages = checks[6]->GetValue();
    aspects->copyFailedFiles = checks[7]->GetValue();
    if (checks[8] != NULL)
		replaceFlag = checks[8]->GetValue();
}

//Event table:
BEGIN_EVENT_TABLE(GuiAspectDialog, wxDialog)
	EVT_BUTTON(BUTTON_OK, GuiAspectDialog::onOk)
	EVT_BUTTON(BUTTON_EDIT, GuiAspectDialog::onMore)
    EVT_BUTTON(BUTTON_CANCEL, GuiAspectDialog::onCancel)
	EVT_CLOSE(GuiAspectDialog::onClose)
END_EVENT_TABLE()

void GuiAspectDialog::onOk(wxCommandEvent& event)
{
	readData();
	EndModal(BUTTON_OK);
}

void GuiAspectDialog::onMore(wxCommandEvent& event)
{
	readData();
	EndModal(BUTTON_EDIT);
}

void GuiAspectDialog::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiAspectDialog::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
