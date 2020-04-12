/*!
* \file GuiMain.h
* \author Lars Thomas Boye 2007
*
* GuiMain is the top-level representation of the application. It doesn't do
* much, just initializing the main window object.
*
* A wxWidgets macro creates the application, defining the main-function. This
* will result in a call to GuiMain::OnInit() on startup, so this is where
* program execution starts in our code.
*/

#ifndef GUIMAIN_H //Avoid problems with multiple includes
#define GUIMAIN_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

/*!
* GuiMain is the top-level representation of the application. It is a
* specialisation of the wxApp class in the wxWidgets library. It has one window,
* a wxFrame, which contains the main control and widgets like a toolbar. GuiMain
* doesn't do much, as the top-level functionality of the program is located in
* the window class. GuiMain just initializes the window object.
*/
class GuiMain : public wxApp
{
	public:
	/*!
	* OnInit() is called on application startup and is a good place for the app
    * initialization. In addition to creating the main window, it can process command
    * line arguments and pass them on to the window.
    */
    virtual bool OnInit();

    /*! OnExit() is called at termination. */
    virtual int OnExit();
};

//! A wxWidgets macro for the declaration of wxGetApp implemented by IMPLEMENT_APP
wxDECLARE_APP(GuiMain);

#endif
