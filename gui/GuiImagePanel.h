/*!
* \file GuiImagePanel.h
* \author Lars Thomas Boye 2018
*
* GUI component for displaying an image from file.
* GuiImagePanel is the main component, while GuiImageDropTarget
* is a helper class for receiving files dragged and dropped on
* the panel. GuiImageDialog is a dialog with image panel.
*/
#ifndef GUIIMAGEPANEL_H
#define GUIIMAGEPANEL_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/dnd.h>
#include <wx/file.h>
#include <wx/clipbrd.h>
#include <wx/metafile.h>
#include "GuiBase.h"


/*!
* Panel showing an image loaded from file. Can either resize the
* image to fit the panel size, or make the image have a static size
* and include scrollbars if the image is larger than the panel.
* The panel can also work as an input for the image file, with
* three different mechanisms supported. Click the panel for a file
* dialog. Right-click for a menu with paste option, to paste from
* the clipboard (bitmap or metafile). Or drag-and-drop an image file.
* The panel is given a fixed file name, and shows this file if it
* exists. Any imported image is saved with this file name, so it
* replaces any existing file.
*
* Based on the example: https://wiki.wxwidgets.org/An_image_panel
*/
class GuiImagePanel : public wxScrolledWindow
{
	public:
		/*!
		* The constructor sets some important parameters. res=true to
		* resize the image to always fit the size of the panel. false
		* for fixed image size, with scrollbars to pan the image if it
		* is larger than the panel. canSel=true to use the panel to
		* import an image file (click for file dialog, right-click to
		* paste from clipboard, or drag-and-drop the file). canOpen to
		* open the image in a stand-alone window when clicked.
		*/
		GuiImagePanel(wxWindow* parent, bool res=false, bool canSel=false, bool canOpen=true);

		virtual ~GuiImagePanel();

		/*! Set text to show when there is no image. */
		void setNoImageText(wxString text);

		/*! Sets the file name, load and show image from file if it exists. */
		void loadImage(wxString file, wxBitmapType format);

		/*! Removes the file name and any image shown. */
		void clearImage();

		/*! Receives file name from drag-and-drop. */
		bool onFileDropped(const wxString &fileNam);

		/*! Left-click to open image in dialog or select from file. */
		void onClick(wxMouseEvent& event);

		/*! Right-click menu, with paste of image from clipboard. */
		void onRightClick(wxMouseEvent& event);

		virtual wxSize DoGetBestClientSize() const;

		/*! Handler for paint event, renders image. */
		void paintEvent(wxPaintEvent & evt);

		/*! Render image. */
        void paintNow();

        /*! Handler for resize events, to refresh the image. */
        void OnSize(wxSizeEvent& event);

        /*! Does the actual drawing of the image, including any scaling. */
        void render(wxDC& dc);

        // some useful events
        /*
         void mouseMoved(wxMouseEvent& event);
         void mouseDown(wxMouseEvent& event);
         void mouseWheelMoved(wxMouseEvent& event);
         void mouseReleased(wxMouseEvent& event);
         void rightClick(wxMouseEvent& event);
         void mouseLeftWindow(wxMouseEvent& event);
         void keyPressed(wxKeyEvent& event);
         void keyReleased(wxKeyEvent& event);
         */

	protected:

	private:
		/*!
		* Processes input image, scaling it down if over max size and
		* saving to file.
		*/
		bool createImage();

		/*! Shows file dialog for selecting image file. */
		void selectFile();

		wxString noImageStr;
		wxString fileName;
		wxBitmapType bitmapType;
		bool canSelect;
		bool showDialog;

		bool hasImage;
		double imgRatio; //w/h of current image
		wxImage image; //Image loaded from file
		wxBitmap* resized; //Bitmap to draw
		bool resizing; //true to resize image to fit panel size
		int w, h; //Remembered panel size, to check for change

	DECLARE_EVENT_TABLE()
};

/*!
* wxFileDropTarget implementation for the GuiImagePanel, to
* receive files dropped on the panel. It simply passes the filename
* to the owning panel.
*/
class GuiImageDropTarget : public wxFileDropTarget
{
	public:
		/*! Constructed by the GuiImagePanel. */
		GuiImageDropTarget(GuiImagePanel* owner);

		virtual ~GuiImageDropTarget();

		virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);

	private:
		GuiImagePanel* owner;
};

/*!
* Dialog with GuiImagePanel as only content.
*/
class GuiImageDialog : public wxDialog
{
	public:
		GuiImageDialog(wxWindow* parent, const wxString& title, wxString file, wxBitmapType format, bool res=false);
		virtual ~GuiImageDialog();
};

#endif // GUIIMAGEPANEL_H
