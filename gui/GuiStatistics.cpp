#include "GuiStatistics.h"

//******************************************************************
//************************ StatisticsReport ************************
//******************************************************************

StatisticsReport::StatisticsReport(wxWindow* parent, const wxPoint& pos, DBStatistics* stats)
: GuiReport(parent, "Statistics", pos, wxSize(560,400))
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	//Report view
	topSizer->Add(reportView, 1, wxEXPAND|wxALL, 8);

	//Buttons (Ok/Cancel) is the last part:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"OK"), 0, wxALL, 10);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size

    stats->printReport(this);
}

//Event table:
BEGIN_EVENT_TABLE(StatisticsReport, wxDialog)
    EVT_BUTTON(BUTTON_OK, StatisticsReport::onOk)
    EVT_CLOSE(StatisticsReport::onClose)
END_EVENT_TABLE()

void StatisticsReport::onOk(wxCommandEvent& event)
{
	EndModal(BUTTON_OK);
}

void StatisticsReport::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_OK);
}


//**************************************************************
//************************ GuiStatsList ************************
//**************************************************************

GuiStatsList::GuiStatsList(wxWindow* parent, StatisticSet* stats)
: wxListCtrl(parent,LVIEW_LIST,wxDefaultPosition, wxDefaultSize,
			wxLC_REPORT|wxLC_VIRTUAL|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL)
{
	statSet = stats;
	for (int i=0; i<statSet->fieldCount; i++) {
		wxListItem col;
		//We would like to change the style of the header line,
		//but this does not seem to be possible (no effect).
		//col.SetBackgroundColour(*wxBLUE);
		//col.SetTextColour(*wxRED);
		//wxFont f = col.GetFont();
		//col.SetFont(f.Bold());
		if (i == 0) {
			col.SetText(statSet->getCategory());
		} else {
			col.SetText(statLabels[statSet->fields[i]]);
		}
		col.SetWidth(statSet->width[i]);
		col.SetAlign(wxLIST_FORMAT_LEFT);
		//if (statSet->fields[i] == sortField)
		InsertColumn(i, col);
	}
	updateList();
}

GuiStatsList::~GuiStatsList()
{
}

void GuiStatsList::updateList()
{
	statList = statSet->getStatistics();
	if (statList->size() > 0) {
		iter = statList->begin();
		iterIndex = 0;
	} else {
		iterIndex = -1;
	}
	unsigned int i = statList->size();
	SetItemCount(i);
	RefreshItems(0, i-1);
}

wxString GuiStatsList::OnGetItemText(long item, long column) const
{
	//if (iterIndex>-1 && item>=0 && item<statList->size()) {
	while (item > iterIndex) {
		iter++;
		iterIndex++;
	}
	while (item < iterIndex) {
		iter--;
		iterIndex--;
	}
	DBStatistics* dbs = *iter;
	if (column == 0) {
		return dbs->heading;
	} else {
		int index = statSet->fields[column];
		if (index < STS_LINEDEFS_AVG)
			return wxString::Format("%i", dbs->intStats[index]);
		else
			return wxString::Format("%.2f", dbs->floatStats[index]);

	}
}

//Event table:
BEGIN_EVENT_TABLE(GuiStatsList, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(LVIEW_LIST, GuiStatsList::itemActivated)
	EVT_LIST_COL_CLICK(LVIEW_LIST, GuiStatsList::colClicked)
END_EVENT_TABLE()

void GuiStatsList::itemActivated(wxListEvent& event)
{
	long item = event.GetIndex();
	while (item > iterIndex) {
		iter++;
		iterIndex++;
	}
	while (item < iterIndex) {
		iter--;
		iterIndex--;
	}
	DBStatistics* dbs = *iter;
	StatisticsReport* report = new StatisticsReport(this, wxPoint(200,200), dbs);
	int result = report->ShowModal();
	report->Destroy();
}

void GuiStatsList::colClicked(wxListEvent& event)
{
	statSet->sort(event.GetColumn());
	updateList();
}

//***************************************************************
//************************ GuiStatistics ************************
//***************************************************************

GuiStatistics::GuiStatistics(wxWindow* parent, const wxPoint& pos, StatisticSet* stats)
: wxDialog(parent, -1, stats->getHeading(), pos, wxSize(800,500), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	//Main layout objects:
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	GuiStatsList* statList = new GuiStatsList(this,stats);
	topSizer->Add(statList, 1, wxEXPAND|wxALL, 8);

	//Buttons (Ok/Cancel) is the last part:
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(new wxButton(this,BUTTON_OK,"OK"), 0, wxALL, 10);
	//bottomSizer->Add(new wxButton(this,BUTTON_CANCEL,"Done"), 0, wxALL, 10);
	topSizer->Add(bottomSizer, 0, wxALIGN_CENTER);

	//Implementing the sizer:
    SetSizer(topSizer);
    //topSizer->SetSizeHints(this); //Set minimal size for window based on topSizer
    //topSizer->Fit(this); //Resize window to match topSizer minimal size
}

GuiStatistics::~GuiStatistics()
{
	//dtor
}

//Event table:
BEGIN_EVENT_TABLE(GuiStatistics, wxDialog)
    EVT_BUTTON(BUTTON_OK, GuiStatistics::onOk)
    EVT_CLOSE(GuiStatistics::onClose)
END_EVENT_TABLE()

void GuiStatistics::onOk(wxCommandEvent& event)
{
	EndModal(BUTTON_OK);
}

void GuiStatistics::onClose(wxCloseEvent& event)
{
	EndModal(BUTTON_OK);
}
