#include "GuiViewSelect.h"

#include "GuiPersonDialogs.h"
#include "GuiTagDialogs.h"

//***************************************************************
//************************ GuiViewSelect ************************
//***************************************************************

GuiViewSelect::GuiViewSelect(wxWindow* parent, GuiViewListener* l)
: wxTreeCtrl(parent, LVIEW_TREE, wxDefaultPosition, wxDefaultSize,
			wxTR_SINGLE|wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS) //wxTR_HIDE_ROOT
{
	listener = l;
	dataViews = NULL;
	SetDropTarget(new GuiViewDropTarget(this));
	SetSizeHints(wxSize(40,40));
}

GuiViewSelect::~GuiViewSelect()
{
	removeDataManager();
}

void GuiViewSelect::populate(DataManager* dataMan, WadMapFields wadSort, WadMapFields mapSort)
{
    dataBase = dataMan;
	if (dataViews != NULL) {
		delete dataViews;
		DeleteAllItems();
	}
	dataViews = new map<wxTreeItemId, DataFilter*>();
	wxTreeItemId root = AddRoot("Data views");

	DataFilter* wadFilt = new DataFilter(FILTER_WAD);
	wadFilt->name = "All wads";
	wadFilt->sortField = wadSort;
	wxTreeItemId item = AppendItem(root, wadFilt->name);
	(*dataViews)[item] = wadFilt;
	defaultView = item;

	DataFilter* dv = new DataFilter(FILTER_MAP);
	dv->name = "All maps";
	dv->sortField = mapSort;
	item = AppendItem(root, dv->name);
	(*dataViews)[item] = dv;
	dataBase->initDataFilters(wadFilt, dv);

	//AuthorFilter "Map author"
	dv = new AuthorFilter(FILTER_MAP, 0);
	dv->name = "Map author...";
    mapAuthor = AppendItem(root, dv->name);
    (*dataViews)[mapAuthor] = dv;

	//TagFilter "Map style tag"
	dv = new TagFilter(FILTER_MAP, 0);
	dv->name = "Map style tag...";
	mapTag = AppendItem(root, dv->name);
    (*dataViews)[mapTag] = dv;

	wxTreeItemId newStuff = AppendItem(root, "New entries");
	dv = new DataIdFilter(FILTER_WAD, DFOP_MORE, dataBase->getWadIdEnd());
	dv->name = "New wads";
	item = AppendItem(newStuff, dv->name);
	(*dataViews)[item] = dv;
	dv = new DataIdFilter(FILTER_MAP, DFOP_MORE, dataBase->getMapIdEnd());
	dv->name = "New maps";
	item = AppendItem(newStuff, dv->name);
	(*dataViews)[item] = dv;

	wxTreeItemId wadFilters = AppendItem(root, "Wad filters");
	varWadFilters(wadFilters);
	iwadFilters(wadFilters, FILTER_WAD);
	engineFilters(wadFilters, FILTER_WAD);
	playStyleFilters(wadFilters, FILTER_WAD);
	yearFilters(wadFilters, FILTER_WAD);
	contentFilters(wadFilters, FILTER_WAD);
	ownStuffFilters(wadFilters, FILTER_WAD);

	wxTreeItemId mapFilters = AppendItem(root, "Map filters");
	varMapFilters(mapFilters);
	gameModeFilters(mapFilters);
	iwadFilters(mapFilters, FILTER_MAP);
	engineFilters(mapFilters, FILTER_MAP);
	playStyleFilters(mapFilters, FILTER_MAP);
	yearFilters(mapFilters, FILTER_MAP);
	contentFilters(mapFilters, FILTER_MAP);
	ownStuffFilters(mapFilters, FILTER_MAP);

	wadLists = AppendItem(root, "Wad lists");
	list<DataListFilter*>* views = dataBase->getWadLists();
	for (list<DataListFilter*>::iterator it=views->begin(); it != views->end(); ++it) {
		dv = *it;
		item = AppendItem(wadLists, dv->name);
		(*dataViews)[item] = dv;
	}
	mapLists = AppendItem(root, "Map lists");
	views = dataBase->getMapLists();
	for (list<DataListFilter*>::iterator it=views->begin(); it != views->end(); ++it) {
		dv = *it;
		item = AppendItem(mapLists, dv->name);
		(*dataViews)[item] = dv;
	}
	Expand(root);
}

void GuiViewSelect::removeDataManager()
{
	dataBase = NULL;
	if (dataViews != NULL) {
		//Delete DataFilter objects, except the lists from the database
		for (map<wxTreeItemId,DataFilter*>::iterator it = dataViews->begin(); it!=dataViews->end(); it++) {
			if (!it->second->isList())
				delete it->second;
		}
		delete dataViews;
		DeleteAllItems();
		dataViews = NULL;
	}
}

//Event table:
BEGIN_EVENT_TABLE(GuiViewSelect, wxTreeCtrl)
	EVT_TREE_SEL_CHANGED(LVIEW_TREE, onSelection)
	EVT_TREE_ITEM_MENU(LVIEW_TREE, onMenu)
	EVT_TREE_BEGIN_LABEL_EDIT(LVIEW_TREE, onBeginEdit)
	EVT_TREE_END_LABEL_EDIT(LVIEW_TREE, onEndEdit)
END_EVENT_TABLE()

void GuiViewSelect::onSelection(wxTreeEvent& event)
{
	map<wxTreeItemId,DataFilter*>::iterator it = dataViews->find(event.GetItem());
	if (it != dataViews->end()) {
		//Selected a view
		DataFilter* df = it->second;
		if (it->first == mapAuthor) {
            //Select author
            AuthorFilter* af = dynamic_cast<AuthorFilter*>(df);
            dataBase->openAuthorTextFile();
            GuiPersonList* personDialog = new GuiPersonList(this, wxDefaultPosition, dataBase, DDMODE_VIEW, af->val);
            if (personDialog->ShowModal() == BUTTON_OK) {
				af->val = personDialog->getSelectedPerson()->dbid;
				af->name = "Map author: " + dataBase->getAuthorString(personDialog->getSelectedPerson());
				af->isChanged = true;
				SetItemText(mapAuthor, af->name);
            } else {
				df = NULL;
            }
            personDialog->Destroy();
            dataBase->closeAuthorTextFile();
		} else if (it->first == mapTag) {
		    //Select tag
		    TagFilter* tf = dynamic_cast<TagFilter*>(df);
            GuiTagList* tagDialog = new GuiTagList(this, wxDefaultPosition, dataBase, DDMODE_VIEW, tf->val);
            int id = tagDialog->ShowModal();
            tagDialog->Destroy();
            if (id > 0) {
                tf->val = id;
                tf->name = "Map style tag: " + wxString(dataBase->getTagString(id));
                tf->isChanged = true;
                SetItemText(mapTag, tf->name);
            } else {
                df = NULL;
            }
		}
		if (df != NULL){
            dataBase->changeDataFilter(df);
            listener->onViewSelected(df);
		}
	}
}

void GuiViewSelect::onMenu(wxTreeEvent& event)
{
	wxTreeItemId id = event.GetItem();
	if (id==wadLists || id==mapLists) {
		wxMenu menu;
		menu.Append(10, "New list");
		int result = GetPopupMenuSelectionFromUser(menu, event.GetPoint());
		if (result==10) {
			if (id==wadLists) {
				DataFilter* df = new DataListFilter(FILTER_WAD_LIST);
				df->name = "New list";
				//df->sortField = dataBase->getWadFilter()->sortField;
				wxTreeItemId item = AppendItem(wadLists, df->name);
				(*dataViews)[item] = df;
				dataBase->addDataFilter(df);
			} else { //mapLists
				DataFilter* df = new DataListFilter(FILTER_MAP_LIST);
				df->name = "New list";
				//df->sortField = dataBase->getMapFilter()->sortField;
				wxTreeItemId item = AppendItem(mapLists, df->name);
				(*dataViews)[item] = df;
				dataBase->addDataFilter(df);
			}
		}
	} else {
		map<wxTreeItemId,DataFilter*>::iterator it = dataViews->find(id);
		if (it == dataViews->end()) return;
		DataFilter* dfilter = it->second;
		if ((dfilter->type==FILTER_WAD_LIST) || (dfilter->type==FILTER_MAP_LIST)) {
			wxMenu menu;
			menu.Append(10, "Delete list");
			int result = GetPopupMenuSelectionFromUser(menu, event.GetPoint());
			if (result==10) {
				bool wasSelected = (IsSelected(id));
				Delete(id);
				dataViews->erase(it);
				if (wasSelected) //Change selection
					SelectItem(defaultView, true);
				dataBase->removeDataFilter(dfilter);
				delete dfilter;
			}
		}
	}
}

void GuiViewSelect::onBeginEdit(wxTreeEvent& event)
{
	//Can only edit name of user-made lists
	map<wxTreeItemId,DataFilter*>::iterator it = dataViews->find(event.GetItem());
	if (it != dataViews->end()) {
		DataFilter* df = it->second;
		if (df->type==FILTER_WAD || df->type==FILTER_MAP)
			event.Veto();
	} else {
		event.Veto();
	}
}

void GuiViewSelect::onEndEdit(wxTreeEvent& event)
{
	if (event.IsEditCancelled())
		return;
	map<wxTreeItemId,DataFilter*>::iterator it = dataViews->find(event.GetItem());
	if (it != dataViews->end()) {
		DataFilter* df = it->second;
		df->name = event.GetLabel();
		//Save views?
	}
}

bool GuiViewSelect::onDragDropped(wxCoord x, wxCoord y, const wxString &data)
{
	int flags;
	wxTreeItemId id = HitTest(wxPoint(x,y), flags);
	if (!id.IsOk())
		return false;
	map<wxTreeItemId,DataFilter*>::iterator it = dataViews->find(id);
	if (it == dataViews->end())
		return false;
	DataFilter* df = it->second;
	if ((df->type!=FILTER_WAD_LIST) && (df->type!=FILTER_MAP_LIST))
		return false;
	wxString* rest = new wxString();
	bool result = false;
	if (df->type==FILTER_WAD_LIST) {
		if (data.StartsWith("WAD:", rest)) {
			unsigned long lng(0);
			if (rest->ToULong(&lng)) {
				DataListFilter* dlv = dynamic_cast<DataListFilter*>(df);
				dlv->addEntry(lng);
				result = true;
			}
		}
	} else if (df->type==FILTER_MAP_LIST) {
		if (data.StartsWith("MAP:", rest)) {
			unsigned long lng(0);
			if (rest->ToULong(&lng)) {
				DataListFilter* dlv = dynamic_cast<DataListFilter*>(df);
				dlv->addEntry(lng);
				result = true;
			}
		} else if (data.StartsWith("WAD:", rest)) {
			unsigned long lng(0);
			rest->ToULong(&lng);
			WadEntry* we = dataBase->findWad(lng);
			if (we != NULL) {
				DataListFilter* dlv = dynamic_cast<DataListFilter*>(df);
				for (int i=0; i<we->numberOfMaps; i++)
					dlv->addEntry(we->mapPointers[i]->dbid);
				result = true;
			}
		}
	}
	delete rest;
	if (result) dataBase->dataFilterModified();
	return result;
}

void GuiViewSelect::addFilter(const wxTreeItemId& parent, DataFilter* dv, wxString name)
{
	dv->name = name;
	wxTreeItemId item = AppendItem(parent, dv->name);
	(*dataViews)[item] = dv;
}

void GuiViewSelect::iwadFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId iwadGroup = AppendItem(root, "Iwad");
	ComboDataFilter* cdf = new ComboDataFilter(type);
	cdf->addFilter(new IwadFilter(type, DFOP_MORE, IWAD_CUSTOM));
	cdf->addFilter(new IwadFilter(type, DFOP_LESS, IWAD_HERETIC));
	addFilter(iwadGroup, cdf, "Any Doom");
	for (unsigned char i=IWAD_DOOM; i<=IWAD_STRIFE; i++) {
		addFilter(iwadGroup, new IwadFilter(type, DFOP_EQUALS, i), iwadNames[i]);
	}
	addFilter(iwadGroup, new IwadFilter(type, DFOP_EQUALS, IWAD_CUSTOM), iwadNames[IWAD_CUSTOM]);
}

void GuiViewSelect::engineFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId engineGroup = AppendItem(root, "Engine");
	for (unsigned char i=DENG_CUSTOM; i<DENG_END; i++) {
		addFilter(engineGroup, new EngineFilter(type, DFOP_EQUALS, i), engineNames[i]);
	}
	ComboDataFilter* cdf = new ComboDataFilter(type);
	cdf->addFilter(new EngineFilter(type, DFOP_MORE, DENG_CUSTOM));
	cdf->addFilter(new EngineFilter(type, DFOP_LESS, DENG_BOOM));
	addFilter(engineGroup, cdf, "Vanilla (org+LR)");

	cdf = new ComboDataFilter(type);
	cdf->addFilter(new EngineFilter(type, DFOP_MORE, DENG_CUSTOM));
	cdf->addFilter(new EngineFilter(type, DFOP_LESS, DENG_LEGACY));
	addFilter(engineGroup, cdf, "Vanilla+Boom");

	cdf = new ComboDataFilter(type);
	cdf->addFilter(new EngineFilter(type, DFOP_MORE, DENG_LEGACY));
	cdf->addFilter(new EngineFilter(type, DFOP_LESS, DENG_END));
	addFilter(engineGroup, cdf, "Any ZDoom");
}

void GuiViewSelect::playStyleFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId psGroup = AppendItem(root, "Play style");
	for (unsigned char i=0; i<3; i++) {
		addFilter(psGroup, new PlayStyleFilter(type, DFOP_EQUALS, i), playStyles[i]);
	}
}

void GuiViewSelect::yearFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId yearGroup = AppendItem(root, "Year");
	int cyear = wxDateTime::GetCurrentYear();
	for (uint16_t y=1994; y<=cyear; y++) {
		addFilter(yearGroup, new YearFilter(type, DFOP_EQUALS, y), wxIntToString(y));
	}
}

void GuiViewSelect::contentFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId cGroup = AppendItem(root, "Content");
	uint16_t all = WF_SPRITES|WF_TEX|WF_GFX|WF_COLOR|WF_SOUND|WF_MUSIC
			|WF_DEHBEX|WF_THINGS|WF_SCRIPT|WF_GLNODES;
	addFilter(cGroup, new WadFlagsInvFilter(type, all), "None (pure stock)");
	addFilter(cGroup, new WadFlagsFilter(type, WF_SPRITES), wadMapLabels[WAD_WF_SPRITES]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_TEX), wadMapLabels[WAD_WF_TEX]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_GFX), wadMapLabels[WAD_WF_GFX]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_COLOR), wadMapLabels[WAD_WF_COLOR]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_SOUND), wadMapLabels[WAD_WF_SOUND]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_MUSIC), wadMapLabels[WAD_WF_MUSIC]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_DEHBEX), wadMapLabels[WAD_WF_DEHBEX]);
	addFilter(cGroup, new WadFlagsInvFilter(type, WF_DEHBEX), "Not "+wadMapLabels[WAD_WF_DEHBEX]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_THINGS), wadMapLabels[WAD_WF_THINGS]);
	addFilter(cGroup, new WadFlagsInvFilter(type, WF_THINGS), "Not "+wadMapLabels[WAD_WF_THINGS]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_SCRIPT), wadMapLabels[WAD_WF_SCRIPT]);
	addFilter(cGroup, new WadFlagsInvFilter(type, WF_SCRIPT), "Not "+wadMapLabels[WAD_WF_SCRIPT]);
	addFilter(cGroup, new WadFlagsFilter(type, WF_GLNODES), wadMapLabels[WAD_WF_GLNODES]);
	addFilter(cGroup, new WadFlagsInvFilter(type, WF_GLNODES), "Not "+wadMapLabels[WAD_WF_GLNODES]);
}

void GuiViewSelect::gameModeFilters(const wxTreeItemId& root)
{
	wxTreeItemId modeGroup = AppendItem(root, "Game modes");
	addFilter(modeGroup, new SinglePlayerFilter(FILTER_MAP, DFOP_MORE, 2), "Single player");
	addFilter(modeGroup, new CooperativeFilter(FILTER_MAP, DFOP_MORE, 2), "Made for Coop");
	addFilter(modeGroup, new CooperativeFilter(FILTER_MAP, DFOP_MORE, 1), "Coop should work");

	ComboDataFilter* cdf = new ComboDataFilter(FILTER_MAP);
	cdf->addFilter(new DeathmatchFilter(FILTER_MAP, DFOP_MORE, 2));
	cdf->addFilter(new SinglePlayerFilter(FILTER_MAP, DFOP_LESS, 2));
	addFilter(modeGroup, cdf, "Deathmatch only");

	addFilter(modeGroup, new DeathmatchFilter(FILTER_MAP, DFOP_LESS, 3), "Not deathmatch");

	for (int i=1; i<3; i++)
		addFilter(modeGroup, new OtherModeFilter(FILTER_MAP, DFOP_EQUALS, i), otherGameModes[i]);
}

void GuiViewSelect::ownStuffFilters(const wxTreeItemId& root, unsigned char type)
{
	wxTreeItemId cGroup = AppendItem(root, "Personal");
	addFilter(cGroup, new OwnRatingFilter(type,DFOP_LESS,101), "Rated (own)");
	addFilter(cGroup, new OwnRatingFilter(type,DFOP_MORE,100), "Not rated (own)");
	if (type == FILTER_WAD) {
		addFilter(cGroup, new OwnFlagsFilter(type, OF_HAVEFILE), "Have file");
		addFilter(cGroup, new OwnFlagsInvFilter(type, OF_HAVEFILE), "Don't have file");
	} else { //FILTER_MAP
		addFilter(cGroup, new PlayedFilter(type,DFOP_LESS,1), "Not played");
		addFilter(cGroup, new PlayedFilter(type,DFOP_MORE,0), "Played");
		addFilter(cGroup, new PlayedFilter(type,DFOP_MORE,1), "Won");
		addFilter(cGroup, new DifficultyFilter(type,DFOP_EQUALS,1), difficultyRatings[1]);
		addFilter(cGroup, new DifficultyFilter(type,DFOP_EQUALS,2), difficultyRatings[2]);
		addFilter(cGroup, new DifficultyFilter(type,DFOP_EQUALS,3), difficultyRatings[3]);
		addFilter(cGroup, new DifficultyFilter(type,DFOP_EQUALS,4), difficultyRatings[4]);
		addFilter(cGroup, new DifficultyFilter(type,DFOP_EQUALS,0), "No diff.rating");
		addFilter(cGroup, new PlayTimeFilter(type,DFOP_MORE,0), "Has Play time");
	}
}

void GuiViewSelect::varWadFilters(const wxTreeItemId& root)
{
	addFilter(root, new WadFlagsFilter(FILTER_WAD, WF_IWAD), "Iwads");
	addFilter(root, new WadFlagsInvFilter(FILTER_WAD, WF_IWAD), "Pwads");
	addFilter(root, new MapCountFilter(FILTER_WAD, DFOP_EQUALS, 1), "Single-map");
	addFilter(root, new MapCountFilter(FILTER_WAD, DFOP_MORE, 1), "Multi-map");
	addFilter(root, new ExtraFilesFilter(FILTER_WAD), "Multiple files");
}

void GuiViewSelect::varMapFilters(const wxTreeItemId& root)
{
	addFilter(root, new MapCountFilter(FILTER_MAP, DFOP_EQUALS, 1), "Single-map wad");
	addFilter(root, new BasedOnFilter(FILTER_MAP), "Based on other");
	addFilter(root, new MapFlagsFilter(FILTER_MAP, MF_MORESPAWN), "Excessive spawning");
	addFilter(root, new MapFlagsFilter(FILTER_MAP, MF_VOODOO), "Voodoo dolls");
	addFilter(root, new MapFlagsFilter(FILTER_MAP, MF_UNKNOWN), "Unknown items");
	addFilter(root, new AreaFilter(FILTER_MAP, DFOP_MORE, 0.0), "With Area");
	addFilter(root, new AreaFilter(FILTER_MAP, DFOP_LESS, 0.001), "Without Area");
}


//*******************************************************************
//************************ GuiViewDropTarget ************************
//*******************************************************************

GuiViewDropTarget::GuiViewDropTarget(GuiViewSelect* owner)
{
	this->owner = owner;
}

GuiViewDropTarget::~GuiViewDropTarget()
{
}

bool GuiViewDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString &data)
{
	return owner->onDragDropped(x,y,data);
}
