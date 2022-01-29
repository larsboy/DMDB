#include "GuiMapManager.h"
#include "GuiWadEntry.h"

GuiMapManager::GuiMapManager(DoomDialogMode m, DataManager* dm, bool hideUnused)
: dataBase(dm), mode(m), hide(hideUnused), parts(0), sizeMed(120,20), sizeSmall(60,20),
mapEntry(NULL), wadText(NULL), wadTextCtrl(NULL), playtimeInput(NULL),
mapPane(NULL), shotPane(NULL), basedLabel(NULL)
{
	canEdit = (mode!=DDMODE_VIEW);
	textStyle = (canEdit)? 0: wxTE_READONLY;
}

GuiMapManager::~GuiMapManager()
{
}

void GuiMapManager::setDataManager(DataManager* dm)
{
	dataBase = dm;
	if (dataBase == NULL)
		clearEntry();
}

wxSizer* GuiMapManager::createTitle(wxWindow* parent, const wxString& label)
{
	parts |= GMP_TITLE;
	wxSize sizeLong=wxSize(200,20);

	titleSizer = new wxFlexGridSizer(0,2,5,5);
	titleSizer->Add(new wxStaticText(parent,-1,label),0);
	titleSizer->AddSpacer(1);

	titleInput = new wxTextCtrl(parent,MAPSTR_TITLE,"",wxDefaultPosition,sizeLong,
		textStyle, wxTextValidator(wxFILTER_ASCII, &mapStrFields[MAPSTR_TITLE]));
	titleLabel = new wxStaticText(parent,-1,wadMapLabels[MAP_TITLE]);
	titleSizer->Add(titleLabel,0);
	titleSizer->Add(titleInput,0);

	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,MAPSTR_NAME,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
		wxTextValidator(wxFILTER_ASCII, &mapStrFields[MAPSTR_NAME]));
	titleSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_NAME]),0);
	titleSizer->Add(txtCtrl,0);

	mapFlagBoxes[6] = new wxCheckBox(parent, 0, "");
	mapFlagBoxes[6]->Enable(canEdit);
	titleSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_NOTLEVEL]),0);
	titleSizer->Add(mapFlagBoxes[6],0);

	return titleSizer;
}

wxSizer* GuiMapManager::createRefs(wxWindow* parent, char buttonFlags)
{
	parts |= GMP_REFS;
	wxString authButton = (canEdit)? "Select": "View";
	wxFlexGridSizer*  gridSizer = new wxFlexGridSizer(0,3,5,5);

	// Wad (not if no button and hide)
	if (((buttonFlags&GMB_WAD)!=0) || (!hide)) {
		wadTextCtrl = new wxTextCtrl(parent,MAPSTR_WAD,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
			wxTextValidator(wxFILTER_ASCII, &mapStrFields[MAPSTR_WAD]));
		gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_END_MAP_START]),0,wxALIGN_CENTER_VERTICAL);
		gridSizer->Add(wadTextCtrl,0,wxALIGN_CENTER_VERTICAL);
		if ((buttonFlags&GMB_WAD) == 0)
			gridSizer->AddSpacer(1);
		else
			gridSizer->Add(new wxButton(parent,BUTTON_WAD,"Wad entry",wxDefaultPosition,
				wxDefaultSize, wxBU_EXACTFIT),0);
	}

	// Authors:
	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,MAPSTR_AUTHOR1,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
		wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_AUTHOR1]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_AUTHOR1]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,0,wxALIGN_CENTER_VERTICAL);
	if ((buttonFlags&GMB_AUTHORS) == 0) {
		auth1Button = NULL;
		gridSizer->AddSpacer(1);
	} else {
		auth1Button = new wxButton(parent,BUTTON_AUTHOR1,authButton,wxDefaultPosition,
			wxDefaultSize, wxBU_EXACTFIT);
		gridSizer->Add(auth1Button,0);
	}

	txtCtrl = new wxTextCtrl(parent,MAPSTR_AUTHOR2,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
		wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_AUTHOR2]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_AUTHOR2]),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,0,wxALIGN_CENTER_VERTICAL);
	if ((buttonFlags&GMB_AUTHORS) == 0) {
		auth2Button = NULL;
		gridSizer->AddSpacer(1);
	} else {
		auth2Button = new wxButton(parent,BUTTON_AUTHOR2,authButton,wxDefaultPosition,
			wxDefaultSize, wxBU_EXACTFIT);
		gridSizer->Add(auth2Button,0);
	}

	// basedOn map:
	if (((buttonFlags&GMB_BASEDON)!=0) || (!hide)) {
		txtCtrl = new wxTextCtrl(parent,MAPSTR_BASEDON,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
			wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_BASEDON]));
		basedLabel = new wxStaticText(parent,-1,wadMapLabels[MAP_BASEDON]);
		gridSizer->Add(basedLabel,0,wxALIGN_CENTER_VERTICAL);
		gridSizer->Add(txtCtrl,0,wxALIGN_CENTER_VERTICAL);
		if ((buttonFlags&GMB_BASEDON) == 0) {
			basedButton = NULL;
			gridSizer->AddSpacer(1);
		} else {
			basedButton = new wxButton(parent,BUTTON_MAP,authButton,wxDefaultPosition,
				wxDefaultSize, wxBU_EXACTFIT);
			gridSizer->Add(basedButton,0);
		}
	}
	return gridSizer;
}

wxSizer* GuiMapManager::createModes(wxWindow* parent, bool unspecified)
{
	parts |= GMP_MODES;
	int choices = (unspecified)? 5: 4;
	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,"Game modes:"),0);
	gridSizer->AddSpacer(1);

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_SINGLE]),0);
	if (canEdit) {
		mapChoice[0] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,choices,gameModeSupport);
		gridSizer->Add(mapChoice[0],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,MAPSTR_SINGLE,"",wxDefaultPosition,sizeMed,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_SINGLE])),0);
	}

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_COOP]),0);
	if (canEdit) {
		mapChoice[1] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,choices,gameModeSupport);
		gridSizer->Add(mapChoice[1],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,MAPSTR_COOP,"",wxDefaultPosition,sizeMed,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_COOP])),0);
	}

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_DM]),0);
	if (canEdit) {
		mapChoice[2] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,choices,gameModeSupport);
		gridSizer->Add(mapChoice[2],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,MAPSTR_DM,"",wxDefaultPosition,sizeMed,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_DM])),0);
	}

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MODE]),0);
	if (canEdit) {
		mapChoice[3] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,3,otherGameModes);
		gridSizer->Add(mapChoice[3],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,MAPSTR_MODE,"",wxDefaultPosition,sizeMed,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &mapStrFields[MAPSTR_MODE])),0);
	}
	return gridSizer;
}

wxBoxSizer* GuiMapManager::createTags(wxWindow* parent)
{
	parts |= GMP_TAGS;
	wxBoxSizer* middleSizer = new wxBoxSizer(wxVERTICAL);
	middleSizer->Add(new wxStaticText(parent,-1,"Style tags:"),0,wxBOTTOM,5);
	listBox = new wxListBox(parent, LVIEW_LIST, wxDefaultPosition, wxSize(80,120));
	middleSizer->Add(listBox,0,0,0);

	if (canEdit) {
		wxBoxSizer* tagSizer = new wxBoxSizer(wxHORIZONTAL);
		tagSizer->Add(new wxButton(parent,BUTTON_TAGADD,"+",wxDefaultPosition,
			wxDefaultSize,wxBU_EXACTFIT), 0, wxALL, 2);
		tagSizer->Add(new wxButton(parent,BUTTON_TAGDEL,"-",wxDefaultPosition,
			wxDefaultSize,wxBU_EXACTFIT), 0, wxALL, 2);
		middleSizer->Add(tagSizer,0,wxBOTTOM,10);
	} else {
		middleSizer->AddSpacer(10);
	}
	return middleSizer;
}

wxSizer* GuiMapManager::createStats(wxWindow* parent)
{
	parts |= GMP_STATS;
	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,"Stats:"),0);
	gridSizer->AddSpacer(1);

	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,MAPSTR_LINEDEFS,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_LINEDEFS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_LINEDEFS]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_SECTORS,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_SECTORS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_SECTORS]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_THINGS,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_THINGS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_THINGS]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_SECRETS,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_SECRETS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_SECRETS]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_AREA,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_AREA]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_AREA]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_LINEDENS,"",wxDefaultPosition,sizeSmall,
		wxTE_READONLY,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_LINEDENS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_LINEDEF_DENS]),0);
	gridSizer->Add(txtCtrl,0);

	mapFlagBoxes[2] = new wxCheckBox(parent, 0, "");
	mapFlagBoxes[2]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_DIFFSET]),0);
	gridSizer->Add(mapFlagBoxes[2],0);

	mapFlagBoxes[3] = new wxCheckBox(parent, 0, "");
	mapFlagBoxes[3]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_VOODOO]),0);
	gridSizer->Add(mapFlagBoxes[3],0);

	mapFlagBoxes[4] = new wxCheckBox(parent, 0, "");
	mapFlagBoxes[4]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_UNKNOWN]),0);
	gridSizer->Add(mapFlagBoxes[4],0);
	return gridSizer;
}

wxSizer* GuiMapManager::createOwn(wxWindow* parent, bool unspecified)
{
	int playedCh = (unspecified)? 6: 5;
	parts |= GMP_OWN;
	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,"Personal:"),0);
	gridSizer->AddSpacer(1);

	mapChoice[6] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,
		playedCh, playedLabels);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_PLAYED]),0);
	gridSizer->Add(mapChoice[6],0);

	mapChoice[4] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,
		12, rating10);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_OWNRATING]),0);
	gridSizer->Add(mapChoice[4],0);

	mapChoice[5] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,
		5, difficultyRatings);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_DIFFICULTY]),0);
	gridSizer->Add(mapChoice[5],0);

	playtimeInput = new wxTextCtrl(parent,MAPSTR_PLAYTIME,"",wxDefaultPosition,sizeSmall,0,
		wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_PLAYTIME]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_PLAYTIME]),0);
	gridSizer->Add(playtimeInput,0);

	//Disabled until there is content
	playtimeInput->SetEditable(false);
	mapChoice[4]->Enable(false);
	mapChoice[5]->Enable(false);
	mapChoice[6]->Enable(false);
	return gridSizer;
}

wxSizer* GuiMapManager::createGameplay(wxWindow* parent)
{
	parts |= GMP_GAMEPLAY;
	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,"Skill 3 stats:"),0);
	gridSizer->AddSpacer(1);

	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,MAPSTR_ENEMIES,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_ENEMIES]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_ENEMIES]),0);
	gridSizer->Add(txtCtrl,0);

	mapFlagBoxes[0] = new wxCheckBox(parent, 0, "");
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_SPAWN]),0);
	gridSizer->Add(mapFlagBoxes[0],0);
	mapFlagBoxes[0]->Enable(canEdit);

	mapFlagBoxes[1] = new wxCheckBox(parent, 0, "");
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_MF_MORESPAWN]),0);
	gridSizer->Add(mapFlagBoxes[1],0);
	mapFlagBoxes[1]->Enable(canEdit);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_TOTALHP,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_TOTALHP]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_TOTALHP]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_AMMORAT,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_AMMORAT]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_AMMORAT]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_HEALTHRAT,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_HEALTHRAT]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_HEALTHRAT]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_ARMORRAT,"",wxDefaultPosition,sizeSmall,
		textStyle,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_ARMORRAT]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_ARMORRAT]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_ENEMYDENS,"",wxDefaultPosition,sizeSmall,
		wxTE_READONLY,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_ENEMYDENS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_ENEMY_DENS]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,MAPSTR_HPDENS,"",wxDefaultPosition,sizeSmall,
		wxTE_READONLY,wxTextValidator(wxFILTER_NUMERIC, &mapStrFields[MAPSTR_HPDENS]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[MAP_HP_DENS]),0);
	gridSizer->Add(txtCtrl,0);
	return gridSizer;
}

wxTextCtrl* GuiMapManager::createText(wxWindow* parent)
{
	parts |= GMP_TEXT;
	return new wxTextCtrl(parent, 3, "", wxDefaultPosition, wxDefaultSize,
		textStyle|wxTE_MULTILINE, wxTextValidator(wxFILTER_NONE, &noteInput));
}

GuiImagePanel* GuiMapManager::createMapPane(wxWindow* parent)
{
	mapPane = new GuiImagePanel(parent, false, false);
	mapPane->setNoImageText("No map image found.");
	return mapPane;
}

GuiImagePanel* GuiMapManager::createShotPane(wxWindow* parent)
{
	shotPane = new GuiImagePanel(parent, true, true);
	shotPane->setNoImageText("Drag and drop screenshot file here.\nDouble-click to select file.\nRight-click to paste from clipboard.");
	return shotPane;
}

void GuiMapManager::setMapEntry(MapEntry* me)
{
	mapEntry = me;
	if (parts&GMP_TEXT)
		wadText = dataBase->getWadText(mapEntry->wadPointer);
}

void GuiMapManager::writeTagList()
{
	listBox->Clear();
	for (int i=0; i<MAXTAGS; i++) {
		if (mapEntry->tags[i] > 0)
			listBox->Append(dataBase->getTagString(mapEntry->tags[i]));
	}
}

void GuiMapManager::writeEntry()
{
	bool singleMap = ((mapEntry->wadPointer==NULL) || (mapEntry->wadPointer->numberOfMaps==1));
	if (parts&GMP_TITLE) {
		if (singleMap && hide) {
			titleSizer->Show(titleLabel, false);
			titleSizer->Show(titleInput, false);
			titleSizer->Layout();
		} else {
			titleSizer->Show(titleLabel, true);
			titleSizer->Show(titleInput, true);
			titleSizer->Layout();
			mapStrFields[MAPSTR_TITLE] = mapEntry->title;
			if (singleMap)
				titleInput->Enable(false);
			else
				titleInput->Enable(true);
		}
		mapStrFields[MAPSTR_NAME] = mapEntry->name;
		mapFlagBoxes[6]->SetValue(mapEntry->flags&MF_NOTLEVEL);
	}
	if (parts&GMP_REFS) {
		if (wadTextCtrl != NULL)
			mapStrFields[MAPSTR_WAD] = mapEntry->wadPointer->getName();
		mapStrFields[MAPSTR_AUTHOR1] =
			(mapEntry->author1==NULL? "": dataBase->getAuthorString(mapEntry->author1));
		if (auth1Button != NULL)
			auth1Button->Enable(mode!=DDMODE_VIEW || mapEntry->author1!=NULL);
		mapStrFields[MAPSTR_AUTHOR2] =
			(mapEntry->author2==NULL? "": dataBase->getAuthorString(mapEntry->author2));
		if (auth2Button != NULL)
			auth2Button->Enable(mode!=DDMODE_VIEW || mapEntry->author2!=NULL);
		MapEntry* basedMap = (mapEntry->basedOn==0)? NULL: dataBase->findMap(mapEntry->basedOn);
		if (basedMap == NULL) {
			mapStrFields[MAPSTR_BASEDON] = "";
			if ((mode==DDMODE_VIEW) && (basedButton!=NULL))
				basedButton->Enable(false);
		} else {
			//Always include wad?
			mapStrFields[MAPSTR_BASEDON] = basedOnString(mapEntry, basedMap);
			if (basedButton!=NULL) basedButton->Enable(true);
		}
		if (basedLabel != NULL) {
			if (mapEntry->flags&MF_SAMEAS)
				basedLabel->SetLabel("Same as");
			else
				basedLabel->SetLabel("Based on");
		}
	}
	if (parts&GMP_MODES) {
		if (mode == DDMODE_VIEW) {
			mapStrFields[MAPSTR_SINGLE] = dataBase->getMapField(mapEntry,MAP_SINGLE);
			mapStrFields[MAPSTR_COOP] = dataBase->getMapField(mapEntry,MAP_COOP);
			mapStrFields[MAPSTR_DM] = dataBase->getMapField(mapEntry,MAP_DM);
			mapStrFields[MAPSTR_MODE] = dataBase->getMapField(mapEntry,MAP_MODE);
		} else {
			mapChoice[0]->SetSelection(mapEntry->singlePlayer);
			mapChoice[1]->SetSelection(mapEntry->cooperative);
			mapChoice[2]->SetSelection(mapEntry->deathmatch);
			mapChoice[3]->SetSelection(mapEntry->otherMode);
		}
	}
	if (parts&GMP_STATS) {
		mapStrFields[MAPSTR_LINEDEFS] = wxIntToString(mapEntry->linedefs);
		mapStrFields[MAPSTR_SECTORS] = wxIntToString(mapEntry->sectors);
		mapStrFields[MAPSTR_THINGS] = wxIntToString(mapEntry->things);
		mapStrFields[MAPSTR_SECRETS] = wxIntToString(mapEntry->secrets);
		mapStrFields[MAPSTR_AREA] = wxDoubleToString(mapEntry->area, 3);
		mapStrFields[MAPSTR_LINEDENS] = wxDoubleToString(mapEntry->linedefDensity(), 2);
		mapFlagBoxes[2]->SetValue(mapEntry->flags&MF_DIFFSET);
		mapFlagBoxes[3]->SetValue(mapEntry->flags&MF_VOODOO);
		mapFlagBoxes[4]->SetValue(mapEntry->flags&MF_UNKNOWN);
	}
	if (parts&GMP_OWN) {
		mapStrFields[MAPSTR_PLAYTIME] = wxIntToString(mapEntry->playTime);
		mapChoice[4]->SetSelection(dataBase->scaleRating(mapEntry->ownRating));
		mapChoice[5]->SetSelection(mapEntry->difficulty);
		mapChoice[6]->SetSelection(mapEntry->played);
		playtimeInput->SetEditable(true);
		mapChoice[4]->Enable(true);
		mapChoice[5]->Enable(true);
		mapChoice[6]->Enable(true);
	}
	if (parts&GMP_GAMEPLAY) {
		mapStrFields[MAPSTR_ENEMIES] = wxIntToString(mapEntry->enemies);
		mapStrFields[MAPSTR_TOTALHP] = wxIntToString(mapEntry->totalHP);
		mapStrFields[MAPSTR_HEALTHRAT] = wxDoubleToString(mapEntry->healthRatio, 2);
		mapStrFields[MAPSTR_ARMORRAT] = wxDoubleToString(mapEntry->armorRatio, 2);
		mapStrFields[MAPSTR_AMMORAT] = wxDoubleToString(mapEntry->ammoRatio, 2);
		mapStrFields[MAPSTR_ENEMYDENS] = wxDoubleToString(mapEntry->enemyDensity(), 2);
		mapStrFields[MAPSTR_HPDENS] = wxDoubleToString(mapEntry->hitpointDensity(), 2);
		mapFlagBoxes[0]->SetValue(mapEntry->flags&MF_SPAWN);
		mapFlagBoxes[1]->SetValue(mapEntry->flags&MF_MORESPAWN);
	}
	if (parts&GMP_TEXT) {
		if (singleMap)
			noteInput = wadText->getText(WADTEXT_DESC);
		else
			noteInput = wadText->getText(mapEntry->name);
	}
}

void GuiMapManager::readEntry()
{
	long number;
	double doubl;
	bool singleMap = ((mapEntry->wadPointer==NULL) || (mapEntry->wadPointer->numberOfMaps==1));
	if (mode != DDMODE_VIEW) {
		unsigned char flg = mapEntry->flags;
		if (parts&GMP_TITLE) {
			if (!singleMap && (mapEntry->title.compare(mapStrFields[MAPSTR_TITLE]))) {
				mapEntry->title = mapStrFields[MAPSTR_TITLE];
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			//MAPSTR_NAME is read-only
			if (mapFlagBoxes[6]->GetValue()) flg |= MF_NOTLEVEL;
			else flg &= ~MF_NOTLEVEL;
		}
		if (parts&GMP_MODES) {
			if (mapChoice[0]->GetSelection() != mapEntry->singlePlayer) {
				mapEntry->singlePlayer = mapChoice[0]->GetSelection();
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapChoice[1]->GetSelection() != mapEntry->cooperative) {
				mapEntry->cooperative = mapChoice[1]->GetSelection();
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapChoice[2]->GetSelection() != mapEntry->deathmatch) {
				mapEntry->deathmatch = mapChoice[2]->GetSelection();
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapChoice[3]->GetSelection() != mapEntry->otherMode) {
				mapEntry->otherMode = mapChoice[3]->GetSelection();
				mapEntry->ownFlags |= OF_MAINMOD;
			}
		}
		if (parts&GMP_STATS) {
			if (mapStrFields[MAPSTR_LINEDEFS].ToLong(&number) && number!=mapEntry->linedefs) {
				mapEntry->linedefs = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_SECTORS].ToLong(&number) && number!=mapEntry->sectors) {
				mapEntry->sectors = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_THINGS].ToLong(&number) && number!=mapEntry->things) {
				mapEntry->things = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_SECRETS].ToLong(&number) && number!=mapEntry->secrets) {
				mapEntry->secrets = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_AREA].compare(wxDoubleToString(mapEntry->area,3))) {
				mapStrFields[MAPSTR_AREA].ToCDouble(&doubl);
				mapEntry->area = doubl;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapFlagBoxes[2]->GetValue()) flg |= MF_DIFFSET;
			else flg &= ~MF_DIFFSET;
			if (mapFlagBoxes[3]->GetValue()) flg |= MF_VOODOO;
			else flg &= ~MF_VOODOO;
			if (mapFlagBoxes[4]->GetValue()) flg |= MF_UNKNOWN;
			else flg &= ~MF_UNKNOWN;
		}
		if (parts&GMP_GAMEPLAY) {
			if (mapStrFields[MAPSTR_ENEMIES].ToLong(&number) && number!=mapEntry->enemies) {
				mapEntry->enemies = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_TOTALHP].ToLong(&number) && number!=mapEntry->totalHP) {
				mapEntry->totalHP = number;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_HEALTHRAT].compare(wxDoubleToString(mapEntry->healthRatio,2))) {
				mapStrFields[MAPSTR_HEALTHRAT].ToCDouble(&doubl);
				mapEntry->healthRatio = doubl;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_ARMORRAT].compare(wxDoubleToString(mapEntry->armorRatio,2))) {
				mapStrFields[MAPSTR_ARMORRAT].ToCDouble(&doubl);
				mapEntry->armorRatio = doubl;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapStrFields[MAPSTR_AMMORAT].compare(wxDoubleToString(mapEntry->ammoRatio,2))) {
				mapStrFields[MAPSTR_AMMORAT].ToCDouble(&doubl);
				mapEntry->ammoRatio = doubl;
				mapEntry->ownFlags |= OF_MAINMOD;
			}
			if (mapFlagBoxes[0]->GetValue()) flg |= MF_SPAWN;
			else flg &= ~MF_SPAWN;
			if (mapFlagBoxes[1]->GetValue()) flg |= MF_MORESPAWN;
			else flg &= ~MF_MORESPAWN;
		}
		if (parts&GMP_TEXT) {
			if (singleMap) {
				wadText->setText(WADTEXT_DESC, noteInput);
			} else {
				wadText->setText(mapEntry->name, noteInput);
			}
		}

		if (mapEntry->flags != flg) {
			mapEntry->flags = flg;
			mapEntry->ownFlags |= OF_MAINMOD;
		}
	}

	// Personal data (always editable):
	if (parts&GMP_OWN) {
		if (mapStrFields[MAPSTR_PLAYTIME].ToLong(&number) && number!=mapEntry->playTime) {
			mapEntry->playTime = number;
			mapEntry->ownFlags |= OF_OWNMOD;
		}
		unsigned char rat = dataBase->ratingScale(mapChoice[4]->GetSelection());
		if (rat != mapEntry->ownRating) {
			mapEntry->ownRating = rat;
			mapEntry->ownFlags |= OF_OWNMOD;
			if (singleMap && (mapEntry->wadPointer!=NULL)) {
				mapEntry->wadPointer->ownRating = rat;
				mapEntry->wadPointer->ownFlags |= OF_OWNMOD;
				dataBase->wadModified(mapEntry->wadPointer);
			}
		}
		if (mapChoice[5]->GetSelection() != mapEntry->difficulty) {
			mapEntry->difficulty = mapChoice[5]->GetSelection();
			mapEntry->ownFlags |= OF_OWNMOD;
		}
		if (mapChoice[6]->GetSelection() != mapEntry->played) {
			mapEntry->played = mapChoice[6]->GetSelection();
			mapEntry->ownFlags |= OF_OWNMOD;
		}
	}

	dataBase->mapModified(mapEntry);
	if (wadText!=NULL && wadText->changed)
		dataBase->saveWadText(wadText);
}

void GuiMapManager::showImages()
{
	if (mapPane != NULL) {
		wxString imgFile = dataBase->getMapImgFolder()
			+ wxFILE_SEP_PATH + mapEntry->fileName() + ".png";
		mapPane->loadImage(imgFile, wxBITMAP_TYPE_PNG);
	}
	if (shotPane != NULL) {
		wxString imgFile = dataBase->getScreenshotFolder()
			+ wxFILE_SEP_PATH + mapEntry->fileName() + ".png";
		shotPane->loadImage(imgFile, wxBITMAP_TYPE_PNG);
	}
}

void GuiMapManager::clearEntry()
{
	//So far only complete for view mode (only disables input for private fields)
	mapEntry = NULL;
	wadText = NULL;
	for (int i=0; i<MAPSTR_END; i++)
		mapStrFields[i] = "";
	if (parts&GMP_TAGS) {
		listBox->Clear();
	}
	if (parts&GMP_TITLE) {
		mapFlagBoxes[6]->SetValue(false);
	}
	if (parts&GMP_MODES) {
		if (mode != DDMODE_VIEW) {
			mapChoice[0]->SetSelection(0);
			mapChoice[1]->SetSelection(0);
			mapChoice[2]->SetSelection(0);
			mapChoice[3]->SetSelection(0);
		}
	}
	if (parts&GMP_STATS) {
		mapFlagBoxes[2]->SetValue(false);
		mapFlagBoxes[3]->SetValue(false);
		mapFlagBoxes[4]->SetValue(false);
	}
	if (parts&GMP_OWN) {
		mapChoice[4]->SetSelection(11);
		mapChoice[5]->SetSelection(0);
		mapChoice[6]->SetSelection(0);
		playtimeInput->SetEditable(false);
		mapChoice[4]->Enable(false);
		mapChoice[5]->Enable(false);
		mapChoice[6]->Enable(false);
	}
	if (parts&GMP_GAMEPLAY) {
		mapFlagBoxes[0]->SetValue(false);
		mapFlagBoxes[1]->SetValue(false);
	}
	if (parts&GMP_TEXT) {
		noteInput = "";
	}
	if (mapPane != NULL)
		mapPane->clearImage();
	if (shotPane != NULL)
		shotPane->clearImage();
}

void GuiMapManager::openWad(wxWindow* parent)
{
	if (mode > DDMODE_VIEW)
		readEntry();
	ListWrapper<WadEntry*>* wrapper = new ListWrapper<WadEntry*>();
	wrapper->add(mapEntry->wadPointer);
	wrapper->reset();

	wxConfigBase* configObject = wxConfigBase::Get();
	int x = (int) configObject->Read("/WadEntry/x", 300);
	int y = (int) configObject->Read("/WadEntry/y", 200);
	int w = (int) configObject->Read("/WadEntry/w", 900);
	int h = (int) configObject->Read("/WadEntry/h", 600);
	GuiWadEntry* wadDialog = new GuiWadEntry(parent, wxPoint(x,y), wxSize(w,h),
		wrapper, dataBase, mode, false);
	int result = wadDialog->ShowModal();
	wadDialog->Destroy();
	delete wrapper;
	if (mode > DDMODE_VIEW) {
		if (parts&GMP_TEXT)
			wadText = dataBase->getWadText(mapEntry->wadPointer);
		writeEntry();
	}
}

void GuiMapManager::showAuthor(wxWindow* parent, bool second)
{
	AuthorEntry* ae = (second? mapEntry->author2: mapEntry->author1);
	if (ae != NULL) {
		dataBase->openAuthorTextFile();
		wxString notes = dataBase->getAuthorText(ae);
		if (ae->type > 0) {
			AuthorGroupEntry* group = dynamic_cast<AuthorGroupEntry*>(ae);
			GuiGroupEdit* gge = new GuiGroupEdit(parent,wxDefaultPosition,group,notes,dataBase);
			gge->ShowModal();
			gge->Destroy();
		} else {
			GuiPersonEdit* gpe = new GuiPersonEdit(parent,wxDefaultPosition,ae,&notes,true);
			gpe->ShowModal();
			gpe->Destroy();
		}
		dataBase->closeAuthorTextFile();
	}
}

void GuiMapManager::editAuthor(wxWindow* parent, bool second)
{
	AuthorEntry* ae = (second? mapEntry->author2: mapEntry->author1);
	int dbid = (ae==NULL? 0: ae->dbid);
	dataBase->openAuthorTextFile();
	readEntry();
	GuiPersonList* personDialog = new GuiPersonList(parent, wxDefaultPosition, dataBase, DDMODE_EDIT, dbid);
	int result = personDialog->ShowModal();
	if (result==BUTTON_OK) {
		ae = personDialog->getSelectedPerson();
		if (second && (ae!=mapEntry->author2)) {
			mapEntry->author2 = ae;
			mapEntry->ownFlags |= OF_MAINMOD;
		} else if ((!second) && (ae!=mapEntry->author1)) {
			mapEntry->author1 = ae;
			mapEntry->ownFlags |= OF_MAINMOD;
		}
		writeEntry();
	}
	personDialog->Destroy();
	dataBase->closeAuthorTextFile();
}

void GuiMapManager::showBasedOn(wxWindow* parent)
{
	if (mapEntry->basedOn != 0) {
		int x, y;
		parent->GetPosition(&x, &y);
		x+=5; y+=5;
		parent->GetPosition();
		MapEntry* based = dataBase->findMap(mapEntry->basedOn);
		ListWrapper<MapEntry*>* wrapper = new ListWrapper<MapEntry*>();
		wrapper->add(based);
		wrapper->reset();
		GuiMapEntry* mapDialog = new GuiMapEntry(parent, wxPoint(x,y), parent->GetSize(),
			wrapper, dataBase, mode, false);
		int result = mapDialog->ShowModal();
		mapDialog->Destroy();
		delete wrapper;
	}
}

void GuiMapManager::editBasedOn(wxWindow* parent)
{
	readEntry();
	GuiMapSelect* dlg = new GuiMapSelect(parent, wxDefaultPosition, dataBase, mapEntry, true);
	int result = dlg->ShowModal();
	MapEntry* basedMap = dlg->getSelectedMap();
	dlg->Destroy();
	if (result == BUTTON_OK) { //mapEntry was changed (basedOn and/or flag)
		if (mapEntry->basedOn == 0)
			mapStrFields[MAPSTR_BASEDON] = "";
		else
			mapStrFields[MAPSTR_BASEDON] = basedOnString(mapEntry, basedMap);
		if (mapEntry->flags&MF_SAMEAS)
			basedLabel->SetLabel("Same as");
		else
			basedLabel->SetLabel("Based on");
		dataBase->mapModified(mapEntry);
	}
}

void GuiMapManager::addTag(wxWindow* parent)
{
	int index = 0;
	while (index<MAXTAGS && (mapEntry->tags[index]!=0))
		index++;
	if (index == MAXTAGS) {
		wxMessageDialog* dlg = new wxMessageDialog(parent,
			"This entry has the maximum number of tags. To add a new tag, you must first remove another.",
			"Tag list full", wxOK);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		GuiTagList* tagDialog = new GuiTagList(parent, wxDefaultPosition, dataBase, DDMODE_EDIT, 0);
		int result = tagDialog->ShowModal();
		tagDialog->Destroy();
		if (result > 0) {
			for (int i=0; i<MAXTAGS; i++) {
				if (mapEntry->tags[i] == result)
					return; //Already in list
			}
			mapEntry->tags[index] = result;
			mapEntry->ownFlags |= OF_MAINMOD;
			writeTagList();
		}
	}
}

void GuiMapManager::removeSelectedTag()
{
	int i = listBox->GetSelection();
	if (i>=0 && i<MAXTAGS) {
		mapEntry->tags[i]=0;
		mapEntry->ownFlags |= OF_MAINMOD;
		writeTagList();
	}
}

string GuiMapManager::basedOnString(MapEntry* me, MapEntry* base)
{
	if (me->wadPointer == base->wadPointer)
		return base->name;
	else if (base->wadPointer->numberOfMaps == 1)
		return base->getTitle();
	else
		return base->wadPointer->getName()+":"+base->name;
}
