#include "GuiComboDialog.h"

GuiComboDialog::GuiComboDialog(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
				ListWrapper<WadEntry*>* wl, DataManager* dm, DoomDialogMode m, int mapIndex)
: wxDialog(parent, -1, "Wad with maps", pos, siz, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
prevButton(NULL), nextButton(NULL)
{
	wadManager = new GuiWadManager(m, dm, true);
	mapManager = new GuiMapManager(m, dm, true);
	wadList = wl;

	dialogSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* wadSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainWadSizer = new wxBoxSizer(wxHORIZONTAL);

	// Left part - for wad:
	mainWadSizer->Add(wadManager->createMain(this, "Wad:"), 0, 0, 0);
	mainWadSizer->Add(wadManager->createFileOwn(this), 0, wxLEFT, 25);
	mainWadSizer->Add(wadManager->createContent(this), 0, wxLEFT|wxRIGHT, 25);
	wadSizer->Add(mainWadSizer, 0, wxTOP, 8);
	wadSizer->AddSpacer(8);
	wadSizer->Add(wadManager->createNotes(this), 1, wxEXPAND|wxALL, 0);
	wadSizer->AddSpacer(8);
	wadSizer->Add(new wxStaticText(this,-1,"Wad description/review:"),0);
	wadSizer->Add(wadManager->createText(this), 10, wxEXPAND|wxALL, 0);

	// Wad buttons:
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
	wadSizer->Add(bottomSizer,0,wxEXPAND);

	// Add wad part and map list:
	dialogSizer->Add(wadSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 16);
	dialogSizer->Add(wadManager->createMaps(this,true), 0, wxEXPAND|wxBOTTOM|wxRIGHT, 10);

	mapSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainMapSizer = new wxBoxSizer(wxHORIZONTAL);

	// First map column - main:
	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	leftSizer->Add(mapManager->createTitle(this,"Map:"),0,0,0);
	leftSizer->Add(mapManager->createRefs(this,GMB_AUTHORS|GMB_BASEDON),0,wxTOP,10);
	leftSizer->Add(mapManager->createModes(this),0,wxTOP,10);
	mainMapSizer->Add(leftSizer,0,0,0);

	// Second map column - Tags & Stats:
	wxBoxSizer* middleSizer = mapManager->createTags(this);
	middleSizer->Add(mapManager->createStats(this), 0);
	mainMapSizer->Add(middleSizer,0,wxLEFT,25);

	// Third map column - Own stuff & Skill 3 stats:
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(mapManager->createOwn(this), 0);
	rightSizer->AddSpacer(16);
	rightSizer->Add(mapManager->createGameplay(this), 0);
	mainMapSizer->Add(rightSizer,0,wxLEFT,25);

	mapSizer->Add(mainMapSizer,0,wxTOP,8);
	mapSizer->AddSpacer(8);

	noteLabel = new wxStaticText(this,-1,"Map description/review:");
	mapSizer->Add(noteLabel,0);
	noteCtrl = mapManager->createText(this);
	mapSizer->Add(noteCtrl, 1, wxEXPAND|wxBOTTOM, 10);
	dialogSizer->Add(mapSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 16);

	// Images:
	wxConfigBase* config = wxConfigBase::Get();
	int sashPos = (int)config->Read("/ComboEntry/split", -200);
	imageSplitter = new wxSplitterWindow(this, FRAME_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
	GuiImagePanel* mapPane = mapManager->createMapPane(imageSplitter);
	GuiImagePanel* shotPane = mapManager->createShotPane(imageSplitter);
	imageSplitter->SplitHorizontally(mapPane, shotPane, sashPos);
	imageSplitter->SetMinimumPaneSize(40);
	dialogSizer->Add(imageSplitter, 1, wxEXPAND|wxALL, 10);

	//Implementing the sizer:
    SetSizer(dialogSizer);
    //dialogSizer->SetSizeHints(this); //Set minimal size for window based on dialogSizer
    //dialogSizer->Fit(this); //Resize window to match dialogSizer minimal size

    showWad(mapIndex);
}

GuiComboDialog::~GuiComboDialog()
{
	delete wadManager;
	delete mapManager;
}

void GuiComboDialog::savePlacement(wxConfigBase* config)
{
	int x, y, w, h;
	GetPosition(&x, &y);
	GetSize(&w, &h);
	config->Write("/ComboEntry/x", (long) x);
	config->Write("/ComboEntry/y", (long) y);
	config->Write("/ComboEntry/w", (long) w);
	config->Write("/ComboEntry/h", (long) h);
	config->Write("/ComboEntry/split", (long)imageSplitter->GetSashPosition());
}

void GuiComboDialog::showWad(int mapIndex)
{
	WadEntry* we = wadList->entry();
	//Only show Map description for multi-map wads:
	if (we->numberOfMaps == 1) {
		mapSizer->Show(noteLabel, false);
		mapSizer->Show(noteCtrl, false);
	} else {
		mapSizer->Show(noteLabel, true);
		mapSizer->Show(noteCtrl, true);
	}
	wadManager->setWadEntry(we);
	mapManager->setMapEntry(we->mapPointers[mapIndex]);
    wadManager->writeEntry();
	refreshMap(); //Calls TransferDataToWindow();
	wadManager->setSelectedMap(mapIndex);
	if (prevButton != NULL)
		prevButton->Enable(!wadList->isFirst());
	if (nextButton != NULL)
		nextButton->Enable(!wadList->isLast());
}

void GuiComboDialog::refreshMap()
{
	mapManager->writeEntry();
	dialogSizer->Layout();
	TransferDataToWindow();
    mapManager->writeTagList();
    mapManager->showImages();
}

//Event table:
BEGIN_EVENT_TABLE(GuiComboDialog, wxDialog)
	EVT_LISTBOX(LMAP_LIST, onMapSelected)
	EVT_BUTTON(BUTTON_DELETE, onRemoveMap)
	EVT_BUTTON(BUTTON_EDIT, onEditMaps)
	EVT_BUTTON(BUTTON_PREVMAP, onPrevWad)
	EVT_BUTTON(BUTTON_NEXTMAP, onNextWad)

	EVT_BUTTON(BUTTON_AUTHOR1, onAuthorButton)
	EVT_BUTTON(BUTTON_AUTHOR2, onAuthorButton)
	EVT_BUTTON(BUTTON_MAP, onBasedButton)
	EVT_BUTTON(BUTTON_TAGADD, onAddTag)
	EVT_BUTTON(BUTTON_TAGDEL, onRemoveTag)

    EVT_BUTTON(BUTTON_OK, onOk)
    EVT_BUTTON(BUTTON_CANCEL, onCancel)
    EVT_CLOSE(onClose)
END_EVENT_TABLE()

void GuiComboDialog::onMapSelected(wxCommandEvent& event)
{
	int index = event.GetSelection();
	TransferDataFromWindow();
	wadManager->readEntry(); //?
	mapManager->readEntry();

	MapEntry* me = wadList->entry()->mapPointers[index];
	mapManager->setMapEntry(me);
	wadManager->writeEntry(); //?
	refreshMap(); //Calls TransferDataToWindow
	wadManager->setSelectedMap(index);
}

void GuiComboDialog::onRemoveMap(wxCommandEvent& event)
{
	//wadManager->deleteSelectedMap(this);
	//TODO: Update map part
}

void GuiComboDialog::onEditMaps(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	mapManager->readEntry();
	wadManager->editMapCommon(this);

	wadManager->writeEntry();
	mapManager->writeEntry();
	dialogSizer->Layout();
	TransferDataToWindow();
    mapManager->writeTagList();
}

void GuiComboDialog::onPrevWad(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	mapManager->readEntry();
	wadList->previous();
	showWad();
}

void GuiComboDialog::onNextWad(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	mapManager->readEntry();
	wadList->next();
	showWad();
}

void GuiComboDialog::onAuthorButton(wxCommandEvent& event)
{
	bool second = (event.GetId() == BUTTON_AUTHOR2);
	if (mapManager->mode == DDMODE_VIEW) {
		mapManager->showAuthor(this, second);
	} else {
		TransferDataFromWindow();
		mapManager->editAuthor(this, second);
		TransferDataToWindow();
	}
}

void GuiComboDialog::onBasedButton(wxCommandEvent& event)
{
	if (mapManager->mode == DDMODE_VIEW) {
		mapManager->showBasedOn(this);
	} else {
		TransferDataFromWindow();
		mapManager->editBasedOn(this);
		TransferDataToWindow();
	}
}

void GuiComboDialog::onAddTag(wxCommandEvent& event)
{
	mapManager->addTag(this);
}

void GuiComboDialog::onRemoveTag(wxCommandEvent& event)
{
	mapManager->removeSelectedTag();
}

void GuiComboDialog::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	wadManager->readEntry();
	mapManager->readEntry();
	EndModal(BUTTON_OK);
}

void GuiComboDialog::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiComboDialog::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
