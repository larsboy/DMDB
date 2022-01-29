#include "GuiWadPanel.h"

GuiWadPanel::GuiWadPanel(wxWindow* parent)
: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize)
{
	wadManager = new GuiWadManager(DDMODE_VIEW);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	topSizer->Add(wadManager->createMain(this, "Wad:"), 0, wxALL, 10);
	topSizer->Add(wadManager->createFileOwn(this,true,true), 0, wxLEFT|wxRIGHT, 10);
	topSizer->Add(wadManager->createMaps(this), 1, wxEXPAND|wxALL, 10);

	SetSizer(topSizer);
}

GuiWadPanel::~GuiWadPanel()
{
	delete wadManager;
}

void GuiWadPanel::setDataManager(DataManager* dm)
{
	wadManager->setDataManager(dm);
	if (dm == NULL)
		TransferDataToWindow();
}

void GuiWadPanel::setWad(WadEntry* wad)
{
	wadManager->setWadEntry(wad);
    wadManager->writeEntry();
	TransferDataToWindow();
}

void GuiWadPanel::refresh()
{
	wadManager->writeEntry();
	TransferDataToWindow();
}

void GuiWadPanel::clearWad()
{
	wadManager->clearEntry();
	TransferDataToWindow();
}

void GuiWadPanel::readChanges()
{
	if (wadManager->hasEntry()) {
		TransferDataFromWindow();
		wadManager->readEntry();
	}
}

//Event table:
BEGIN_EVENT_TABLE(GuiWadPanel, wxPanel)
	EVT_LISTBOX_DCLICK(LMAP_LIST, GuiWadPanel::onMapSelected)
END_EVENT_TABLE()

void GuiWadPanel::onMapSelected(wxCommandEvent& event)
{
	int index = event.GetSelection();
	TransferDataFromWindow();
	wadManager->readEntry();
	wadManager->modalMapDialog(this, index);
	wadManager->writeEntry();
	TransferDataToWindow();
}
