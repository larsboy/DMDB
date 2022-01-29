#include "GuiMapCommon.h"

GuiMapCommon::GuiMapCommon(wxWindow* parent, const wxPoint& pos, MapEntry* me, DataManager* dm, bool dir)
: wxDialog(parent, -1, "Map template", pos, wxSize(420,370), wxDEFAULT_DIALOG_STYLE) //|wxRESIZE_BORDER
{
	mapManager = new GuiMapManager(DDMODE_EDIT, dm, true);

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	if (dir) {
		topSizer->Add(new wxStaticText(this,-1,"Specify values to be applied to all maps in all wads."),0,wxLEFT|wxRIGHT|wxTOP,16);
		topSizer->Add(new wxStaticText(this,-1,"Leave empty for default values (game modes are inferred from map data)."),0,wxLEFT|wxRIGHT|wxBOTTOM,16);
	} else {
		topSizer->Add(new wxStaticText(this,-1,"Specify values to be applied to all maps in the wad."),0,wxLEFT|wxRIGHT|wxTOP,16);
		topSizer->Add(new wxStaticText(this,-1,"Unspecified fields are not modified."),0,wxLEFT|wxRIGHT|wxBOTTOM,16);
	}
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	leftSizer->Add(mapManager->createRefs(this,GMB_AUTHORS),0,wxBOTTOM,10);
	leftSizer->Add(mapManager->createTags(this),0,wxBOTTOM,10);
	mainSizer->Add(leftSizer,0,0,0);

	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(mapManager->createModes(this,true),0,wxBOTTOM,10);
	rightSizer->Add(mapManager->createOwn(this,true),0,0,0);
	mainSizer->Add(rightSizer,0,wxLEFT,25);
	topSizer->Add(mainSizer,0,wxLEFT|wxRIGHT,16);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (dir)
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Start"), 0, wxALL, 10);
	else
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"OK"), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	topSizer->Add(bottomSizer, 0, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL, 0);

	SetSizer(topSizer);
	topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

    mapManager->setMapEntry(me);
	mapManager->writeEntry();
	TransferDataToWindow();
    mapManager->writeTagList();
}

GuiMapCommon::~GuiMapCommon()
{
	delete mapManager;
}

//Event table:
BEGIN_EVENT_TABLE(GuiMapCommon, wxDialog)
	EVT_BUTTON(BUTTON_AUTHOR1, onAuthorButton)
	EVT_BUTTON(BUTTON_AUTHOR2, onAuthorButton)
	EVT_BUTTON(BUTTON_TAGADD, onAddTag)
	EVT_BUTTON(BUTTON_TAGDEL, onRemoveTag)

	EVT_BUTTON(BUTTON_OK, GuiMapCommon::onOk)
	EVT_BUTTON(BUTTON_CANCEL, GuiMapCommon::onCancel)
	EVT_CLOSE(GuiMapCommon::onClose)
END_EVENT_TABLE()

void GuiMapCommon::onAuthorButton(wxCommandEvent& event)
{
	bool second = (event.GetId() == BUTTON_AUTHOR2);
	TransferDataFromWindow();
	mapManager->editAuthor(this, second);
	TransferDataToWindow();
}

void GuiMapCommon::onAddTag(wxCommandEvent& event)
{
	mapManager->addTag(this);
}

void GuiMapCommon::onRemoveTag(wxCommandEvent& event)
{
	mapManager->removeSelectedTag();
}

void GuiMapCommon::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	mapManager->readEntry();
	EndModal(BUTTON_OK);
}

void GuiMapCommon::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiMapCommon::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
