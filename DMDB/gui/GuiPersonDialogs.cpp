/*
* GuiPersonDialogs implementation
*/

#include "GuiPersonDialogs.h"

//***************************************************************
//************************ GuiPersonList ************************
//***************************************************************

GuiPersonList::GuiPersonList(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, DoomDialogMode m, int dbid)
: wxDialog(parent, -1, "People", pos, wxSize(300,400), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	dataBase = dataMan;
	mode = m;
	selectMode = (dbid>-1);
	groupDialog = NULL;
	searchChars = 0;

	//Create name list:
	listCtrl = new wxListCtrl(this, LPERS_LIST, wxDefaultPosition, wxSize(150,260), //wxDefaultSize,
			wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL); //TODO: |wxLC_VIRTUAL
	listCtrl->InsertColumn(0, "");
	if (refreshList() == -1)
		authorIndex = -1;
	else if (dbid>0)
		authorIndex = dataBase->getAuthorIndex(dbid);
	else
		authorIndex = 0;

	//Create text fields:
	for (int i=0; i<PERSONLABELS; i++)
		personLabels[i] = new wxStaticText(this, -1, " ");
	personDescription = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(150,150), wxTE_MULTILINE|wxTE_READONLY);

	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	searchCtrl = new wxSearchCtrl(this,SEARCH_FIELD,"",wxDefaultPosition,wxDefaultSize);//wxSize(100,20)
	searchCtrl->ShowCancelButton(true);
    topSizer->Add(searchCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(listCtrl, 1, wxEXPAND|wxALL, 8);

	//Right side - author info:
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(personLabels[0], 0, wxALL, 4);
	rightSizer->Add(personLabels[1], 0, wxALL, 4);
	rightSizer->Add(personLabels[2], 0, wxALL, 4);
	rightSizer->Add(personLabels[3], 0, wxALL, 4);
	rightSizer->AddSpacer(4);
	rightSizer->Add(new wxStaticText(this,-1,"Notes:"), 0, wxALL, 4);
	rightSizer->Add(personDescription, 1, wxEXPAND|wxALL, 4);
	mainSizer->Add(rightSizer, 1, wxEXPAND|wxALL, 4);

	//Buttons (New/Edit/Ok) are the last part:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (mode == DDMODE_VIEW) {
		buttons[0] = NULL;
		buttons[1] = NULL;
		buttons[2] = NULL;
	} else {
		buttons[0] = new wxButton(this,BUTTON_NEW,"New");
		bottomSizer->Add(buttons[0], 0, wxALL, 5);
		buttons[1] = new wxButton(this,BUTTON_GROUP,"New Group");
		bottomSizer->Add(buttons[1], 0, wxALL, 5);
		buttons[2] = new wxButton(this,BUTTON_EDIT,"Edit");
		bottomSizer->Add(buttons[2], 0, wxALL, 5);
	}
	if (selectMode) {
		buttons[3] = new wxButton(this,BUTTON_CANCEL,"None");
		bottomSizer->Add(buttons[3], 0, wxALL, 5);
	} else {
		buttons[3] = NULL;
	}
	buttons[4] = new wxButton(this,BUTTON_OK,"Ok");
	bottomSizer->Add(buttons[4], 0, wxALL, 5);

	//Adding the main parts to the layout:
	topSizer->Add(mainSizer, 1, wxEXPAND);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

	selectPerson(authorIndex);
}

GuiPersonList::~GuiPersonList()
{
	if (searchChars > 2)
		dataBase->clearAuthorFilter();
}

void GuiPersonList::selectPerson(long index)
{
	authorIndex = index;
	if (authorIndex > -1) {
		listCtrl->SetItemState(authorIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		listCtrl->EnsureVisible(authorIndex);
	}
	showPerson();
}

void GuiPersonList::showPerson()
{
	AuthorEntry* person = dataBase->getAuthor(authorIndex);
	unsigned int cnt = 0;
	if (person == NULL) {
		personLabels[0]->SetLabel("No selection");
		personLabels[1]->SetLabel("");
		personLabels[2]->SetLabel("");
		personDescription->SetValue("");
	} else if (person->type > 0) { //Group
		personLabels[0]->SetLabel("Group");
		personLabels[1]->SetLabel("Name: "+person->alias1);
		personLabels[2]->SetLabel("");
		personDescription->SetValue(dataBase->getAuthorText(person));
		cnt = dataBase->getMapAuthorCount(person->dbid);
	} else {
		wxString nameStrings[3];
		nameStrings[0] = person->getName();
		nameStrings[1] = person->alias1;
		nameStrings[2] = person->alias2;
		for (int i=0; i<3; i++)
			if (nameStrings[i]=="") nameStrings[i]="-";
		personLabels[0]->SetLabel("Name: "+nameStrings[0]);
		personLabels[1]->SetLabel("Alias: "+nameStrings[1]);
		personLabels[2]->SetLabel("Other alias: "+nameStrings[2]);
		personDescription->SetValue(dataBase->getAuthorText(person));
		cnt = dataBase->getMapAuthorCount(person->dbid);
	}
	if (cnt == 0)
		personLabels[3]->SetLabel("No maps in database");
	else
		personLabels[3]->SetLabel("Maps in database: " + wxIntToString(cnt));
}

long GuiPersonList::refreshList()
{
	listCtrl->DeleteAllItems();
	list<AuthorEntry*>* authors = dataBase->getAuthorList();
	long n = -1;
	//wxLogVerbose("Showing list of %i people", authors->size());
	for (list<AuthorEntry*>::iterator it=authors->begin(); it != authors->end(); ++it) {
		n++;
		AuthorEntry* auth = *it;
		listCtrl->InsertItem(n, dataBase->getAuthorString(auth));
	}
	return n;
}

AuthorEntry* GuiPersonList::getSelectedPerson()
{
	return dataBase->getAuthor(authorIndex);
}

//Event table:
BEGIN_EVENT_TABLE(GuiPersonList, wxDialog)
	EVT_SIZE(GuiPersonList::onResize)
	EVT_LIST_ITEM_ACTIVATED(LPERS_LIST, GuiPersonList::itemActivated)
	EVT_LIST_ITEM_FOCUSED(LPERS_LIST, GuiPersonList::itemFocused)
	EVT_LIST_ITEM_RIGHT_CLICK(LPERS_LIST, GuiPersonList::rightClick)
	EVT_TEXT(SEARCH_FIELD, GuiPersonList::onSearchText)

	EVT_BUTTON(BUTTON_NEW, GuiPersonList::onNew)
	EVT_BUTTON(BUTTON_GROUP, GuiPersonList::onNewGroup)
	EVT_BUTTON(BUTTON_EDIT, GuiPersonList::onEdit)
    EVT_BUTTON(BUTTON_OK, GuiPersonList::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiPersonList::onNone)
    EVT_CLOSE(GuiPersonList::onClose)
END_EVENT_TABLE()

void GuiPersonList::onResize(wxSizeEvent& event)
{
	wxSize s = listCtrl->GetClientSize();
	listCtrl->SetColumnWidth(0, s.GetWidth());
	event.Skip();
}

void GuiPersonList::itemActivated(wxListEvent& event)
{
	authorIndex = event.GetItem().GetId();
	if (groupDialog != NULL) {
		AuthorEntry* person = dataBase->getAuthor(authorIndex);
		if (person->type==0) { //Not allow group in group?
			groupDialog->addPerson(person, dataBase->getAuthorString(person));
		}
	} else if (selectMode) {
		EndModal(BUTTON_OK);
	} else if (mode != DDMODE_VIEW) {
		editDialog();
	}
}

void GuiPersonList::itemFocused(wxListEvent& event)
{
	authorIndex = event.GetItem().GetId();
	showPerson();
}

void GuiPersonList::rightClick(wxListEvent& event)
{
	//itemFocused is also called, so authorIndex is set
	if (mode == DDMODE_VIEW)
		return;
	wxMenu menu;
	menu.Append(10, "Edit");
	menu.Append(11, "Merge");
	menu.Append(12, "Delete");
	int result = GetPopupMenuSelectionFromUser(menu, event.GetPoint());
	if (result==10) {
		editDialog();
	} else if (result==11) {
		onMerge();
	} else if (result==12) {
		onDelete();
	}
}

void GuiPersonList::onSearchText(wxCommandEvent& event)
{
	wxString sstr = searchCtrl->GetValue();
    if (sstr.Length() == 0) {
		if (searchChars > 2) {
			dataBase->clearAuthorFilter();
			refreshList();
		}
    } else if (sstr.Length() > 2) {
    	bool update = (sstr.Length() > searchChars);
        dataBase->filterAuthorList(sstr, update);
        refreshList();
    } else {
    	if (searchChars > 2) {
			dataBase->clearAuthorFilter();
			refreshList();
		}
        int i = dataBase->getAuthorIndex(sstr);
        if (i > -1) {
            selectPerson(i);
        } else {
            listCtrl->EnsureVisible(0);
        }
    }
    searchChars = sstr.Length();
}

void GuiPersonList::onNew(wxCommandEvent& event)
{
	AuthorEntry* person = new AuthorEntry(0);
	wxString notes("");
	GuiPersonEdit* personDialog = new GuiPersonEdit(this, wxDefaultPosition, person, &notes);
	if (personDialog->ShowModal() > 0) {
		AuthorEntry* old = dataBase->findAuthor(person->namef, person->namel, person->alias1);
		if (old != NULL) {
			wxMessageDialog* dlg = new wxMessageDialog(this,
				"A person with the same name or alias is already in the database. Are you sure you wish to add the new entry?",
				"Person exist", wxYES_NO);
			int res = dlg->ShowModal();
			dlg->Destroy();
			if (res == wxID_NO) {
				delete person;
				selectPerson(dataBase->getAuthorIndex(old->dbid));
				personDialog->Destroy();
				return;
			}
		}
		dataBase->addAuthor(person, notes);
		refreshList();
		selectPerson(dataBase->getAuthorIndex(person->dbid));
		dataBase->saveAuthors();
	} else {
		delete person;
	}
	personDialog->Destroy();
}

void GuiPersonList::onNewGroup(wxCommandEvent& event)
{
	for (int i=1; i<5; i++) {
		if (buttons[i]!=NULL) buttons[i]->Enable(false);
	}
	AuthorGroupEntry* group = new AuthorGroupEntry(0);
	wxString notes("");
	groupDialog = new GuiGroupEdit(this, wxDefaultPosition, group, notes, dataBase, this);
	groupDialog->Show();
}

void GuiPersonList::groupEditDone(int changed)
{
	AuthorGroupEntry* group = groupDialog->getAuthorGroup();
	wxString notes = groupDialog->getAuthorNotes();
	if (changed > 0) { //Group was changed
		if (group->dbid == 0) {
			//New group
			AuthorEntry* old = dataBase->findAuthor("", "", group->alias1);
			if (old != NULL) {
				wxMessageDialog* dlg = new wxMessageDialog(this,
					"A group or person with the same name is already in the database. Are you sure you wish to add the new entry?",
					"Name exist", wxYES_NO);
				int res = dlg->ShowModal();
				dlg->Destroy();
				if (res == wxID_NO) {
					delete group;
					selectPerson(dataBase->getAuthorIndex(old->dbid));
					closeGroupDialog();
					return;
				}
			}
			dataBase->addAuthor(group, notes);
		} else {
			//Changes to existing group
			if (changed & ENTRY_TEXT)
				dataBase->authorTextModified(group, notes);
			if (changed & ENTRY_CORE)
				dataBase->authorModified(group);
		}
		refreshList();
		selectPerson(dataBase->getAuthorIndex(group->dbid));
		dataBase->saveAuthors();

	} else if (group->dbid == 0) {
		delete group;
	}
	closeGroupDialog();
}

void GuiPersonList::closeGroupDialog()
{
	groupDialog->Destroy();
	groupDialog = NULL;
	for (int i=1; i<5; i++) {
		if (buttons[i]!=NULL) buttons[i]->Enable(true);
	}
}

void GuiPersonList::editDialog()
{
	AuthorEntry* person = dataBase->getAuthor(authorIndex);
	if (person!=NULL && person->type>0) {
		//Group
		wxString notes = dataBase->getAuthorText(person);
		AuthorGroupEntry* group = dynamic_cast<AuthorGroupEntry*>(person);
		for (int i=1; i<5; i++) {
			if (buttons[i]!=NULL) buttons[i]->Enable(false);
		}
		groupDialog = new GuiGroupEdit(this, wxDefaultPosition, group, notes, dataBase, this);
		groupDialog->Show();

	} else if (person != NULL) {
		wxString notes = dataBase->getAuthorText(person);
		GuiPersonEdit* personDialog = new GuiPersonEdit(this, wxDefaultPosition, person, &notes);
		int res = personDialog->ShowModal();
		personDialog->Destroy();
		if (res & ENTRY_TEXT)
			dataBase->authorTextModified(person, notes);
		if (res & ENTRY_CORE) {
			//Update person
			dataBase->authorModified(person);
			refreshList();
			selectPerson(dataBase->getAuthorIndex(person->dbid));
			dataBase->saveAuthors();
		} else if (res & ENTRY_TEXT) {
			selectPerson(dataBase->getAuthorIndex(person->dbid));
		}
    }
}

void GuiPersonList::onMerge()
{
	AuthorEntry* person = dataBase->getAuthor(authorIndex);
	if (person != NULL) {
		wxString name = dataBase->getAuthorString(person);
		wxMessageDialog* dlg = new wxMessageDialog(this,
			wxString::Format("Merge entry %s? If so, you will select a second entry to merge with (double-click or select and click OK in the next dialog). Entry %s will be deleted from the database, and any instances of it in map entries will be replaced by the second entry.",name,name),
			"Merge entries", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		int result = dlg->ShowModal();
		dlg->Destroy();
		if (result != wxID_YES)
			return;
		//Find person entry to merge with
		GuiPersonList* gpl = new GuiPersonList(this, wxDefaultPosition, dataBase, DDMODE_VIEW, 0);
		result = gpl->ShowModal();
		if (result != BUTTON_OK) {
			gpl->Destroy();
			return;
		}
		AuthorEntry* other = gpl->getSelectedPerson();
		gpl->Destroy();
		if (other == person) {
			wxMessageDialog* dlg = new wxMessageDialog(this, "Entry can't be merged with itself.",
				"Invalid selection", wxOK|wxICON_ERROR);
			dlg->ShowModal();
			dlg->Destroy();
		} else if (other != NULL) {
			dataBase->mergeAuthors(person, other);
			if (refreshList() == -1) authorIndex = -1;
			else authorIndex = 0;
			selectPerson(authorIndex);
			dataBase->saveWadsMaps(); //Save here, or later?
		}
	}
}

void GuiPersonList::onDelete()
{
	AuthorEntry* person = dataBase->getAuthor(authorIndex);
	if (person != NULL) {
		wxString name = dataBase->getAuthorString(person);
		wxMessageDialog* dlg = new wxMessageDialog(this,
			wxString::Format("Delete entry %s from database? It will be removed from map entries and the people list, with no option to undo.",name),
			"Delete entry", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		int result = dlg->ShowModal();
		dlg->Destroy();
		if (result == wxID_YES) {
			dataBase->deleteAuthor(person);
			if (refreshList() == -1) authorIndex = -1;
			else authorIndex = 0;
			selectPerson(authorIndex);
			dataBase->saveWadsMaps(); //Save here, or later?
		}
	}
}

void GuiPersonList::onEdit(wxCommandEvent& event)
{
	editDialog();
}

void GuiPersonList::onOk(wxCommandEvent& event)
{
	EndModal(BUTTON_OK);
}

void GuiPersonList::onNone(wxCommandEvent& event)
{
	authorIndex = -1;
	EndModal(BUTTON_OK);
}

void GuiPersonList::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

//***************************************************************
//************************ GuiPersonEdit ************************
//***************************************************************

GuiPersonEdit::GuiPersonEdit(wxWindow* parent, const wxPoint& pos,
			AuthorEntry* person, wxString* notes, bool show)
: wxDialog(parent, -1, "Person", pos, wxSize(500,300), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	author = person;
	authorNotes = notes;
	inputs[0] = author->namef;
	inputs[1] = author->namel;
	inputs[2] = author->alias1;
	inputs[3] = author->alias2;
	noteInput = *authorNotes;

	long textStyle = (show)? wxTE_READONLY: 0;
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(new wxStaticText(this,-1,"Name"),0,wxRIGHT|wxALIGN_CENTER_VERTICAL,5);
	wxTextCtrl* txtCtrl = new wxTextCtrl(this,0,"",wxDefaultPosition,wxSize(60,20),
		textStyle, wxTextValidator(wxFILTER_NONE, &inputs[0]));
	topSizer->Add(txtCtrl,0,wxRIGHT,10);
	txtCtrl = new wxTextCtrl(this,0,"",wxDefaultPosition,wxSize(60,20),
		textStyle, wxTextValidator(wxFILTER_NONE, &inputs[1]));
	topSizer->Add(txtCtrl,0,wxRIGHT,10);

	topSizer->Add(new wxStaticText(this,-1,"Alias"),0,wxRIGHT|wxALIGN_CENTER_VERTICAL,5);
	txtCtrl = new wxTextCtrl(this,1,"",wxDefaultPosition,wxSize(100,20),
		textStyle, wxTextValidator(wxFILTER_NONE, &inputs[2]));
	topSizer->Add(txtCtrl,0,wxRIGHT,10);

	topSizer->Add(new wxStaticText(this,-1,"Other alias"),0,wxRIGHT|wxALIGN_CENTER_VERTICAL,5);
	txtCtrl = new wxTextCtrl(this,2,"",wxDefaultPosition,wxSize(100,20),
		textStyle, wxTextValidator(wxFILTER_NONE, &inputs[3]));
	topSizer->Add(txtCtrl,0,wxRIGHT,0);

	mainSizer->Add(topSizer, 0, wxALL, 10);
	mainSizer->Add(new wxStaticText(this,-1,"Notes:"), 0, wxRIGHT|wxLEFT, 10);
	txtCtrl = new wxTextCtrl(this, 3, "", wxDefaultPosition, wxDefaultSize,
		textStyle|wxTE_MULTILINE, wxTextValidator(wxFILTER_NONE, &noteInput));
	mainSizer->Add(txtCtrl, 1, wxEXPAND|wxRIGHT|wxLEFT, 10);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (show) {
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Close"), 0, wxALL, 10);
	} else {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Save"), 0, wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	}
	mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER, 0);

	//Implementing the sizer:
    SetSizer(mainSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window
    //topSizer->Fit(this); //Resize window to match minimal size

    TransferDataToWindow();
}

//Event table:
BEGIN_EVENT_TABLE(GuiPersonEdit, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiPersonEdit::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiPersonEdit::onCancel)
    EVT_CLOSE(GuiPersonEdit::onClose)
END_EVENT_TABLE()

void GuiPersonEdit::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	if (inputs[0].Length()>0 || inputs[1].Length()>0 || inputs[2].Length()>0) {
		int ret = 0;
		if (!inputs[0].IsSameAs(author->namef)) {
			author->namef = inputs[0];
			ret = ENTRY_CORE;
		}
		if (!inputs[1].IsSameAs(author->namel)) {
			author->namel = inputs[1];
			ret = ENTRY_CORE;
		}
		if (!inputs[2].IsSameAs(author->alias1)) {
			author->alias1 = inputs[2];
			ret = ENTRY_CORE;
		}
		if (!inputs[3].IsSameAs(author->alias2)) {
			author->alias2 = inputs[3];
			ret = ENTRY_CORE;
		}
		if (!noteInput.IsSameAs(*authorNotes)) {
			(*authorNotes) = noteInput;
			ret |= ENTRY_TEXT;
		}
		EndModal(ret);
	} else {
		wxMessageDialog* dlg = new wxMessageDialog(this,
				"Either Name or Alias must be specified", "Invalid entry", wxOK);
		dlg->ShowModal();
		dlg->Destroy();
	}
}

void GuiPersonEdit::onCancel(wxCommandEvent& event)
{
	EndModal(0);
}

void GuiPersonEdit::onClose(wxCloseEvent& event)
{
	EndModal(0);
}

//**************************************************************
//************************ GuiGroupEdit ************************
//**************************************************************

GuiGroupEdit::GuiGroupEdit(wxWindow* parent, const wxPoint& pos, AuthorGroupEntry* group,
			wxString notes, DataManager* dataMan, GuiPersonList* ownr)
: wxDialog(parent, -1, "Group", pos, wxSize(500,300), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	owner = ownr;
	authorGroup = group;
	authorNotes = notes;
	nameInput = authorGroup->alias1;
	noteInput = authorNotes;

	long textStyle = (owner==NULL)? wxTE_READONLY: 0;
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	if (owner != NULL) {
		mainSizer->Add(new wxStaticText(this,-1,"Double-click people entries in the People dialog to add to the group.\nDouble-click entries in the group to remove."),
			0,wxALL,5);
	}

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	listBox = new wxListBox(this, LVIEW_LIST, wxDefaultPosition, wxDefaultSize); //wxSize(60,-1)
	topSizer->Add(listBox,1,wxEXPAND|wxALL,8);

	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(new wxStaticText(this,-1,"Group name:"),0,0,5);
	wxTextCtrl* txtCtrl = new wxTextCtrl(this,0,"",wxDefaultPosition,wxDefaultSize,//wxSize(100,20),
		textStyle, wxTextValidator(wxFILTER_NONE, &nameInput));
	rightSizer->Add(txtCtrl,0,wxEXPAND,10);

	rightSizer->Add(new wxStaticText(this,-1,"Notes:"), 0, 0, 10);
	txtCtrl = new wxTextCtrl(this, 3, "", wxDefaultPosition, wxDefaultSize,
		textStyle|wxTE_MULTILINE, wxTextValidator(wxFILTER_NONE, &noteInput));
	rightSizer->Add(txtCtrl, 1, wxEXPAND, 10);
	topSizer->Add(rightSizer,1,wxEXPAND|wxALL,8);

	mainSizer->Add(topSizer, 1, wxEXPAND);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (owner==NULL) {
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Close"), 0, wxALL, 10);
	} else {
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Save"), 0, wxALL, 10);
		//bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	}
	mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER, 0);

	//Implementing the sizer:
    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this); //Set minimal size for window
    mainSizer->Fit(this); //Resize window to match minimal size

    TransferDataToWindow();
    for (int i=0; i<authorGroup->type; i++)
		listBox->Append(dataMan->getAuthorString(authorGroup->pointers[i]));
	listChange = false;
}

void GuiGroupEdit::addPerson(AuthorEntry* person, wxString name)
{
	if (authorGroup->addEntry(person)) {
		listBox->AppendAndEnsureVisible(name);
		listChange = true;
	}
}

//Event table:
BEGIN_EVENT_TABLE(GuiGroupEdit, wxDialog)
	EVT_LISTBOX_DCLICK(LVIEW_LIST, GuiGroupEdit::onEntryRemove)
    EVT_BUTTON(BUTTON_OK, GuiGroupEdit::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiGroupEdit::onCancel)
    EVT_CLOSE(GuiGroupEdit::onClose)
END_EVENT_TABLE()

void GuiGroupEdit::onEntryRemove(wxCommandEvent& event)
{
	if (owner != NULL) {
		int index = event.GetSelection();
		authorGroup->removeEntry(index);
		listBox->Delete(index);
		listChange = true;
	}
}

void GuiGroupEdit::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	if (nameInput.Length() == 0) {
		wxMessageDialog* dlg = new wxMessageDialog(this,
				"Name must be specified", "Invalid group", wxOK);
		dlg->ShowModal();
		dlg->Destroy();
	} else if (authorGroup->type == 0) {
		wxMessageDialog* dlg = new wxMessageDialog(this,
				"The group must have at least one entry", "Invalid group", wxOK);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		int ret = (listChange)? ENTRY_CORE: 0;
		if (!nameInput.IsSameAs(authorGroup->alias1)) {
			authorGroup->alias1 = nameInput;
			ret = ENTRY_CORE;
		}
		if (!noteInput.IsSameAs(authorNotes)) {
			authorNotes = noteInput;
			ret |= ENTRY_TEXT;
		}
		if (owner != NULL)
			owner->groupEditDone(ret);
		else
			EndModal(0);
	}
}

void GuiGroupEdit::onCancel(wxCommandEvent& event)
{
	if (owner != NULL)
		owner->groupEditDone(0);
	else
		EndModal(0);
}

void GuiGroupEdit::onClose(wxCloseEvent& event)
{
	if (owner != NULL)
		owner->groupEditDone(0);
	else
		EndModal(0);
}
