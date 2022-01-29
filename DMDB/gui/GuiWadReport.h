/*!
* \file GuiWadReport.h
* \author Lars Thomas Boye 2018
*
* Defines user interface components for reading and processing wad
* files and other Doom-engine resource files. GuiWadReport is the
* main GUI dialog of the process, acting as a user interface for a
* WadReader object. GuiWadType is a simple dialog for specifying the
* target iwad and engine of a wad.
*/

#ifndef GUIWADREPORT_H
#define GUIWADREPORT_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiBase.h"
#include "GuiMapReport.h"
#include "GuiThingDef.h"
#include "GuiAspectDialog.h"
#include "../data/WadStats.h"
#include "../data/WadReader.h"

/*!
* GuiWadReport is a wxDialog which represents a user interface for
* a WadReader object. It shows the results of the analysis of wads
* and other resource files, being a GuiReport with a rich text widget
* as its main component. It lists the maps found, and can launch
* analysis and reports on the individual maps. It provides the user
* interface to add and remove files to the WadReader, and can launch
* a dialog to show and edit the thing definitions used in map analysis.
*/
class GuiWadReport : public GuiReport
{
	public:
	/*!
	* Constructed with the WadReader object. At this point it should
	* have done the initial processing of a wad/archive (processWads),
	* as the dialog shows the report and allows adding more files.
	*/
	GuiWadReport(wxWindow* parent, const wxPoint& pos, WadReader* reader);

	virtual ~GuiWadReport() {}

	//Event handlers:
	void onMapSelected(wxCommandEvent& event); //!< Launch GuiMapReport
	void onAddFile(wxCommandEvent& event); //!< Add file to WadReader
	void onRemoveFile(wxCommandEvent& event); //!< Remove file from WadReader
	void onThingDefs(wxCommandEvent& event); //!< Show ThingDef dialog
	void onAspects(wxCommandEvent& event); //!< Show Aspects dialog
	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onCancel(wxCommandEvent& event); //!< Ending with cancel
	void onClose(wxCloseEvent& event); //!< Dialog closed

	private:
	/*! Displays the list of maps currently known to the WadReader. */
	void listMaps();

	/*! Process deh file, with progress dialog. */
	void addDeh(wxString fileName);

	/*! Process wad/pk3 file, with progress dialog. */
	void addWad(wxString fileName, bool pk3);

	WadReader* wadReader;
	wxListBox* listBox;
	wxButton* removeButton;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

/*!
* Dialog for selecting an iwad and engine. These are given as
* unsigned integers, corresponding to the IwadType and EngineType
* enums of the data model. Initial values are provided, and the
* results can be retrieved after the user has closed the dialog.
* If the dialog is used for processing multiple files, the number
* of files are displayed, and there are extra options (buttons
* More and Cancel).
*/
class GuiWadType : public wxDialog
{
	public:
	/*!
	* Constructed with initial iwad and engine choices. Specify number of
	* files if used for multi-file processing, adding instructions and
	* extra buttons.
	*/
	GuiWadType(wxWindow* parent, const wxPoint& pos, unsigned char iwd, unsigned char eng, int files=-1);

	virtual ~GuiWadType() {}

	void onOk(wxCommandEvent& event); //!< Ending with ok
	void onMore(wxCommandEvent& event); //!< Ending with More
	void onCancel(wxCommandEvent& event); //!< Ending with Cancel
	void onClose(wxCloseEvent& event); //!< Dialog closed

	/*! Call after the dialog is done, to get the final iwad choice. */
	unsigned char getIwad() { return iwad; }

	/*! Call after the dialog is done, to get the final engine choice. */
	unsigned char getEngine() { return engine; }

	private:
	unsigned char iwad;
	unsigned char engine;
	wxChoice* iwadChoice;
	wxChoice* engineChoice;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUIWADREPORT_H
