#include "GuiWadManager.h"
#include "GuiMapCommon.h"

GuiWadManager::GuiWadManager(DoomDialogMode m, DataManager* dm, bool hideUnused)
: dataBase(dm), mode(m), hide(hideUnused), parts(0), sizeLong(200,20), sizeMed(120,20),
wadEntry(NULL), wadText(NULL), commonButton(NULL), deleteButton(NULL)
{
	canEdit = (mode!=DDMODE_VIEW);
	textStyle = (canEdit)? 0: wxTE_READONLY;
}

GuiWadManager::~GuiWadManager()
{
}

void GuiWadManager::setDataManager(DataManager* dm)
{
	dataBase = dm;
	if (dataBase == NULL)
		clearEntry();
}

wxSizer* GuiWadManager::createMain(wxWindow* parent, const wxString& label)
{
	parts |= GWP_MAIN;

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,label),0);
	gridSizer->AddSpacer(1);

	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,WADSTR_TITLE,"",wxDefaultPosition,sizeLong,
		textStyle, wxTextValidator(wxFILTER_ASCII, &wadStrFields[WADSTR_TITLE]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_TITLE]),0);
	gridSizer->Add(txtCtrl,0);

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_IWAD]),0);
	if (canEdit) {
		wadChoice[0] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,IWAD_END,iwadNames);
		gridSizer->Add(wadChoice[0],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,WADSTR_IWAD,"",wxDefaultPosition,sizeLong,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_IWAD])),0);
	}

	wadFlagBoxes[0] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[0]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_IWAD]),0);
	gridSizer->Add(wadFlagBoxes[0],0);

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_ENGINE]),0);
	if (canEdit) {
		wadChoice[1] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,DENG_END,engineNames);
		gridSizer->Add(wadChoice[1],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,WADSTR_ENGINE,"",wxDefaultPosition,sizeLong,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_ENGINE])),0);
	}

	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_PLAYSTYLE]),0);
	if (canEdit) {
		wadChoice[2] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,3,playStyles);
		gridSizer->Add(wadChoice[2],0);
	} else {
		gridSizer->Add(new wxTextCtrl(parent,WADSTR_PLAYSTYLE,"",wxDefaultPosition,sizeLong,
			wxTE_READONLY,wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_PLAYSTYLE])),0);
	}

	txtCtrl = new wxTextCtrl(parent,WADSTR_YEAR,"",wxDefaultPosition,(canEdit)?sizeMed:sizeLong,
		textStyle, wxTextValidator(wxFILTER_NUMERIC, &wadStrFields[WADSTR_YEAR]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_YEAR]),0);
	gridSizer->Add(txtCtrl,0);

	txtCtrl = new wxTextCtrl(parent,WADSTR_IDGAMES,"",wxDefaultPosition,(canEdit)?sizeMed:sizeLong,
		textStyle, wxTextValidator(wxFILTER_NUMERIC, &wadStrFields[WADSTR_IDGAMES]));
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_IDGAMES]),0);
	gridSizer->Add(txtCtrl,0);
	return gridSizer;
}

wxSizer* GuiWadManager::createFileOwn(wxWindow* parent, bool file, bool own)
{
	fileOwnSizer = new wxFlexGridSizer(0,2,5,5);
	if (file) { // File fields
		parts |= GWP_FILE;
		fileOwnSizer->Add(new wxStaticText(parent,-1,"File:"),0);
		fileOwnSizer->AddSpacer(1);

		wxTextCtrl* txtCtrl = new wxTextCtrl(parent,WADSTR_FILENAME,"",wxDefaultPosition,sizeLong,
			textStyle, wxTextValidator(wxFILTER_ASCII, &wadStrFields[WADSTR_FILENAME]));
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_FILENAME]),0);
		fileOwnSizer->Add(txtCtrl,0);

		txtCtrl = new wxTextCtrl(parent,WADSTR_FILESIZE,"",wxDefaultPosition,sizeLong,
			textStyle, wxTextValidator(wxFILTER_NUMERIC, &wadStrFields[WADSTR_FILESIZE]));
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_FILESIZE]),0);
		fileOwnSizer->Add(txtCtrl,0);

		txtCtrl = new wxTextCtrl(parent,WADSTR_MD5,"",wxDefaultPosition,sizeLong,wxTE_READONLY,
			wxTextValidator(wxFILTER_ALPHANUMERIC, &wadStrFields[WADSTR_MD5]));
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_MD5DIGEST]),0);
		fileOwnSizer->Add(txtCtrl,0);

		txtCtrl = new wxTextCtrl(parent,WADSTR_XFILES,"",wxDefaultPosition,sizeLong,
			textStyle, wxTextValidator(wxFILTER_ASCII, &wadStrFields[WADSTR_XFILES]));
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_EXTRAFILES]),0);
		fileOwnSizer->Add(txtCtrl,0);

	}
	if (file && own) {
		fileOwnSizer->AddSpacer(10);
		fileOwnSizer->AddSpacer(10);
	}
	if (own) { // Own stuff
		parts |= GWP_OWN;
		fileOwnSizer->Add(new wxStaticText(parent,-1,"Personal:"),0);
		fileOwnSizer->AddSpacer(1);

		wadChoice[3] = new wxChoice(parent,0,wxDefaultPosition,sizeMed,
			12, rating10);
		wadChoice[3]->Enable(false);
		orLabel = new wxStaticText(parent,-1,wadMapLabels[WAD_OWNRATING]);
		fileOwnSizer->Add(orLabel,0);
		fileOwnSizer->Add(wadChoice[3],0);

		wxTextCtrl* txtCtrl = new wxTextCtrl(parent,WADSTR_PLAYTIME,"",wxDefaultPosition,sizeMed,wxTE_READONLY,
			wxTextValidator(wxFILTER_NUMERIC, &wadStrFields[WADSTR_PLAYTIME]));
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_PLAYTIME]),0);
		fileOwnSizer->Add(txtCtrl,0);

		wadFlagBoxes[11] = new wxCheckBox(parent, 0, "");
		wadFlagBoxes[11]->Enable(false);
		fileOwnSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_OF_HAVEFILE]),0);
		fileOwnSizer->Add(wadFlagBoxes[11],0);
	}
	return fileOwnSizer;
}

wxSizer* GuiWadManager::createContent(wxWindow* parent)
{
	parts |= GWP_CONTENT;

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(parent,-1,"Content:"),0);
	gridSizer->AddSpacer(1);

	wadFlagBoxes[1] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[1]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_SPRITES]),0);
	gridSizer->Add(wadFlagBoxes[1],0);

	wadFlagBoxes[2] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[2]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_TEX]),0);
	gridSizer->Add(wadFlagBoxes[2],0);

	wadFlagBoxes[3] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[3]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_GFX]),0);
	gridSizer->Add(wadFlagBoxes[3],0);

	wadFlagBoxes[4] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[4]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_COLOR]),0);
	gridSizer->Add(wadFlagBoxes[4],0);

	wadFlagBoxes[5] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[5]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_SOUND]),0);
	gridSizer->Add(wadFlagBoxes[5],0);

	wadFlagBoxes[6] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[6]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_MUSIC]),0);
	gridSizer->Add(wadFlagBoxes[6],0);

	wadFlagBoxes[7] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[7]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_DEHBEX]),0);
	gridSizer->Add(wadFlagBoxes[7],0);

	wadFlagBoxes[8] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[8]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_THINGS]),0);
	gridSizer->Add(wadFlagBoxes[8],0);

	wadFlagBoxes[9] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[9]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_SCRIPT]),0);
	gridSizer->Add(wadFlagBoxes[9],0);

	wadFlagBoxes[10] = new wxCheckBox(parent, 0, "");
	wadFlagBoxes[10]->Enable(canEdit);
	gridSizer->Add(new wxStaticText(parent,-1,wadMapLabels[WAD_WF_GLNODES]),0);
	gridSizer->Add(wadFlagBoxes[10],0);
	return gridSizer;
}

wxSizer* GuiWadManager::createNotes(wxWindow* parent)
{
	parts |= GWP_NOTES;

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->SetFlexibleDirection(wxHORIZONTAL);
	gridSizer->AddGrowableCol(1,1);
	wxTextCtrl* txtCtrl = new wxTextCtrl(parent,-1,"",wxDefaultPosition,wxDefaultSize,textStyle,
		wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_RESOURCES]));
	gridSizer->Add(new wxStaticText(parent,-1,"Resources"),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,1,wxEXPAND);

	txtCtrl = new wxTextCtrl(parent,-1,"",wxDefaultPosition,wxDefaultSize,textStyle,
		wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_PORTNOTES]));
	gridSizer->Add(new wxStaticText(parent,-1,"Port notes"),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,1,wxEXPAND);

	txtCtrl = new wxTextCtrl(parent,-1,"",wxDefaultPosition,wxDefaultSize,textStyle,
		wxTextValidator(wxFILTER_NONE, &wadStrFields[WADSTR_LINKS]));
	gridSizer->Add(new wxStaticText(parent,-1,"Web links"),0,wxALIGN_CENTER_VERTICAL);
	gridSizer->Add(txtCtrl,1,wxEXPAND);
	return gridSizer;
}

wxTextCtrl* GuiWadManager::createText(wxWindow* parent)
{
	parts |= GWP_TEXT;
	return new wxTextCtrl(parent, 3, "", wxDefaultPosition, wxDefaultSize,
		textStyle|wxTE_MULTILINE, wxTextValidator(wxFILTER_NONE, &noteInput));
}

wxBoxSizer* GuiWadManager::createMaps(wxWindow* parent, bool editCommon, bool deleteMaps)
{
	parts |= GWP_MAPS;
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(new wxStaticText(parent,-1,"Maps:"),0,wxBOTTOM|wxTOP,5);
	listBox = new wxListBox(parent, LMAP_LIST, wxDefaultPosition, wxDefaultSize); //wxSize(60,-1)
	rightSizer->Add(listBox,1,wxEXPAND,0);
	if (deleteMaps && (mode!=DDMODE_VIEW)) {
		deleteButton = new wxButton(parent,BUTTON_DELETE,"-",wxDefaultPosition,
			wxDefaultSize,wxBU_EXACTFIT);
		rightSizer->Add(deleteButton, 0, wxALL, 2);
	}
	if (editCommon && (mode!=DDMODE_VIEW)) {
		commonButton = new wxButton(parent,BUTTON_EDIT,"Edit all");
		rightSizer->Add(commonButton, 0, wxEXPAND|wxALL, 2);
	}
	return rightSizer;
}

void GuiWadManager::setWadEntry(WadEntry* we)
{
	wadEntry = we;
	if ((parts&GWP_NOTES) || (parts&GWP_TEXT))
		wadText = dataBase->getWadText(wadEntry);
}

void GuiWadManager::writeEntry()
{
	if (parts&GWP_MAIN) {
		wadStrFields[WADSTR_IDGAMES] = wxIntToString(wadEntry->idGames);
		wadStrFields[WADSTR_TITLE] = wadEntry->title;
		wadStrFields[WADSTR_YEAR] = wxIntToString(wadEntry->year);
		wadStrFields[WADSTR_RATING] = wxIntToString(wadEntry->rating); //Not shown
		wadFlagBoxes[0]->SetValue(wadEntry->flags&WF_IWAD);
		if (mode == DDMODE_VIEW) {
			wadStrFields[WADSTR_IWAD] = dataBase->getWadField(wadEntry,WAD_IWAD);
			wadStrFields[WADSTR_ENGINE] = dataBase->getWadField(wadEntry,WAD_ENGINE);
			wadStrFields[WADSTR_PLAYSTYLE] = dataBase->getWadField(wadEntry,WAD_PLAYSTYLE);
		} else {
			wadChoice[0]->SetSelection(wadEntry->iwad);
			wadChoice[1]->SetSelection(wadEntry->engine);
			wadChoice[2]->SetSelection(wadEntry->playStyle);
		}
	}
	if (parts&GWP_FILE) {
		wadStrFields[WADSTR_FILENAME] = wadEntry->fileName;
		wadStrFields[WADSTR_FILESIZE] = wxIntToString(wadEntry->fileSize);
		wadStrFields[WADSTR_MD5] = wadEntry->getMd5String();
		wadStrFields[WADSTR_XFILES] = wadEntry->extraFiles;
	}
	if (parts&GWP_OWN) {
		if ((wadEntry->numberOfMaps==1) && hide) {
			fileOwnSizer->Show(orLabel, false);
			fileOwnSizer->Show(wadChoice[3], false);
			fileOwnSizer->Layout();
		} else {
			fileOwnSizer->Show(orLabel, true);
			fileOwnSizer->Show(wadChoice[3], true);
			fileOwnSizer->Layout();
			wadChoice[3]->SetSelection(dataBase->scaleRating(wadEntry->ownRating));
			if (wadEntry->numberOfMaps == 1)
				wadChoice[3]->Enable(false);
			else
				wadChoice[3]->Enable(true);
		}
		wadStrFields[WADSTR_PLAYTIME] = wxIntToString(wadEntry->getTotalPlaytime());
		wadFlagBoxes[11]->SetValue(wadEntry->ownFlags&OF_HAVEFILE);
	}
	if (parts&GWP_CONTENT) {
		wadFlagBoxes[1]->SetValue(wadEntry->flags&WF_SPRITES);
		wadFlagBoxes[2]->SetValue(wadEntry->flags&WF_TEX);
		wadFlagBoxes[3]->SetValue(wadEntry->flags&WF_GFX);
		wadFlagBoxes[4]->SetValue(wadEntry->flags&WF_COLOR);
		wadFlagBoxes[5]->SetValue(wadEntry->flags&WF_SOUND);
		wadFlagBoxes[6]->SetValue(wadEntry->flags&WF_MUSIC);
		wadFlagBoxes[7]->SetValue(wadEntry->flags&WF_DEHBEX);
		wadFlagBoxes[8]->SetValue(wadEntry->flags&WF_THINGS);
		wadFlagBoxes[9]->SetValue(wadEntry->flags&WF_SCRIPT);
		wadFlagBoxes[10]->SetValue(wadEntry->flags&WF_GLNODES);
	}
	if (parts&GWP_NOTES) {
		wadStrFields[WADSTR_RESOURCES] = wadText->getText(WADTEXT_RES);
		wadStrFields[WADSTR_PORTNOTES] = wadText->getText(WADTEXT_PORT);
		wadStrFields[WADSTR_LINKS] = wadText->getText(WADTEXT_LINKS);
	}
	if (parts&GWP_TEXT) {
		noteInput = wadText->getText(WADTEXT_DESC);
	}
	if (parts&GWP_MAPS) {
		listBox->Clear();
		for (int i=0; i<wadEntry->numberOfMaps; i++)
			listBox->Append(wadEntry->mapPointers[i]->name);
		if (commonButton != NULL)
			commonButton->Enable(wadEntry->numberOfMaps > 1);
		if (deleteButton != NULL)
			deleteButton->Enable(wadEntry->numberOfMaps > 1);
	}
}

void GuiWadManager::readEntry()
{
	if (mode != DDMODE_VIEW) {
		long number;
		uint16_t flg = wadEntry->flags;

		if (parts&GWP_MAIN) {
			if (wadEntry->title.compare(wadStrFields[WADSTR_TITLE])) {
				wadEntry->title = wadStrFields[WADSTR_TITLE];
				wadEntry->ownFlags |= OF_MAINMOD;
				if (wadEntry->numberOfMaps==1) {
					wadEntry->mapPointers[0]->title = wadStrFields[WADSTR_TITLE];
					wadEntry->mapPointers[0]->ownFlags |= OF_MAINMOD;
					dataBase->mapModified(wadEntry->mapPointers[0]);
				}
			}
			if (wadStrFields[WADSTR_IDGAMES].ToLong(&number) && number!=wadEntry->idGames) {
				wadEntry->idGames = number;
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadStrFields[WADSTR_YEAR].ToLong(&number) && number!=wadEntry->year) {
				wadEntry->year = number;
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadFlagBoxes[0]->GetValue()) flg |= WF_IWAD;
			else flg &= ~WF_IWAD;
			if (wadChoice[0]->GetSelection() != wadEntry->iwad) {
				wadEntry->iwad = wadChoice[0]->GetSelection();
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadChoice[1]->GetSelection() != wadEntry->engine) {
				wadEntry->engine = wadChoice[1]->GetSelection();
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadChoice[2]->GetSelection() != wadEntry->playStyle) {
				wadEntry->playStyle = wadChoice[2]->GetSelection();
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			//wadStrFields[WADSTR_RATING] is read-only
		}

		if (parts&GWP_FILE) {
			if (wadEntry->fileName.compare(wadStrFields[WADSTR_FILENAME])) {
				wadEntry->fileName = wadStrFields[WADSTR_FILENAME];
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadEntry->extraFiles.compare(wadStrFields[WADSTR_XFILES])) {
				wadEntry->extraFiles = wadStrFields[WADSTR_XFILES];
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			if (wadStrFields[WADSTR_FILESIZE].ToLong(&number) && number!=wadEntry->fileSize) {
				wadEntry->fileSize = number;
				wadEntry->ownFlags |= OF_MAINMOD;
			}
			//wadStrFields[WADSTR_MD5] is read-only
		}

		if (parts&GWP_CONTENT) {
			if (wadFlagBoxes[1]->GetValue()) flg |= WF_SPRITES;
			else flg &= ~WF_SPRITES;
			if (wadFlagBoxes[2]->GetValue()) flg |= WF_TEX;
			else flg &= ~WF_TEX;
			if (wadFlagBoxes[3]->GetValue()) flg |= WF_GFX;
			else flg &= ~WF_GFX;
			if (wadFlagBoxes[4]->GetValue()) flg |= WF_COLOR;
			else flg &= ~WF_COLOR;
			if (wadFlagBoxes[5]->GetValue()) flg |= WF_SOUND;
			else flg &= ~WF_SOUND;
			if (wadFlagBoxes[6]->GetValue()) flg |= WF_MUSIC;
			else flg &= ~WF_MUSIC;
			if (wadFlagBoxes[7]->GetValue()) flg |= WF_DEHBEX;
			else flg &= ~WF_DEHBEX;
			if (wadFlagBoxes[8]->GetValue()) flg |= WF_THINGS;
			else flg &= ~WF_THINGS;
			if (wadFlagBoxes[9]->GetValue()) flg |= WF_SCRIPT;
			else flg &= ~WF_SCRIPT;
			if (wadFlagBoxes[10]->GetValue()) flg |= WF_GLNODES;
			else flg &= ~WF_GLNODES;
		}

		if (parts&GWP_NOTES) {
			wadText->setText(WADTEXT_RES, wadStrFields[WADSTR_RESOURCES]);
			wadText->setText(WADTEXT_PORT, wadStrFields[WADSTR_PORTNOTES]);
			wadText->setText(WADTEXT_LINKS, wadStrFields[WADSTR_LINKS]);
		}

		if (parts&GWP_TEXT) {
			wadText->setText(WADTEXT_DESC, noteInput);
		}

		//Check if flags changed
		if (wadEntry->flags != flg) {
			wadEntry->flags = flg;
			wadEntry->ownFlags |= OF_MAINMOD;
		}
	}

	// Personal data (always editable):
	if (parts&GWP_OWN) {
		if (wadEntry->numberOfMaps != 1) {
			unsigned char rat = dataBase->ratingScale(wadChoice[3]->GetSelection());
			if (rat != wadEntry->ownRating) {
				wadEntry->ownRating = rat;
				wadEntry->ownFlags |= OF_OWNMOD;
			}
		}
	}

	dataBase->wadModified(wadEntry);
	if (wadText!=NULL && wadText->changed)
		dataBase->saveWadText(wadText);
}

void GuiWadManager::clearEntry()
{
	//So far only complete for view mode (only disables input for private fields)
	wadEntry = NULL;
	wadText = NULL;
	for (int i=0; i<WADSTR_END; i++)
		wadStrFields[i] = "";
	if (parts&GWP_MAIN) {
		wadFlagBoxes[0]->SetValue(false);
		if (mode != DDMODE_VIEW) {
			wadChoice[0]->SetSelection(0);
			wadChoice[1]->SetSelection(0);
			wadChoice[2]->SetSelection(0);
		}
	}
	if (parts&GWP_OWN) {
		wadChoice[3]->SetSelection(11);
		wadChoice[3]->Enable(false);
		wadFlagBoxes[11]->SetValue(false);
	}
	if (parts&GWP_CONTENT) {
		for (int j=1; j<11; j++)
			wadFlagBoxes[j]->SetValue(false);
	}
	if (parts&GWP_TEXT) {
		noteInput = "";
	}
	if (parts&GWP_MAPS) {
		listBox->Clear();
		if (commonButton != NULL)
			commonButton->Enable(false);
		if (deleteButton != NULL)
			deleteButton->Enable(false);
	}
}

void GuiWadManager::modalMapDialog(wxWindow* parent, int mapIndex)
{
	if (mapIndex>=0 && mapIndex<wadEntry->numberOfMaps) {
		wxConfigBase* configObject = wxConfigBase::Get();
		int x = (int) configObject->Read("/MapEntry/x", 300);
		int y = (int) configObject->Read("/MapEntry/y", 200);
		int w = (int) configObject->Read("/MapEntry/w", 800);
		int h = (int) configObject->Read("/MapEntry/h", 550);

		ListWrapper<MapEntry*>* mapList = new ListWrapper<MapEntry*>();
		mapList->fromVector(wadEntry->mapPointers, wadEntry->numberOfMaps);
		mapList->setIndex(mapIndex);
		//wxLogVerbose("Selected map %s", mapList->entry()->name);
		GuiMapEntry* mapDialog = new GuiMapEntry(parent, wxPoint(x,y), wxSize(w,h),
			mapList, dataBase, mode, false);
		int result = mapDialog->ShowModal();
		mapDialog->Destroy();
		delete mapList;
	}
}

void GuiWadManager::editMapCommon(wxWindow* parent)
{
	MapEntry* mapTemp = new MapEntry(0);
	wadEntry->getCommonFields(mapTemp);
	GuiMapCommon* mapDialog = new GuiMapCommon(parent, wxDefaultPosition,
		mapTemp, dataBase);
	int result = mapDialog->ShowModal();
	if (result == BUTTON_OK) {
		wadEntry->setCommonFields(mapTemp, true);
		for (int i=0; i<wadEntry->numberOfMaps; i++)
			dataBase->mapModified(wadEntry->mapPointers[i]);
	}
	delete mapTemp;
}

bool GuiWadManager::deleteSelectedMap(wxWindow* parent)
{
	if (wadEntry->numberOfMaps < 2) {
		wxString msg = (mode == DDMODE_NEW)?
				"A wad entry must have at least one map. Cancel the wad entry to discard it.":
				"The map can't be deleted here, as a wad entry must have at least one map. Right-click on the wad or map in the main list to delete it.";
		wxMessageDialog* dlg = new wxMessageDialog(parent, msg,
				"Delete map", wxOK|wxICON_WARNING);
		dlg->ShowModal();
		dlg->Destroy();
		return false;
	}
	int i = listBox->GetSelection();
	if (i<0 || i>=wadEntry->numberOfMaps)
		return false;
	MapEntry* delMap = wadEntry->mapPointers[i];
	if (mode == DDMODE_NEW) {
		//Wad not yet in database
		dataBase->deleteMapFiles(delMap);
		if (wadText != NULL)
			wadText->removeText(delMap->name);
		wadEntry->removeMap(i,true);
		listBox->Clear();
		for (int i=0; i<wadEntry->numberOfMaps; i++)
			listBox->Append(wadEntry->mapPointers[i]->name);
		return true;
	} else if (mode == DDMODE_EDIT) {
		//Delete from database
		wxMessageDialog* dlg = new wxMessageDialog(parent,
				"Are you sure you wish to delete the map entry from the database? Text and images of the map will also be deleted.",
				"Delete map", wxYES_NO|wxNO_DEFAULT|wxICON_WARNING);
		int result = dlg->ShowModal();
		dlg->Destroy();
		if (result==wxID_YES) {
			dataBase->deleteMap(delMap);
			listBox->Clear();
			for (int i=0; i<wadEntry->numberOfMaps; i++)
				listBox->Append(wadEntry->mapPointers[i]->name);
			return true;
		}
		return false;
	}
}

void GuiWadManager::setSelectedMap(int index)
{
	if (parts&GWP_MAPS)
		listBox->SetSelection(index);
}
