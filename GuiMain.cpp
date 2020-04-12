/*
* Implementation of GuiMain.
* GuiMain is the top-level representation of the application, but it doesn't do
* much, as the top-level functionality of the program is located in the main
* frame class; GuiFrame. All GuiMain does is create the GuiFrame.
*/

#include "GuiMain.h"
#include "gui/GuiBase.h"
#include "gui/GuiFrame.h"


//A wxWidgets macro creates the application, defining the main-function and
//the accessor function wxGetApp() which will return a reference to the
//GuiMain-object.
wxIMPLEMENT_APP(GuiMain);

//************************ GuiMain ************************

//Program execution starts here:
bool GuiMain::OnInit()
{
	//Vendor/App names identify the application to the OS. It is used for the
	//registry entries keeping application settings. It is defined in GuiBase.
	SetVendorName(VENDOR_NAME);
	SetAppName(APP_NAME);

	//Image handlers?
	//wxInitAllImageHandlers();
	//To support PNG images:
	wxImage::AddHandler(new wxPNGHandler);

    //Create the main application window. APP_TITLE is defined in GuiBase.
    GuiFrame* topFrame = new GuiFrame(APP_TITLE);
    //topFrame->SetIcon(wxICON(top_icon));

    //Show the main application window:
    topFrame->Show(TRUE);

    //We can define a top window, but it's not necessary when we only have one
    //main window.
    //SetTopWindow(topFrame);

	//Command line arguments are available through argc (of type int) and argv
	//(of type char**). So it can be associated with a file type, or handle
	//dragging and dropping a file on the program icon. The first command line
	//argument is the exe-file of the application; if there are more arguments
	//the next one can be passed to topFrame.
	wxString comArgument="";
	if (argc>1)
	{
		comArgument.Append(argv[1]);
		//topFrame->loadFile(comArgument);
	}

	//The application might show a dialog at startup
	//topFrame->showStartupDialog();

    //When true is returned, wxApp::OnRun() will be called which will enter the
    //main message loop and the application will run. Returning FALSE would
    //exit the application.
    return TRUE;
}

int GuiMain::OnExit()
{
	//Delete the config object the application has been using:
	delete wxConfigBase::Set((wxConfigBase *) NULL);

	return 0;
}
