#include "GuiTagDialogs.h"

//************************************************************
//************************ GuiTagList ************************
//************************************************************

GuiTagList::GuiTagList(wxWindow* parent, const wxPoint& pos, DataManager* dataMan, DoomDialogMode m, int dbid)
: wxDialog(parent, -1, "Style tags", pos, wxSize(200,300), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	dataBase = dataMan;
	mode = m;
	selectMode = (dbid>-1);
	searchChars = 0;

	//Create name list:
	listCtrl = new wxListCtrl(this, LPERS_LIST, wxDefaultPosition, wxSize(180,200), //wxDefaultSize,
			wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL);
	listCtrl->InsertColumn(0, "");
	if (refreshList() == -1)
		tagIndex = -1;
	else if (dbid>0)
        tagIndex = dataBase->getTagIndex(dbid);
    else
		tagIndex = 0;

	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	searchCtrl = new wxSearchCtrl(this,SEARCH_FIELD,"",wxDefaultPosition,wxDefaultSize);//wxSize(100,20)
	searchCtrl->ShowCancelButton(true);
    topSizer->Add(searchCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);

	//List and buttons:
	if (mode == DDMODE_VIEW) {
		topSizer->Add(listCtrl, 1, wxEXPAND|wxALL, 8);
		topSizer->Add(new wxButton(this,BUTTON_OK,"Ok"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
	} else {
		wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
		mainSizer->Add(listCtrl, 2, wxEXPAND|wxALL, 8);
		wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
		rightSizer->Add(new wxButton(this,BUTTON_NEW,"New"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
		rightSizer->Add(new wxButton(this,BUTTON_EDIT,"Edit"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
		rightSizer->Add(new wxButton(this,BUTTON_MERGE,"Merge"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
		rightSizer->Add(new wxButton(this,BUTTON_DELETE,"Delete"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
		rightSizer->AddSpacer(18);
		rightSizer->Add(new wxButton(this,BUTTON_OK,"Ok"), 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 8);
		mainSizer->Add(rightSizer, 1, wxEXPAND, 0); //|wxALIGN_CENTER
		topSizer->Add(mainSizer, wxEXPAND);
	}

	//Implementing the sizer:
    SetSizer(topSizer);
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size

    selectTag(tagIndex);
}

GuiTagList::~GuiTagList()
{
	if (searchChars > 2)
		dataBase->clearTagFilter();
}

void GuiTagList::selectTag(int index)
{
	tagIndex = index;
	if (tagIndex > -1) {
        //wxLogVerbose("Scroll to selected tag index %i", tagIndex);
        listCtrl->SetItemState(tagIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        listCtrl->EnsureVisible(tagIndex);
	}
}

int GuiTagList::refreshList()
{
	listCtrl->DeleteAllItems();
	list<TagEntry*>* tags = dataBase->getTagList();
	long n = -1;
	//wxLogVerbose("Showing list of %i tags", tags->size());
	for (list<TagEntry*>::iterator it=tags->begin(); it != tags->end(); ++it) {
		n++;
		TagEntry* tag = *it;
		listCtrl->InsertItem(n, tag->tag);
	}
	return n;
}

//Event table:
BEGIN_EVENT_TABLE(GuiTagList, wxDialog)
	EVT_SIZE(GuiTagList::onResize)
	EVT_LIST_ITEM_ACTIVATED(LPERS_LIST, GuiTagList::itemActivated)
	EVT_LIST_ITEM_FOCUSED(LPERS_LIST, GuiTagList::itemFocused)
	EVT_TEXT(SEARCH_FIELD, GuiTagList::onSearchText)

	EVT_BUTTON(BUTTON_NEW, GuiTagList::onNew)
	EVT_BUTTON(BUTTON_EDIT, GuiTagList::onEdit)
	EVT_BUTTON(BUTTON_MERGE, GuiTagList::onMerge)
	EVT_BUTTON(BUTTON_DELETE, GuiTagList::onDelete)
    EVT_BUTTON(BUTTON_OK, GuiTagList::onOk)
    EVT_CLOSE(GuiTagList::onClose)
END_EVENT_TABLE()

void GuiTagList::onResize(wxSizeEvent& event)
{
	wxSize s = listCtrl->GetClientSize();
	listCtrl->SetColumnWidth(0, s.GetWidth());
	event.Skip();
}

void GuiTagList::itemActivated(wxListEvent& event)
{
	tagIndex = event.GetItem().GetId();
	if (selectMode) {
		TagEntry* tag = dataBase->getTagEntry(tagIndex);
		if (tag == NULL)
			EndModal(0);
		else
			EndModal(tag->dbid);
	} else if (mode != DDMODE_VIEW) {
		editDialog();
	}
}

void GuiTagList::itemFocused(wxListEvent& event)
{
	tagIndex = event.GetItem().GetId();
}

void GuiTagList::onSearchText(wxCommandEvent& event)
{
    wxString sstr = searchCtrl->GetValue();
    if (sstr.Length() == 0) {
		if (searchChars > 2) {
			dataBase->clearTagFilter();
			refreshList();
		}
    } else if (sstr.Length() > 2) {
    	bool update = (sstr.Length() > searchChars);
        dataBase->filterTagList(sstr, update);
        refreshList();
    } else {
    	if (searchChars > 2) {
			dataBase->clearTagFilter();
			refreshList();
		}
        int i = dataBase->getTagIndex(sstr);
        if (i > -1) {
            selectTag(i);
        } else {
            listCtrl->EnsureVisible(0);
        }
    }
    searchChars = sstr.Length();
}

void GuiTagList::onNew(wxCommandEvent& event)
{
	TagEntry* tag = new TagEntry(0, "");
	GuiTagEdit* tagDialog = new GuiTagEdit(this, wxDefaultPosition, tag);
	if (tagDialog->ShowModal() > 0) {
		TagEntry* old = dataBase->findTag(tag->tag);
		if (old != NULL) {
			wxMessageDialog* dlg = new wxMessageDialog(this,
				"The entered tag already exists", "Invalid entry", wxOK);
			dlg->ShowModal();
			dlg->Destroy();
			delete tag;
			selectTag(dataBase->getTagIndex(old->dbid));
		} else {
			dataBase->addTag(tag);
			refreshList();
			selectTag(dataBase->getTagIndex(tag->dbid));
			dataBase->saveTags();
		}
	} else {
		delete tag;
	}
	tagDialog->Destroy();
}

void GuiTagList::editDialog()
{
	TagEntry* tag = dataBase->getTagEntry(tagIndex);
	if (tag != NULL) {
		GuiTagEdit* tagDialog = new GuiTagEdit(this, wxDefaultPosition, tag);
		int res = tagDialog->ShowModal();
		tagDialog->Destroy();
		if (res > 0) {
			//Update tag
			dataBase->tagModified(tag);
			refreshList();
			selectTag(dataBase->getTagIndex(tag->dbid));
			dataBase->saveTags();
		}
    }
}

void GuiTagList::onEdit(wxCommandEvent& event)
{
	editDialog();
}

void GuiTagList::onMerge(wxCommandEvent& event)
{
	TagEntry* tag = dataBase->getTagEntry(tagIndex);
	if (tag != NULL) {
		wxMessageDialog* dlg = new wxMessageDialog(this,
			wxString::Format("Merge tag %s? If so, you will select a second tag to merge with (double-click or select and click OK in the next dialog). Tag %s will be deleted from the database, and any instances of it in map entries will be replaced by the second tag.",tag->tag,tag->tag),
			"Merge tags", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		int result = dlg->ShowModal();
		dlg->Destroy();
		if (result != wxID_YES)
			return;
		//Find tag to merge with
		GuiTagList* gtl = new GuiTagList(this, wxDefaultPosition, dataBase, DDMODE_VIEW, 0);
		result = gtl->ShowModal();
		gtl->Destroy();
		if (result == tag->dbid) {
			wxMessageDialog* dlg = new wxMessageDialog(this, "Tag can't be merged with itself.",
				"Invalid selection", wxOK|wxICON_ERROR);
			dlg->ShowModal();
			dlg->Destroy();
		} else if (result > 0) {
			dataBase->mergeTags(tag, result);
			if (refreshList() == -1) tagIndex = -1;
			else tagIndex = 0;
			selectTag(tagIndex);
			dataBase->saveTags();
			dataBase->saveWadsMaps(); //Save here, or later?
		}
	}
}

void GuiTagList::onDelete(wxCommandEvent& event)
{
	TagEntry* tag = dataBase->getTagEntry(tagIndex);
	if (tag != NULL) {
		wxMessageDialog* dlg = new wxMessageDialog(this,
			wxString::Format("Delete tag %s from database? It will be removed from map entries and the tag list, with no option to undo.",tag->tag),
			"Delete tag", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		int result = dlg->ShowModal();
		dlg->Destroy();
		if (result == wxID_YES) {
			dataBase->deleteTag(tag);
			if (refreshList() == -1) tagIndex = -1;
			else tagIndex = 0;
			selectTag(tagIndex);
			dataBase->saveTags();
			dataBase->saveWadsMaps(); //Save here, or later?
		}
	}
}

void GuiTagList::onOk(wxCommandEvent& event)
{
	TagEntry* tag = NULL;
	if (selectMode)
		tag = dataBase->getTagEntry(tagIndex);
	if (tag == NULL)
		EndModal(0);
	else
		EndModal(tag->dbid);
}

void GuiTagList::onClose(wxCloseEvent& event)
{
	EndModal(0);
}


//************************************************************
//************************ GuiTagEdit ************************
//************************************************************

GuiTagEdit::GuiTagEdit(wxWindow* parent, const wxPoint& pos, TagEntry* tag)
: wxDialog(parent, -1, "Enter tag", pos, wxSize(400,160), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER )
{
	tagPtr = tag;
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	if (tagPtr->tag.length() > 0) {
		mainSizer->Add(new wxStaticText(this,-1,"Note that changing an existing tag affects all instances of this tag.",
            wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END) ,0,wxALL,5);
	}
	input = tagPtr->tag;
	wxTextCtrl* txtCtrl = new wxTextCtrl(this,0,"",wxDefaultPosition,wxDefaultSize,0,
		wxTextValidator(wxFILTER_ASCII, &input));
	mainSizer->Add(txtCtrl,0,wxEXPAND|wxALL,10);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"Save"), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER, 0);

	//Implementing the sizer:
    SetSizer(mainSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window
    //topSizer->Fit(this); //Resize window to match minimal size

    TransferDataToWindow();
}

//Event table:
BEGIN_EVENT_TABLE(GuiTagEdit, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiTagEdit::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiTagEdit::onCancel)
    EVT_CLOSE(GuiTagEdit::onClose)
END_EVENT_TABLE()

void GuiTagEdit::onOk(wxCommandEvent& event)
{
	TransferDataFromWindow();
	if (input.Length()>0) {
		if (input.IsSameAs(tagPtr->tag))
			EndModal(0);
		else
			tagPtr->tag = input;
			EndModal(ENTRY_CORE);
	} else {
		wxMessageDialog* dlg = new wxMessageDialog(this,
				"Specify a tag, or Cancel", "Invalid entry", wxOK);
		dlg->ShowModal();
		dlg->Destroy();
	}
}

void GuiTagEdit::onCancel(wxCommandEvent& event)
{
	EndModal(0);
}

void GuiTagEdit::onClose(wxCloseEvent& event)
{
	EndModal(0);
}
