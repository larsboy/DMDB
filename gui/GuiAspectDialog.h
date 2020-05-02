/*!
* \file GuiAspectDialog.h
* \author Lars Thomas Boye 2019
*
* A simple dialog for specifying which aspects of wad and map
* entries to update.
*/

#ifndef GUIASPECTDIALOG_H
#define GUIASPECTDIALOG_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "GuiBase.h"
#include "../data/WadReader.h"

/*!
* Dialog corresponding to the WadStatAspects struct for
* specifying which aspects of wad and map entries to update.
* The struct has a set of flags, and the dialog has corresponding
* checkboxes. When used at the start of processing multiple wads,
* it can include an additional checkbox for whether to replace
* existing entries or not, if matches are found. This also adds
* extra buttons (More and Cancel).
*/
class GuiAspectDialog : public wxDialog
{
    public:
    	/*!
    	* Give the dialog a custom title and instructional message.
    	* If repl is true, the dialog will include the extra checkbox
    	* for the "replace existing" flag, and extra buttons.
    	*/
        GuiAspectDialog(wxWindow* parent, const wxPoint& pos, WadStatAspects* asp, const wxString& title, const wxString& msg, bool repl=false);
        virtual ~GuiAspectDialog();

        //Event handlers:
        void onOk(wxCommandEvent& event); //!< Ending with ok
        void onMore(wxCommandEvent& event); //!< Ending with More
		void onCancel(wxCommandEvent& event); //!< Ending with Cancel
		void onClose(wxCloseEvent& event); //!< Dialog closed

		/*!
		* If the dialog has the replace checkbox, get its
		* value after the dialog has closed.
		*/
		bool getReplaceFlag() { return replaceFlag; }

    private:
        void writeData(); //!< Transfer data from struct to GUI
		void readData(); //!< Transfer data from GUI to struct

		bool replaceFlag;
        WadStatAspects* aspects;
        wxCheckBox* checks[9];

    DECLARE_EVENT_TABLE()
};

#endif // GUIASPECTDIALOG_H
