#include "GuiMapEntry.h"

GuiMapEntry::GuiMapEntry(wxWindow* parent, const wxPoint& pos, const wxSize& siz,
			ListWrapper<MapEntry*>* ml, DataManager* dm, DoomDialogMode m, bool openWads)
: wxDialog(parent, -1, "Map entry", pos, siz, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
prevButton(NULL), nextButton(NULL), openStuff(openWads)
{
	mapManager = new GuiMapManager(m, dm);
	mapList = ml;

	wxBoxSizer* dialogSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	// First column - main:
	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	leftSizer->Add(mapManager->createTitle(this,"Main:"),0,0,0);
	char buttons = GMB_AUTHORS | GMB_BASEDON;
	if (openStuff) buttons |= GMB_WAD;
	leftSizer->Add(mapManager->createRefs(this,buttons),0,wxTOP,10);
	leftSizer->Add(mapManager->createModes(this),0,wxTOP,10);
	mainSizer->Add(leftSizer,0,0,0);

	// Second column - Tags & Stats:
	wxBoxSizer* middleSizer = mapManager->createTags(this);
	middleSizer->Add(mapManager->createStats(this), 0);
	mainSizer->Add(middleSizer,0,wxLEFT,25);

	// Third column - Own stuff & Skill 3 stats:
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(mapManager->createOwn(this), 0);
	rightSizer->AddSpacer(16);
	rightSizer->Add(mapManager->createGameplay(this), 0);
	mainSizer->Add(rightSizer,0,wxLEFT,25);

	topSizer->Add(mainSizer,0,wxTOP,8);
	topSizer->AddSpacer(8);

	// Text inputs:
	topSizer->Add(new wxStaticText(this,-1,"Description/review:"),0);
	topSizer->Add(mapManager->createText(this), 10, wxEXPAND|wxALL, 0);

	// Buttons:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (mapList->getSize() > 1) {
		prevButton = new wxButton(this,BUTTON_PREVMAP,"< Previous");
		nextButton = new wxButton(this,BUTTON_NEXTMAP,"Next >");
		bottomSizer->Add(prevButton, 1, wxEXPAND|wxALL, 10);
	}
	if (m == DDMODE_NEW) {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Back to Wad"), 2, wxEXPAND|wxALL, 10);
	} else { //EDIT/VIEW
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Done"), 2, wxEXPAND|wxALL, 10);
	}
	if (nextButton != NULL)
		bottomSizer->Add(nextButton, 1, wxEXPAND|wxALL, 10);
	topSizer->Add(bottomSizer,0,wxEXPAND);

	dialogSizer->Add(topSizer, 0, wxEXPAND|wxLEFT|wxRIGHT, 16);

	// Images:
	wxConfigBase* config = wxConfigBase::Get();
	int sashPos = (int)config->Read("/MapEntry/split", -200);
	//wxLogVerbose("Loaded sashPos %i", sashPos);
	imageSplitter = new wxSplitterWindow(this, FRAME_SPLITTER, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH);
	GuiImagePanel* mapPane = mapManager->createMapPane(imageSplitter);
	GuiImagePanel* shotPane = mapManager->createShotPane(imageSplitter);
	imageSplitter->SplitHorizontally(mapPane, shotPane, sashPos);
	imageSplitter->SetMinimumPaneSize(40);
	dialogSizer->Add(imageSplitter, 1, wxEXPAND|wxALL, 10);

	//Implementing the sizer:
    SetSizer(dialogSizer);
    //dialogSizer->SetSizeHints(this); //Set minimal size for window based on dialogSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size

    mapManager->setMapEntry(mapList->entry());
    refreshAll();
}

GuiMapEntry::~GuiMapEntry()
{
	delete mapManager;
}

void GuiMapEntry::savePlacement(wxConfigBase* config)
{
	int x, y, w, h;
	GetPosition(&x, &y);
	GetSize(&w, &h);
	config->Write("/MapEntry/x", (long) x);
	config->Write("/MapEntry/y", (long) y);
	config->Write("/MapEntry/w", (long) w);
	config->Write("/MapEntry/h", (long) h);
	config->Write("/MapEntry/split", (long)imageSplitter->GetSashPosition());
}

void GuiMapEntry::refreshAll()
{
	mapManager->writeEntry();
	TransferDataToWindow();
    mapManager->writeTagList();
    if (prevButton != NULL)
		prevButton->Enable(!mapList->isFirst());
	if (nextButton != NULL)
		nextButton->Enable(!mapList->isLast());
    mapManager->showImages();
}

//Event table:
BEGIN_EVENT_TABLE(GuiMapEntry, wxDialog)
	EVT_BUTTON(BUTTON_AUTHOR1, onAuthorButton)
	EVT_BUTTON(BUTTON_AUTHOR2, onAuthorButton)
	EVT_BUTTON(BUTTON_MAP, onBasedButton)
	EVT_BUTTON(BUTTON_WAD, onWadButton)
	EVT_BUTTON(BUTTON_TAGADD, onAddTag)
	EVT_BUTTON(BUTTON_TAGDEL, onRemoveTag)

	EVT_BUTTON(BUTTON_PREVMAP, onPrevMap)
	EVT_BUTTON(BUTTON_NEXTMAP, onNextMap)
    EVT_BUTTON(BUTTON_OK, GuiMapEntry::onOk)
    EVT_CLOSE(GuiMapEntry::onClose)
END_EVENT_TABLE()

void GuiMapEntry::onAuthorButton(wxCommandEvent& event)
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

void GuiMapEntry::onBasedButton(wxCommandEvent& event)
{
	if (mapManager->mode == DDMODE_VIEW) {
		mapManager->showBasedOn(this);
	} else {
		TransferDataFromWindow();
		mapManager->editBasedOn(this);
		TransferDataToWindow();
	}
}

void GuiMapEntry::onWadButton(wxCommandEvent& event)
{
	if (openStuff) {
		if (mapManager->mode != DDMODE_VIEW) TransferDataFromWindow();
		mapManager->openWad(this);
		if (mapManager->mode != DDMODE_VIEW) TransferDataToWindow();
	}
}

void GuiMapEntry::onAddTag(wxCommandEvent& event)
{
	mapManager->addTag(this);
}

void GuiMapEntry::onRemoveTag(wxCommandEvent& event)
{
	mapManager->removeSelectedTag();
}

void GuiMapEntry::onPrevMap(wxCommandEvent& event)
{
	TransferDataFromWindow();
	mapManager->readEntry();
	mapList->previous();
	mapManager->setMapEntry(mapList->entry());
	refreshAll();
}

void GuiMapEntry::onNextMap(wxCommandEvent& event)
{
	TransferDataFromWindow();
	mapManager->readEntry();
	mapList->next();
	mapManager->setMapEntry(mapList->entry());
	refreshAll();
}

void GuiMapEntry::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	mapManager->readEntry();
	EndModal(BUTTON_OK);
}

void GuiMapEntry::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiMapEntry::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
