#include "GuiWadReport.h"

//**************************************************************
//************************ GuiWadReport ************************
//**************************************************************

GuiWadReport::GuiWadReport(wxWindow* parent, const wxPoint& pos, WadReader* reader)
: GuiReport(parent, "Wad report", pos, wxSize(600,500))
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	//Report view
	mainSizer->Add(reportView, 5, wxEXPAND|wxALL, 8);

	//Map list
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	rightSizer->Add(new wxStaticText(this,-1,"Maps:"),0,wxBOTTOM,5);
	listBox = new wxListBox(this, LVIEW_LIST, wxDefaultPosition, wxDefaultSize); //wxSize(60,-1)
	rightSizer->Add(listBox,1,wxEXPAND,0);
	mainSizer->Add(rightSizer, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 8);

	//Buttons (Ok/Cancel) is the last part:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	if (reader->hasArchive()) {
		removeButton = new wxButton(this,BUTTON_DELETE,"Remove file...");
		bottomSizer->Add(removeButton, 0, wxALL, 10);
		removeButton->Enable(reader->numberOfWads()>1);
	} else {
		bottomSizer->Add(new wxButton(this,BUTTON_ADDFILE,"Add file..."), 0, wxALL, 10);
	}
	bottomSizer->Add(new wxButton(this,LTHING_OPEN,"Map Things..."), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_EDIT,"Aspects..."), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"Add to database"), 0, wxALL, 10);
	bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Discard"), 0, wxALL, 10);

	//Adding the main parts to the layout:
	topSizer->Add(mainSizer, 1, wxEXPAND);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size

    wadReader = reader;
    wadReader->printReport(this);
    listMaps();
}

void GuiWadReport::listMaps()
{
	listBox->Clear();
	vector<wxString>* mapNames = wadReader->getMapNames();
	for (int i=0; i<mapNames->size(); i++)
		listBox->Append(mapNames->at(i));
	delete mapNames;
}

//Event table:
BEGIN_EVENT_TABLE(GuiWadReport, wxDialog)
	EVT_LISTBOX_DCLICK(LVIEW_LIST, GuiWadReport::onMapSelected)
	EVT_BUTTON(BUTTON_ADDFILE, GuiWadReport::onAddFile)
	EVT_BUTTON(BUTTON_DELETE, GuiWadReport::onRemoveFile)
	EVT_BUTTON(LTHING_OPEN, GuiWadReport::onThingDefs)
	EVT_BUTTON(BUTTON_EDIT, GuiWadReport::onAspects)
    EVT_BUTTON(BUTTON_OK, GuiWadReport::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiWadReport::onCancel)
    EVT_CLOSE(GuiWadReport::onClose)
END_EVENT_TABLE()

void GuiWadReport::onMapSelected(wxCommandEvent& event)
{
	string fileName("");
	string mapName;
	wxString entry = event.GetString(); // [fileName:]mapName
	int i = entry.Find(':');
	if (i != wxNOT_FOUND) {
		fileName = entry.SubString(0,i-1).c_str();
		mapName = entry.Mid(i+1).c_str();
	} else {
		mapName = entry.c_str();
	}

	WadProgress* progress = new WadProgress("Analyse map");
	GuiProgress* progDialog = NULL;
	int bytes = wadReader->getMapLumpSize(fileName, mapName);
	if (bytes > 400000) { //Show progress dialog for large map
		progDialog = new GuiProgress(this, wxDefaultPosition);
		progDialog->Show();
		progress->setDialog(progDialog);
	}
	MapStats* mapStats = wadReader->processMap(fileName, mapName, progress);
	if (progDialog != NULL) {
		progress->setDialog(NULL);
		progDialog->Close(true);
		progDialog->Destroy();
	}
	if (progress->hasFailed()) {
		wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
			"Map analysis failed", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		GuiMapReport* mapDialog = new GuiMapReport(this, wxDefaultPosition, mapStats);
		int result = mapDialog->ShowModal();
		mapDialog->Destroy();
	}
	delete progress;
	if (mapStats != NULL)
		delete mapStats;
}

void GuiWadReport::onAddFile(wxCommandEvent& event)
{
	wxString fileName="";
	//Get filename from dialog
	wxFileDialog* fdlg = new wxFileDialog(this, "Open file", "", "",
		"Wad files (*.wad)|*.wad|Pk3 files (*.pk3)|*.pk3|Deh files (*.deh)|*.deh|All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	if (fdlg->ShowModal()==wxID_OK) {
		fileName=fdlg->GetPath();
		fdlg->Destroy();
	} else {
		fdlg->Destroy();
		return; //No file selected
	}
	try {
		if (fileName.EndsWith("deh") || fileName.EndsWith("DEH")) {
			addDeh(fileName);
		} else if (fileName.EndsWith("wad") || fileName.EndsWith("WAD")) {
			addWad(fileName, false);
		} else {
			addWad(fileName, true);
		}
	} catch (GuiError e) {
		wxMessageDialog* dlg = new wxMessageDialog(this, e.text1,
			"File error", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	}
}

void GuiWadReport::addDeh(wxString fileName)
{
	WadProgress* progress = new WadProgress("Process Dehacked");
	wadReader->addDeh(fileName, this, progress);
	if (progress->hasFailed()) {
		wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
			"File analysis failed", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	}
	delete progress;
}

void GuiWadReport::addWad(wxString fileName, bool pk3)
{
	GuiProgress* progDialog = new GuiProgress(this, wxDefaultPosition);
	progDialog->Show();
	WadProgress* progress = new WadProgress("Process file");
	progress->setDialog(progDialog);
	wadReader->addWad(fileName, pk3, this, progress); //Try to load file
	progress->setDialog(NULL);
	progDialog->Close(true);
	progDialog->Destroy();
	if (progress->hasFailed()) {
		wxMessageDialog* dlg = new wxMessageDialog(this, progress->getError(),
			"File analysis failed", wxOK | wxICON_ERROR);
		dlg->ShowModal();
		dlg->Destroy();
	} else {
		listMaps();
	}
	delete progress;
}

void GuiWadReport::onRemoveFile(wxCommandEvent& event)
{
	vector<wxString>* wads = wadReader->getWadNames();
	wxArrayString as;
	for (int i=0; i<wads->size(); i++)
		as.Add(wads->at(i));
	delete wads;
	wxSingleChoiceDialog* dlg = new wxSingleChoiceDialog(this, "Select wad to remove",
		"Remove file", as);
	if (dlg->ShowModal()==wxID_OK) {
		wadReader->removeWad(dlg->GetSelection());
		reportView->Clear();
		paragraph = false;
		wadReader->printReport(this);
		listMaps();
		removeButton->Enable(wadReader->numberOfWads()>1);
	}
	dlg->Destroy();
}

void GuiWadReport::onThingDefs(wxCommandEvent& event)
{
	GuiThingDialog* dialog = new GuiThingDialog(this, wxDefaultPosition, wadReader->getThingDefs(), true);
	int result = dialog->ShowModal();
	dialog->Destroy();
	//The ThingDefs of WadReader reflects any changes made in the dialog
}

void GuiWadReport::onAspects(wxCommandEvent& event)
{
	WadStatAspects* aspects = wadReader->getAspects();
	GuiAspectDialog* aspectDialog = new GuiAspectDialog(this, wxDefaultPosition, aspects,
		"Aspects to set", "Set which fields to fill in when creating or updating database entries:");
	int result = aspectDialog->ShowModal();
	aspectDialog->Destroy();
}

void GuiWadReport::onOk(wxCommandEvent& event)
{
	EndModal(BUTTON_OK);
}

void GuiWadReport::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiWadReport::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}


//************************************************************
//************************ GuiWadType ************************
//************************************************************

GuiWadType::GuiWadType(wxWindow* parent, const wxPoint& pos, unsigned char iwd, unsigned char eng, int files)
: wxDialog(parent, -1, "Wad classification", pos, wxSize(400,300), wxDEFAULT_DIALOG_STYLE),
iwad(iwd), engine(eng)
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxSize sizeMed=wxSize(120,20);

	if (files > -1) {
		wxStaticText* text = new wxStaticText(this, -1, wxString::Format(
			"Found %i files to process. If you know all are for a specific iwad or engine, you can specify it here. Leave blank for a best guess to be made in each case.",
			files));
		text->Wrap(400);
		topSizer->Add(text, 0, wxTOP|wxLEFT|wxRIGHT, 16);
	}

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(0,2,5,5);
	gridSizer->Add(new wxStaticText(this,-1,wadMapLabels[WAD_IWAD]),0);
	iwadChoice = new wxChoice(this,0,wxDefaultPosition,sizeMed,IWAD_END,iwadNames);
	gridSizer->Add(iwadChoice,0);
	iwadChoice->SetSelection(iwad);

	gridSizer->Add(new wxStaticText(this,-1,wadMapLabels[WAD_ENGINE]),0);
	engineChoice = new wxChoice(this,0,wxDefaultPosition,sizeMed,DENG_END,engineNames);
	gridSizer->Add(engineChoice,0);
	engineChoice->SetSelection(engine);
	topSizer->Add(gridSizer,0,wxALL,20);

	//topSizer->AddSpacer(10);

	if (files > -1) {
		wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
		bottomSizer->Add(new wxButton(this,BUTTON_OK,"Start"), 0, wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_EDIT,"More..."), 0, wxALL, 10);
		bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
		topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);
	} else {
		topSizer->Add(new wxButton(this,BUTTON_OK,"Done"), 0, wxALL|wxALIGN_CENTER,10);
	}

	SetSizer(topSizer);
    topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    topSizer->Fit(this); //Resize window to match topSizer minimal size
}

//Event table:
BEGIN_EVENT_TABLE(GuiWadType, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiWadType::onOk)
    EVT_BUTTON(BUTTON_EDIT, GuiWadType::onMore)
    EVT_BUTTON(BUTTON_CANCEL, GuiWadType::onCancel)
    EVT_CLOSE(GuiWadType::onClose)
END_EVENT_TABLE()

void GuiWadType::onOk(wxCommandEvent& event)
{
	iwad = iwadChoice->GetSelection();
	engine = engineChoice->GetSelection();
	EndModal(BUTTON_OK);
}

void GuiWadType::onMore(wxCommandEvent& event)
{
	iwad = iwadChoice->GetSelection();
	engine = engineChoice->GetSelection();
	EndModal(BUTTON_EDIT);
}

void GuiWadType::onCancel(wxCommandEvent& event)
{
	EndModal(BUTTON_CANCEL);
}

void GuiWadType::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_CANCEL);
}
