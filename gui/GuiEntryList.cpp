#include "GuiEntryList.h"


GuiEntryList::GuiEntryList(wxWindow* parent, GuiEntryListener* l, ApplicationSettings* as)
: wxListCtrl(parent,LVIEW_LIST,wxDefaultPosition, wxDefaultSize,
			wxLC_REPORT|wxLC_VIRTUAL|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL),
			settings(as), dataBase(NULL), listener(l), dataType(-1), searchChars(0)
{
	fields[0] = NULL;
	fields[1] = NULL;
	SetSizeHints(wxSize(80,40));
}

GuiEntryList::~GuiEntryList()
{
	if (fields[0] != NULL) delete fields[0];
	if (fields[1] != NULL) delete fields[1];
}

void GuiEntryList::setDataManager(DataManager* dataMan)
{
	dataBase = dataMan;
	ClearAll();
	dataType = -1; //No current list
}

void GuiEntryList::loadListFields()
{
	EntryListFields* wadFields = settings->getWadFields();
	if (fields[0] != NULL) {
		if (fields[0]->isSame(wadFields)) {
			delete wadFields;
		} else {
			delete fields[0];
			fields[0] = wadFields;
			if (dataType==0) createList();
		}
	} else {
		fields[0] = wadFields;
	}

	EntryListFields* mapFields = settings->getMapFields();
	if (fields[1] != NULL) {
		if (fields[1]->isSame(mapFields)) {
			delete mapFields;
		} else {
			delete fields[1];
			fields[1] = mapFields;
			if (dataType==1) createList();
		}
	} else {
		fields[1] = mapFields;
	}
}

void GuiEntryList::saveListFields()
{
	if (dataType > -1) {
		readFields(dataType);
		if (fields[0]->modified)
			settings->setWadFields(fields[0]);
		fields[0]->modified = false;
		if (fields[1]->modified)
			settings->setMapFields(fields[1]);
		fields[1]->modified = false;
	}
}

void GuiEntryList::createList(int type)
{
	if (dataType > -1) { //Save current columns
		readFields(dataType);
		if (fields[dataType]->modified) {
			if (dataType==0)
				settings->setWadFields(fields[0]);
			else
				settings->setMapFields(fields[1]);
			fields[dataType]->modified = false;
		}
	}
	dataType = type;
	createList();
}

void GuiEntryList::createList()
{
	if (dataBase==NULL) return;
	ClearAll();
	//SetHeaderAttr(wxItemAttr(*wxBLACK, *wxLIGHT_GREY, *wxNORMAL_FONT));
	//Only in newer versions of wxWidgets?
	EntryListFields* fld = fields[dataType];
	WadMapFields sfield = (dataType==0)? dataBase->getWadFilter()->sortField: dataBase->getMapFilter()->sortField;
	for (int i=0; i<fld->fieldCount; i++) {
		wxListItem col;
		//col.SetId(i);
		if (fld->fields[i] == sfield)
			col.SetText(wxString("<")+wadMapLabels[fld->fields[i]]+">");
		else
			col.SetText(wadMapLabels[fld->fields[i]]);
		col.SetWidth(fld->width[i]);
		col.SetAlign(wxLIST_FORMAT_LEFT);
		//col.SetTextColour(*wxRED);
		//col.SetFont(*wxITALIC_FONT);
		InsertColumn(i, col);
	}
	if (!fld->order.IsEmpty())
		SetColumnsOrder(fld->order);
	refreshList();
}

void GuiEntryList::refreshList()
{
	unsigned int i;
	if (dataBase == NULL)
		i = 0;
	else if (dataType == 0)
		i = dataBase->getWadSize();
	else
		i = dataBase->getMapSize();
	SetItemCount(i);
	RefreshItems(0, i-1);
}

void GuiEntryList::onSearchText(wxString sstr)
{
	if (sstr.Length() == 0) {
		if (searchChars > 2) {
			if (dataType==0) dataBase->setTextFilter("", FILTER_WAD);
			else dataBase->setTextFilter("", FILTER_MAP);
			refreshList();
		}
    } else if (sstr.Length() > 2) {
    	bool update = (sstr.Length() > searchChars);
    	if (dataType==0) dataBase->setTextFilter(sstr, FILTER_WAD, update);
		else dataBase->setTextFilter(sstr, FILTER_MAP, update);
        refreshList();
    } else {
    	if (searchChars > 2) {
			if (dataType==0) dataBase->setTextFilter("", FILTER_WAD);
			else dataBase->setTextFilter("", FILTER_MAP);
			refreshList();
		}
        int i = (dataType==0)? dataBase->getWadIndex(sstr): dataBase->getMapIndex(sstr);
        if (i > -1) {
			EnsureVisible(i);
			SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        } else {
            EnsureVisible(0);
        }
    }
    searchChars = sstr.Length();
}

void GuiEntryList::readFields(int type)
{
	EntryListFields* fld = fields[type];
	for (int i=0; i<fld->fieldCount; i++)
		fld->setWidth(i, GetColumnWidth(i));
	fld->order = GetColumnsOrder();
}

wxString GuiEntryList::OnGetItemText(long item, long column) const
{
	if (dataType==0) {
		WadEntry* we = dataBase->getWad(item);
		return dataBase->getWadField(we, fields[0]->fields[column]);
	} else {
		MapEntry* me = dataBase->getMap(item);
		return dataBase->getMapField(me, fields[1]->fields[column]);
	}
}

wxListItemAttr* GuiEntryList::OnGetItemAttr(long item) const
{
	//Can have special style for rows
	return NULL;
}

//Event table:
BEGIN_EVENT_TABLE(GuiEntryList, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(LVIEW_LIST, GuiEntryList::itemActivated)
	EVT_LIST_ITEM_FOCUSED(LVIEW_LIST, GuiEntryList::itemFocused)
	EVT_LIST_ITEM_RIGHT_CLICK(LVIEW_LIST, GuiEntryList::rightClick)
	EVT_LIST_COL_CLICK(LVIEW_LIST, GuiEntryList::colClicked)
	//EVT_LIST_COL_END_DRAG(LVIEW_LIST, GuiEntryList::colDragged)
	EVT_LIST_BEGIN_DRAG(LVIEW_LIST, GuiEntryList::beginDrag)
END_EVENT_TABLE()

void GuiEntryList::itemActivated(wxListEvent& event)
{
	//wxLogVerbose("Activated item %i", event.GetIndex());
	if (dataType==0)
		listener->onWadActivated(event.GetIndex());
	else
		listener->onMapActivated(event.GetIndex());
}

void GuiEntryList::itemFocused(wxListEvent& event)
{
	//wxLogVerbose("Item %i focused", event.GetIndex());
	if (dataType==0)
		listener->onWadFocused(event.GetIndex());
	else
		listener->onMapFocused(event.GetIndex());
}

void GuiEntryList::rightClick(wxListEvent& event)
{
	wxMenu menu;
	if (dataType==0) {
		menu.Append(12, "Show wad");
		menu.Append(13, "Show wad+maps");
		if (dataBase->getWadFilter()->isList())
			menu.Append(10, "Remove from list");
		menu.Append(11, "Delete wad");
	} else {
		menu.Append(14, "Show map");
		menu.Append(15, "Show wad+maps");
		if (dataBase->getMapFilter()->isList())
			menu.Append(10, "Remove from list");
		menu.Append(16, "Delete map");
	}
	int result = GetPopupMenuSelectionFromUser(menu, event.GetPoint());
	if (result==10) {
		if (dataBase->removeFromListFilter(dataType, event.GetIndex()))
			refreshList();
	} else if (result==11) {
		wxMessageDialog* dlg = new wxMessageDialog(this,
				"Are you sure you wish to delete the wad entry from the database?",
				"Delete wad", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		result = dlg->ShowModal();
		dlg->Destroy();
		if (result==wxID_YES) {
			WadEntry* we = dataBase->getWad(event.GetIndex());
			dataBase->deleteWad(we, event.GetIndex());
			listener->onWadFocused(-1);
			dataBase->saveWadsMaps();
			refreshList();
		}
	} else if (result==12) {
		listener->onWadActivated(event.GetIndex(), 1);
	} else if (result==13) {
		listener->onWadActivated(event.GetIndex(), 2);
	} else if (result==14) {
		listener->onMapActivated(event.GetIndex(), 1);
	} else if (result==15) {
		listener->onMapActivated(event.GetIndex(), 2);
	} else if (result==16) {
		MapEntry* me = dataBase->getMap(event.GetIndex());
		wxString msg = (me->wadPointer->numberOfMaps == 1)?
				"Are you sure you wish to delete the map entry from the database? As it is the only map in the wad, the wad entry will also be deleted, along with text and images.":
				"Are you sure you wish to delete the map entry from the database? Text and images of the map will also be deleted.";
		wxMessageDialog* dlg = new wxMessageDialog(this, msg,
				"Delete map", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		result = dlg->ShowModal();
		dlg->Destroy();
		if (result==wxID_YES) {
			//TODO
			dataBase->deleteMap(me, event.GetIndex());
			listener->onMapFocused(-1);
			dataBase->saveWadsMaps();
			refreshList();
		}
	}
}

void GuiEntryList::colClicked(wxListEvent& event)
{
	int col = event.GetColumn();
	bool ok;
	WadMapFields oldField;
	bool rev;
	if (dataType==0) {
		oldField = dataBase->getWadFilter()->sortField;
		rev = dataBase->getWadFilter()->sortReverse;
	} else {
		oldField = dataBase->getMapFilter()->sortField;
		rev = dataBase->getMapFilter()->sortReverse;
	}
	int oldCol = fields[dataType]->findIndex(oldField);
	if (col == oldCol) rev = !rev;
	else rev = false;
	if (dataType==0) {
		ok = dataBase->setWadSort(fields[0]->fields[col], rev);
	} else {
		ok = dataBase->setMapSort(fields[1]->fields[col], rev);
	}
	if (ok) { //Changed sort column
		if (oldCol > -1) {
			wxListItem old = wxListItem();
			old.SetText(wadMapLabels[fields[dataType]->fields[oldCol]]);
			SetColumn(oldCol, old);
		}
		wxListItem item = wxListItem();
		if (rev)
			item.SetText(wxString(">")+wadMapLabels[fields[dataType]->fields[col]]+"<");
		else
			item.SetText(wxString("<")+wadMapLabels[fields[dataType]->fields[col]]+">");
		SetColumn(col, item);
		refreshList();
	}
	//item.SetWidth(fld->width[i]);
	//item.SetAlign(wxLIST_FORMAT_LEFT);
}

/*
void GuiEntryList::colDragged(wxListEvent& event)
{
	wxLogVerbose("colDragged column %i", event.GetColumn());
}*/

void GuiEntryList::beginDrag(wxListEvent& event)
{
	wxDropSource dragSource(this);
	if (dataType==0) {
		WadEntry* we = dataBase->getWad(event.GetIndex());
		if (we == NULL) return;
		wxString str("WAD:");
		str<<we->dbid;
		wxTextDataObject my_data(str);
		dragSource.SetData(my_data);
		wxDragResult result = dragSource.DoDragDrop(true);
	} else {
		MapEntry* me = dataBase->getMap(event.GetIndex());
		if (me == NULL) return;
		wxString str("MAP:");
		str<<me->dbid;
		wxTextDataObject my_data(str);
		dragSource.SetData(my_data);
		wxDragResult result = dragSource.DoDragDrop(true);
	}
}
