#include "GuiWadEntry.h"

GuiWadEntry::GuiWadEntry(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
			ListWrapper<WadEntry*>* wl, DataManager* dm, DoomDialogMode m, bool openMaps)
: wxDialog(parent, -1, "Wad entry", pos, siz, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
commonButton(NULL), prevButton(NULL), nextButton(NULL), openMap(openMaps)
{
	wadManager = new GuiWadManager(m, dm);
	wadList = wl;

	wxBoxSizer* dialogSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	mainSizer->Add(wadManager->createMain(this, "Main:"), 0, 0, 0);
	mainSizer->Add(wadManager->createFileOwn(this), 0, wxLEFT, 25);
	mainSizer->Add(wadManager->createContent(this), 0, wxLEFT|wxRIGHT, 25);

	topSizer->Add(mainSizer, 0, wxTOP, 8);
	topSizer->AddSpacer(8);
	topSizer->Add(wadManager->createNotes(this), 1, wxEXPAND|wxALL, 0);
	topSizer->AddSpacer(8);

	topSizer->Add(new wxStaticText(this,-1,"Description/review:"),0);
	topSizer->Add(wadManager->createText(this), 10, wxEXPAND|wxALL, 0);

	// Buttons:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (wadList->getSize() > 1) {
		prevButton = new wxButton(this,BUTTON_PREVMAP,"< Previous");
		nextButton = new wxButton(this,BUTTON_NEXTMAP,"Next >");
		bottomSizer->Add(prevButton, 1, wxEXPAND|wxALL, 10);
	}
	if (wadManager->mode == DDMODE_NEW) {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Save"), 1, wxEXPAND|wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 1, wxEXPAND|wxALL, 10);
	} else {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Done"), 2, wxEXPAND|wxALL, 10);
	}
	if (nextButton != NULL)
		bottomSizer->Add(nextButton, 1, wxEXPAND|wxALL, 10);
	topSizer->Add(bottomSizer,0,wxEXPAND);

	dialogSizer->Add(topSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 16);
	dialogSizer->Add(wadManager->createMaps(this,true,true), 1, wxEXPAND|wxBOTTOM|wxRIGHT, 16);

	//Implementing the sizer:
    SetSizer(dialogSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size

    showWad();
}

GuiWadEntry::~GuiWadEntry()
{
	delete wadManager;
}

void GuiWadEntry::savePlacement(wxConfigBase* config)
{
	int x, y, w, h;
	GetPosition(&x, &y);
	GetSize(&w, &h);
	config->Write("/WadEntry/x", (long) x);
	config->Write("/WadEntry/y", (long) y);
	config->Write("/WadEntry/w", (long) w);
	config->Write("/WadEntry/h", (long) h);
}

void GuiWadEntry::showWad()
{
	wadManager->setWadEntry(wadList->entry());
    wadManager->writeEntry();
	TransferDataToWindow();
	if (prevButton != NULL)
		prevButton->Enable(!wadList->isFirst());
	if (nextButton != NULL)
		nextButton->Enable(!wadList->isLast());
}

//Event table:
BEGIN_EVENT_TABLE(GuiWadEntry, wxDialog)
	EVT_LISTBOX_DCLICK(LMAP_LIST, GuiWadEntry::onMapSelected)
	EVT_BUTTON(BUTTON_DELETE, GuiWadEntry::onRemoveMap)
	EVT_BUTTON(BUTTON_EDIT, GuiWadEntry::onEditMaps)
	EVT_BUTTON(BUTTON_PREVMAP, GuiWadEntry::onPrevWad)
	EVT_BUTTON(BUTTON_NEXTMAP, GuiWadEntry::onNextWad)
    EVT_BUTTON(BUTTON_OK, GuiWadEntry::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiWadEntry::onCancel)
    EVT_CLOSE(GuiWadEntry::onClose)
END_EVENT_TABLE()

void GuiWadEntry::onMapSelected(wxCommandEvent& event)
{
	if (openMap) {
		int index = event.GetSelection();
		TransferDataFromWindow();
		wadManager->readEntry();
		wadManager->modalMapDialog(this, index);
		wadManager->writeEntry();
		TransferDataToWindow();
	}
}

void GuiWadEntry::onRemoveMap(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	wadManager->deleteSelectedMap(this);
	wadManager->writeEntry();
	TransferDataToWindow();
}

void GuiWadEntry::onEditMaps(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	wadManager->editMapCommon(this);
	wadManager->writeEntry();
	TransferDataToWindow();
}

void GuiWadEntry::onPrevWad(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	wadList->previous();
	showWad();
}

void GuiWadEntry::onNextWad(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	wadList->next();
	showWad();
}

void GuiWadEntry::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	EndModal(BUTTON_OK);
}

void GuiWadEntry::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiWadEntry::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
