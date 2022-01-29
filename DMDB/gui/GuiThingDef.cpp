#include "GuiThingDef.h"

bool compare_id(const ThingDef* first, const ThingDef* second)
{
	return (first->id < second->id);
}

bool compare_deh(const ThingDef* first, const ThingDef* second)
{
	return (first->dehacked < second->dehacked);
}

bool compare_name(const ThingDef* first, const ThingDef* second)
{
	return (first->name.compare(second->name) < 0);
}

bool compare_cat(const ThingDef* first, const ThingDef* second)
{
	if (first->cat != second->cat)
		return (first->cat < second->cat);
	else
		return (first->id < second->id);
}

//****************************************************************
//************************ GuiThingDialog ************************
//****************************************************************

GuiThingDialog::GuiThingDialog(wxWindow* parent, const wxPoint& pos, ThingDefList* things, bool onlyDone)
: wxDialog(parent, -1, "Thing definitions", pos, wxSize(580,500), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
thingIndex(-1), currentThing(NULL)
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	listCtrl = new wxListCtrl(this, LVIEW_LIST, wxDefaultPosition, wxDefaultSize,
			wxLC_REPORT|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL);

	createColumn(THA_ID, 50);
	createColumn(THA_DEH, 50);
	createColumn(THA_NAME, 100);
	createColumn(THA_CAT, 100);
	createColumn(THA_HP, 50);
	createColumn(THA_SPAWN, 50);
	createColumn(THA_HEALTH, 50);
	createColumn(THA_AMMO, 50);
	createColumn(THA_ARMOR, 50);

	masterList = things;
	thingList = masterList->exportToList();
	thingComp = compare_id;
	thingList->sort(*thingComp);

	/** Add (wxWindow *window, int proportion=0, int flag=0, int border=0) */
	/** wxTOP wxBOTTOM wxLEFT wxRIGHT wxALL wxEXPAND wxALIGN_* */

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_NEW,"Add"), 0, wxALL, 5);
	delButton = new wxButton(this,BUTTON_DELETE,"Delete");
	bottomSizer->Add(delButton, 0, wxALL, 5);
	bottomSizer->Add(10,10,1);
	if (onlyDone) {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Done"), 0, wxALL, 5);
	} else {
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 5);
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Save"), 0, wxALL, 5);
	}

	//Adding the main parts to the layout:
	topSizer->Add(listCtrl, 1, wxEXPAND|wxALL, 5);
	topSizer->Add(bottomSizer, 0, wxEXPAND|wxALL, 5);

	showList();

	//Implementing the sizer:
    SetSizer(topSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size
}

GuiThingDialog::~GuiThingDialog()
{
	delete thingList;
}

void GuiThingDialog::createColumn(thingAttrs ta, int w)
{
	wxListItem col;
	col.SetId(ta);
	col.SetText(thingAttrStr[ta]);
	col.SetWidth(w);
	listCtrl->InsertColumn(ta, col);
}

void GuiThingDialog::showList()
{
	listCtrl->DeleteAllItems();
	long n = -1;
	for (list<ThingDef*>::iterator it=thingList->begin(); it != thingList->end(); ++it) {
		n++;
		ThingDef* thing = *it;
		wxListItem li;
		li.SetText(wxString::Format("%i", thing->id));
		li.SetId(n);
		if (thing->modified) {
			wxFont f = li.GetFont();
			li.SetFont(f.Bold());
		}
		n = listCtrl->InsertItem(li);
		listCtrl->SetItem(n, 1, wxString::Format("%i", thing->dehacked));
		listCtrl->SetItem(n, 2, thing->name);
		listCtrl->SetItem(n, 3, thingCatStr[thing->cat]);
		listCtrl->SetItem(n, 4, wxString::Format("%i", thing->hp));
		if (thing->spawner) {
			listCtrl->SetItem(n, 5, "*");
		} else {
			listCtrl->SetItem(n, 5, "");
		}
		listCtrl->SetItem(n, 6, wxString::Format("%i", thing->health));
		listCtrl->SetItem(n, 7, wxString::Format("%i", thing->ammo));
		listCtrl->SetItem(n, 8, wxString::Format("%i", thing->armor));
	}
	if (thingIndex == -1) {
		delButton->Enable(false);
	} else {
		delButton->Enable(true);
		listCtrl->SetItemState(thingIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		listCtrl->EnsureVisible(thingIndex);
	}
}

ThingDef* GuiThingDialog::findThing(uint16_t id)
{
	for (list<ThingDef*>::iterator it=thingList->begin(); it != thingList->end(); ++it) {
		if ((*it)->id == id)
			return (*it);
	}
	return NULL;
}

ThingDef* GuiThingDialog::thingFromIndex(int index)
{
	ThingDef* thing = NULL;
	if (index > -1) {
		list<ThingDef*>::iterator it = thingList->begin();
		for (int i=0; i<thingIndex; i++)
			it++;
		thing = *it;
	}
	return thing;
}

int GuiThingDialog::indexFromThing(ThingDef* tf)
{
	long n = -1;
	for (list<ThingDef*>::iterator it=thingList->begin(); it != thingList->end(); ++it) {
		n++;
		if ((*it)->id == tf->id)
			return n;
	}
	return -1;
}

//Event table:
BEGIN_EVENT_TABLE(GuiThingDialog, wxDialog)
	EVT_LIST_ITEM_ACTIVATED(LVIEW_LIST, GuiThingDialog::itemActivated)
	EVT_LIST_ITEM_FOCUSED(LVIEW_LIST, GuiThingDialog::itemFocused)
	EVT_LIST_COL_CLICK(LVIEW_LIST, GuiThingDialog::colClicked)

	EVT_BUTTON(BUTTON_NEW, GuiThingDialog::addThing)
	EVT_BUTTON(BUTTON_DELETE, GuiThingDialog::removeThing)

    EVT_BUTTON(BUTTON_OK, GuiThingDialog::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiThingDialog::onCancel)
    EVT_CLOSE(GuiThingDialog::onClose)
END_EVENT_TABLE()

void GuiThingDialog::itemActivated(wxListEvent& event)
{
	//wxLogVerbose("Activated item %s", event.GetItem().GetText());
	thingIndex = event.GetItem().GetId();
	currentThing = thingFromIndex(thingIndex);
	delButton->Enable(true);

	ThingDef* thing = new ThingDef("");
	thing->copyFrom(currentThing);
	GuiThingDef* dialog = new GuiThingDef(this, wxDefaultPosition, thing);
	int result = dialog->ShowModal();
	dialog->Destroy();
	if (result == BUTTON_OK) {
		//Check if valid?
		currentThing->copyFrom(thing);
		currentThing->modified = true;
		//Update list
		listCtrl->SetItem(thingIndex, 0, wxString::Format("%i", currentThing->id));
		listCtrl->SetItem(thingIndex, 1, wxString::Format("%i", currentThing->dehacked));
		listCtrl->SetItem(thingIndex, 2, currentThing->name);
		listCtrl->SetItem(thingIndex, 3, thingCatStr[currentThing->cat]);
		listCtrl->SetItem(thingIndex, 4, wxString::Format("%i", currentThing->hp));
		if (currentThing->spawner) {
			listCtrl->SetItem(thingIndex, 5, "*");
		} else {
			listCtrl->SetItem(thingIndex, 5, "");
		}
		listCtrl->SetItem(thingIndex, 6, wxString::Format("%i", currentThing->health));
		listCtrl->SetItem(thingIndex, 7, wxString::Format("%i", currentThing->ammo));
		listCtrl->SetItem(thingIndex, 8, wxString::Format("%i", currentThing->armor));
		wxFont f = listCtrl->GetItemFont(thingIndex);
		listCtrl->SetItemFont(thingIndex, f.Bold());
	}
	delete thing;
}

void GuiThingDialog::itemFocused(wxListEvent& event)
{
	//wxLogVerbose("Item %i focused", event.GetItem().GetId());
	thingIndex = event.GetItem().GetId();
	currentThing = thingFromIndex(thingIndex);
	delButton->Enable(true);
}

void GuiThingDialog::colClicked(wxListEvent& event)
{
	if (event.GetColumn() == THA_ID)
		thingComp = compare_id;
	else if (event.GetColumn() == THA_DEH)
		thingComp = compare_deh;
	else if (event.GetColumn() == THA_NAME)
		thingComp = compare_name;
	else if (event.GetColumn() == THA_CAT)
		thingComp = compare_cat;
	else
		return;

	thingList->sort(*thingComp);
	if (currentThing != NULL)
		thingIndex = indexFromThing(currentThing);
	showList();
}

void GuiThingDialog::addThing(wxCommandEvent& event)
{
	ThingDef* thing = new ThingDef("");
	GuiThingDef* dialog = new GuiThingDef(this, wxDefaultPosition, thing);
	int result = dialog->ShowModal();
	dialog->Destroy();
	if (result == BUTTON_CANCEL) {
		delete thing;
	} else if (findThing(thing->id) != NULL) { //Must be unique ID
		wxMessageDialog* dlg = new wxMessageDialog(this, "This ID is already taken. A unique ID is needed to create a new thing.",
			"Duplicate ID", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
		delete thing;
	} else {
		currentThing = thing;
		currentThing->modified = true;
		thingList->push_back(currentThing);
		thingList->sort(*thingComp);
		thingIndex = indexFromThing(currentThing);
		showList();
	}
}

void GuiThingDialog::removeThing(wxCommandEvent& event)
{
	if (thingIndex>-1 && thingIndex<thingList->size()) {
		list<ThingDef*>::iterator it = thingList->begin();
		for (int i=0; i<thingIndex; i++)
			it++;
		thingList->erase(it);
		delete currentThing;
		currentThing = NULL;
		//thingList->sort(*thingComp);
		thingIndex = -1;
		showList();
	}
}

void GuiThingDialog::onOk(wxCommandEvent& event)
{
	masterList->importFromList(thingList);
	EndModal(BUTTON_OK);
}

void GuiThingDialog::onCancel(wxCommandEvent& event)
{
	masterList->importFromList(thingList);
	EndModal(BUTTON_CANCEL);
}

void GuiThingDialog::onClose(wxCloseEvent& event)
{
	masterList->importFromList(thingList);
	EndModal(BUTTON_CANCEL);
}

//*************************************************************
//************************ GuiThingDef ************************
//*************************************************************

GuiThingDef::GuiThingDef(wxWindow* parent, const wxPoint& pos, ThingDef* td)
: wxDialog(parent, -1, "Edit ThingDef", pos, wxSize(300,280), wxDEFAULT_DIALOG_STYLE),
currentThing(td)
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxSize inpSize=wxSize(50,20);

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	wxTextCtrl* txtCtrl = new wxTextCtrl(this,THA_ID,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_ID]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_ID]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(this,THA_DEH,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_DEH]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_DEH]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(this,THA_NAME,"",wxDefaultPosition,wxSize(120,20),0,
		wxTextValidator(wxFILTER_ALPHANUMERIC, &attrInput[THA_NAME]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_NAME]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,0);

	catChoice = new wxChoice(this, THA_CAT, wxDefaultPosition,
		wxDefaultSize, THING_END, thingCatStr);
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_CAT]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(catChoice,0);

	txtCtrl = new wxTextCtrl(this,THA_HP,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_HP]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_HP]),0);
	gridSizer->Add(txtCtrl,0);

	spawnerCheck = new wxCheckBox(this, THA_SPAWN, "");
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_SPAWN]),0);
	gridSizer->Add(spawnerCheck,0);

	txtCtrl = new wxTextCtrl(this,THA_HEALTH,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_HEALTH]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_HEALTH]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(this,THA_AMMO,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_AMMO]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_AMMO]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(this,THA_ARMOR,"",wxDefaultPosition,inpSize,0,
		wxTextValidator(wxFILTER_NUMERIC, &attrInput[THA_ARMOR]));
	gridSizer->Add(new wxStaticText(this,-1,thingAttrStr[THA_ARMOR]),0);
	gridSizer->Add(txtCtrl,0);

	topSizer->Add(gridSizer,0,wxALL,10);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"Set"), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

    writeThing();
}

GuiThingDef::~GuiThingDef()
{
}

void GuiThingDef::writeThing()
{
	attrInput[THA_ID] = wxIntToString(currentThing->id);
	attrInput[THA_DEH] = wxIntToString(currentThing->dehacked);
	attrInput[THA_NAME] = currentThing->name;
	catChoice->SetSelection(currentThing->cat);
	attrInput[THA_HP] = wxIntToString(currentThing->hp);
	spawnerCheck->SetValue(currentThing->spawner);
	attrInput[THA_HEALTH] = wxIntToString(currentThing->health);
	attrInput[THA_AMMO] = wxIntToString(currentThing->ammo);
	attrInput[THA_ARMOR] = wxIntToString(currentThing->armor);

	TransferDataToWindow();
}

void GuiThingDef::readThing()
{
	TransferDataFromWindow();
	long number;
	if (attrInput[THA_ID].ToLong(&number))
		currentThing->id = number;
	if (attrInput[THA_DEH].ToLong(&number))
		currentThing->dehacked = number;
	currentThing->name = attrInput[THA_NAME];
	currentThing->cat = ThingCat(catChoice->GetSelection());
	if (attrInput[THA_HP].ToLong(&number))
		currentThing->hp = number;
	currentThing->spawner = spawnerCheck->GetValue();
	if (attrInput[THA_HEALTH].ToLong(&number))
		currentThing->health = number;
	if (attrInput[THA_AMMO].ToLong(&number))
		currentThing->ammo = number;
	if (attrInput[THA_ARMOR].ToLong(&number))
		currentThing->armor = number;
}

//Event table:
BEGIN_EVENT_TABLE(GuiThingDef, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiThingDef::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiThingDef::onCancel)
    EVT_CLOSE(GuiThingDef::onClose)
END_EVENT_TABLE()

void GuiThingDef::onOk(wxCommandEvent& event)
{
	readThing();
	currentThing->modified = true;

	if (currentThing->name.length() == 0) {
		wxMessageDialog* dlg = new wxMessageDialog(this, "The thing definition must have a name.",
			"Missing name", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	} else if (currentThing->id == 0) {
		wxMessageDialog* dlg = new wxMessageDialog(this, "Non-zero ID needed.",
			"Missing ID", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		EndModal(BUTTON_OK);
	}
}

void GuiThingDef::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiThingDef::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
