#include "GuiMapSelect.h"

//************************************************************
//************************ GuiWadList ************************
//************************************************************

GuiWadList::GuiWadList(wxWindow* parent, const wxSize& siz)
: wxListCtrl(parent, LWAD_LIST, wxDefaultPosition, siz, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL|wxLC_VIRTUAL),
	wadList(NULL)
{
	InsertColumn(0, "");
}

GuiWadList::~GuiWadList()
{
}

void GuiWadList::setWadList(ListWrapper<WadEntry*>* wads)
{
	wadList = wads;
	DeleteAllItems();
	unsigned int i = wadList->getSize();
	SetItemCount(i);
	if (i > 0)
		RefreshItems(0, i-1);
}

bool GuiWadList::setSelected(WadEntry* wad)
{
	int index = -1;
	if (wadList->getSize()>0) {
		wadList->reset();
		if (wadList->entry() == wad)
			index = wadList->getIndex();
		while (wadList->next() && index==-1) {
			if (wadList->entry() == wad)
				index = wadList->getIndex();
		}
	}
	if (index > -1) {
		SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		EnsureVisible(index);
		return true;
	} else {
		return false;
	}
}

void GuiWadList::jumpToWad(wxString prefix)
{
	int index = -1;
	if ((prefix.Length()>0) && (wadList->getSize()>0)) {
		wadList->reset();
		if ( wxString(wadList->entry()->title).CmpNoCase(prefix) >= 0)
			index = wadList->getIndex();
		while (wadList->next() && index==-1) {
			if ( wxString(wadList->entry()->title).CmpNoCase(prefix) >= 0)
				index = wadList->getIndex();
		}
	}
	if (index > -1) {
		EnsureVisible(index);
	} else {
		EnsureVisible(0);
	}
}

WadEntry* GuiWadList::getWad(long index)
{
	wadList->setIndex(index);
	return wadList->entry();
}

wxString GuiWadList::OnGetItemText(long item, long column) const
{
	if (wadList == NULL) return "";
	wadList->setIndex(item);
	return wadList->entry()->title;
}

//**************************************************************
//************************ GuiMapSelect ************************
//**************************************************************

GuiMapSelect::GuiMapSelect(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, MapEntry* mapEnt, bool basedOn)
: wxDialog(parent, -1, "Select map", pos, wxSize(320,500), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ), dataBase(dataMan),
wadList(NULL), sameAs(NULL), startMap(0), forBasedOn(NULL), currentWad(NULL), mapIndex(-1), searchChars(0)
{
	wadListCtrl = new GuiWadList(this, wxSize(150,260));
	mapListCtrl = new wxListBox(this, LMAP_LIST, wxDefaultPosition,  wxSize(150,260));
	if (mapEnt != NULL) {
		currentWad = mapEnt->wadPointer;
		if (basedOn) {
			startMap = mapEnt->basedOn;
			forBasedOn = mapEnt;
			if (startMap == 0) {
				//Use own wad as initial selection
			} else {
				MapEntry* based = dataBase->findMap(startMap);
				currentWad = based->wadPointer;
				mapIndex = getMapIndex(startMap);
			}
			sameAs = new wxCheckBox(this,-1,"Same as");
			sameAs->SetValue(forBasedOn->flags&MF_SAMEAS);
		} else {
			startMap = mapEnt->dbid;
			mapIndex = getMapIndex(startMap);
		}
	}

	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	searchCtrl = new wxSearchCtrl(this,SEARCH_FIELD,"",wxDefaultPosition,wxDefaultSize);//wxSize(100,20)
	searchCtrl->ShowCancelButton(true);
    topSizer->Add(searchCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
    if (sameAs != NULL)
		topSizer->Add(sameAs, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 8);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(wadListCtrl, 1, wxEXPAND|wxALL, 2);
	mainSizer->Add(mapListCtrl, 1, wxEXPAND|wxALL, 2);
	topSizer->Add(mainSizer, 1, wxEXPAND|wxALL, 6);

	//Buttons:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"None"), 0, wxALL, 5);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"Ok"), 0, wxALL, 5);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

	createWadList("");
}

GuiMapSelect::~GuiMapSelect()
{
	if (wadList != NULL)
		delete wadList;
}

int GuiMapSelect::getMapIndex(uint32_t dbid)
{
	int ind = -1;
	for (int i=0; i<currentWad->numberOfMaps; i++) {
		if (currentWad->mapPointers[i]->dbid == dbid)
			ind = i;
	}
	return ind;
}

void GuiMapSelect::createWadList(wxString sstr, bool update)
{
	wxLogVerbose("createWadList for search %s", sstr);
	if ((wadList != NULL) && !update) {
		delete wadList;
		wadList = NULL;
	}
	ListWrapper<WadEntry*>* newList = dataBase->getWadTitleList(sstr,wadList);
	if (wadList != NULL) delete wadList;
	wadList = newList;
	wadListCtrl->setWadList(wadList);
	wxLogVerbose("Sat list of size %i", wadList->getSize());

	if (currentWad != NULL) {
		bool included = wadListCtrl->setSelected(currentWad);
		if (included) {
			showMaps();
			if (mapIndex > -1)
				mapListCtrl->SetSelection(mapIndex);
		} else {
			mapListCtrl->Clear();
			mapIndex = -1;
		}
	}
}

void GuiMapSelect::showMaps()
{
	mapListCtrl->Clear();
	if (currentWad != NULL) {
		for (int i=0; i<currentWad->numberOfMaps; i++)
			mapListCtrl->Append(currentWad->mapPointers[i]->name);
	}
}

void GuiMapSelect::illegalMap() {
	wxMessageDialog* dlg = new wxMessageDialog(this, "A map can't be based on itself.",
		"Illegal selection", wxOK | wxICON_ERROR);
	dlg->ShowModal();
	dlg->Destroy();
}

MapEntry* GuiMapSelect::getSelectedMap()
{
	if (currentWad == NULL)
		return NULL;
	if (mapIndex > -1)
		return currentWad->mapPointers[mapIndex];
	else
		return NULL;
}

//Event table:
BEGIN_EVENT_TABLE(GuiMapSelect, wxDialog)
	EVT_SIZE(GuiMapSelect::onResize)
	EVT_LIST_ITEM_FOCUSED(LWAD_LIST, GuiMapSelect::wadFocused)
	EVT_LISTBOX_DCLICK(LMAP_LIST, GuiMapSelect::mapActivated)
	EVT_TEXT(SEARCH_FIELD, GuiMapSelect::onSearchText)

    EVT_BUTTON(BUTTON_OK, GuiMapSelect::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiMapSelect::onNone)
    EVT_CLOSE(GuiMapSelect::onClose)
END_EVENT_TABLE()

void GuiMapSelect::onResize(wxSizeEvent& event)
{
	wxSize s = wadListCtrl->GetClientSize();
	wadListCtrl->SetColumnWidth(0, s.GetWidth());
	event.Skip();
}

void GuiMapSelect::wadFocused(wxListEvent& event)
{
	long wadIndex = event.GetIndex(); //GetItem().GetId();
	if (wadIndex >= 0)
		currentWad = wadListCtrl->getWad(wadIndex);
	else
		currentWad = NULL;
	showMaps();
}

void GuiMapSelect::mapActivated(wxCommandEvent& event)
{
	mapIndex = event.GetSelection();
	if (mapIndex>=0 && mapIndex<currentWad->numberOfMaps) {
		if (startMap == currentWad->mapPointers[mapIndex]->dbid)
			selectionDone(BUTTON_CANCEL); //No chance
		else if (currentWad->mapPointers[mapIndex] == forBasedOn)
			illegalMap();
		else
			selectionDone(BUTTON_OK); //Change
	} else {
		selectionDone(BUTTON_CANCEL);
	}
}

void GuiMapSelect::onSearchText(wxCommandEvent& event)
{
	wxString sstr = searchCtrl->GetValue();
    if (sstr.Length() == 0) {
		if (searchChars > 2) {
			createWadList(sstr);
		}
    } else if (sstr.Length() > 2) {
    	bool update = (sstr.Length() > searchChars);
        createWadList(sstr, update);
    } else {
    	if (searchChars > 2) {
			createWadList("");
		}
		wadListCtrl->jumpToWad(sstr);
    }
    searchChars = sstr.Length();
}

void GuiMapSelect::selectionDone(int retCode)
{
	if (forBasedOn != NULL) {
		if (retCode == BUTTON_OK) {
			//Set new selection on forBasedOn
			if ((currentWad==NULL) || (mapIndex<0)) {
				forBasedOn->basedOn = 0;
			} else {
				forBasedOn->basedOn = currentWad->mapPointers[mapIndex]->dbid;
			}
			forBasedOn->ownFlags |= OF_MAINMOD;
		}
		//sameAs flag:
		unsigned char flg = forBasedOn->flags;
		if (sameAs->GetValue()) flg |= MF_SAMEAS;
		else flg &= ~MF_SAMEAS;
		if (flg != forBasedOn->flags) {
			forBasedOn->flags = flg;
			forBasedOn->ownFlags |= OF_MAINMOD;
		}
	}
	EndModal(retCode);
}

void GuiMapSelect::onOk(wxCommandEvent& event)
{
	if (currentWad != NULL)
		mapIndex = mapListCtrl->GetSelection();
	else
		mapIndex = wxNOT_FOUND;

	if (mapIndex == wxNOT_FOUND) { //No map selected
		if (startMap == 0)
			selectionDone(BUTTON_CANCEL); //No chance
		else
			selectionDone(BUTTON_OK); //Change
	} else {
		if (startMap == currentWad->mapPointers[mapIndex]->dbid)
			selectionDone(BUTTON_CANCEL); //No chance
		else if (currentWad->mapPointers[mapIndex] == forBasedOn)
			illegalMap();
		else
			selectionDone(BUTTON_OK); //Change
	}
}

void GuiMapSelect::onNone(wxCommandEvent& event)
{
	currentWad = NULL;
	mapIndex = -1;
	if (startMap == 0)
		selectionDone(BUTTON_CANCEL); //No chance
	else
		selectionDone(BUTTON_OK); //Change
}

void GuiMapSelect::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
