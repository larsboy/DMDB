/*!
* \file GuiProgress.h
* \author Lars Thomas Boye 2020
*
* Dialog showing status/progress when processing wad files.
*/

#ifndef GUIPROGRESS_H
#define GUIPROGRESS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/richtext/richtextctrl.h>
#include "GuiBase.h"

/*!
* Dialog to show during potentially long-running operations, such as
* when processing wad files. It displays the name of the current task
* and a progress bar to be updated as the task progresses. It can also
* include a text log, to show a log of tasks done. In this mode, the
* dialog can be closed by the user, with a button enabled when ready.
*/
class GuiProgress : public wxDialog
{
	public: //wxSize(320,300)
		/*!
		* Regular wxDialog constructor arguments, except for the log option.
		* Enable this to include a text log in the dialog, as well as a
		* button to close the dialog.
		*/
		GuiProgress(wxWindow* parent, const wxPoint& pos, const wxSize& siz=wxDefaultSize, bool log=false);

		virtual ~GuiProgress();

		/*!
		* Set a name to display, identifying the current task in progress.
		*/
		void setLabel(wxString name);

		/*!
		* Set the range of the progress bar - the number of units
		* needed to fill it. pos is the number of filled units,
		* 0 by default.
		*/
		void setRange(int range, int pos=0);

		/*!
		* Set the position of the progress bar, in progress units up to
		* the range set with setRange.
		*/
		void setProgress(int pos);

		/*!
		* Add a line of text to the log, if the dialog includes a log.
		*/
		void logLine(wxString line);

		/*!
		* Call when the tracked process is finished, to close the dialog.
		* If the dialog has a close button, this is enabled, so that the
		* user can close the dialog.
		*/
		void finished();

		void onOk(wxCommandEvent& event); //!< Close dialog
		//void onClose(wxCloseEvent& event); //!< Dialog closed

	protected:

	private:
		wxStaticText* label;
		wxGauge* bar;
		wxRichTextCtrl* logView;
		wxButton* closeButton;

	//Macro for the event table
	DECLARE_EVENT_TABLE()
};

#endif // GUIPROGRESS_H
