#include "GuiMapPanel.h"

GuiMapPanel::GuiMapPanel(wxWindow* parent)
: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize)
{
	mapManager = new GuiMapManager(DDMODE_VIEW);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	topSizer->Add(mapManager->createTitle(this,"Map:"), 0, wxALL, 10);
	topSizer->Add(mapManager->createRefs(this,0), 0, wxLEFT|wxRIGHT, 10);
	topSizer->AddSpacer(10);
	wxBoxSizer* tags = mapManager->createTags(this);
	topSizer->Add(tags, 0, wxLEFT, 70);
	topSizer->Add(mapManager->createOwn(this), 0, wxLEFT|wxRIGHT, 10);
	GuiImagePanel* shotPane = mapManager->createShotPane(this);
	topSizer->Add(shotPane, 1, wxEXPAND|wxALL, 10);

	SetSizer(topSizer);
}

GuiMapPanel::~GuiMapPanel()
{
	delete mapManager;
}

void GuiMapPanel::setDataManager(DataManager* dm)
{
	mapManager->setDataManager(dm);
	if (dm == NULL)
		TransferDataToWindow();
}

void GuiMapPanel::setMap(MapEntry* mapEnt)
{
	mapManager->setMapEntry(mapEnt);
    mapManager->writeEntry();
	TransferDataToWindow();
	mapManager->writeTagList();
	mapManager->showImages();
}

void GuiMapPanel::refresh()
{
	mapManager->writeEntry();
	TransferDataToWindow();
	mapManager->writeTagList();
	mapManager->showImages();
}

void GuiMapPanel::clearMap()
{
	mapManager->clearEntry();
	TransferDataToWindow();
}

void GuiMapPanel::readChanges()
{
	if (mapManager->hasEntry()) {
		TransferDataFromWindow();
		mapManager->readEntry();
	}
}
