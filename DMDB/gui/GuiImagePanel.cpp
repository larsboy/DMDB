#include "GuiImagePanel.h"

//***************************************************************
//************************ GuiImagePanel ************************
//***************************************************************

GuiImagePanel::GuiImagePanel(wxWindow* parent, bool res, bool canSel, bool canOpen)
: wxScrolledWindow(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_SIMPLE),
noImageStr(""), fileName(""), canSelect(canSel), showDialog(canOpen),
hasImage(false), imgRatio(0.0), resized(NULL), resizing(res)
{
	if (canSelect)
		SetDropTarget(new GuiImageDropTarget(this));
}

GuiImagePanel::~GuiImagePanel()
{
	if (resized != NULL)
		delete resized;
}

void GuiImagePanel::setNoImageText(wxString text)
{
	noImageStr = text;
}

void GuiImagePanel::loadImage(wxString file, wxBitmapType format)
{
	fileName = file;
	bitmapType = format;
	if (resized != NULL)
		delete resized;
	resized = NULL;
	hasImage = false;

	hasImage = wxFile::Exists(fileName);
	if (hasImage)
		hasImage = image.LoadFile(file, bitmapType);
    w = -1;
    h = -1;
    if (hasImage) {
    	imgRatio = (double)image.GetWidth() / (double)image.GetHeight();
		if (!resizing) {
			resized = new wxBitmap(image);
			SetVirtualSize(image.GetWidth(), image.GetHeight());
			SetScrollRate(10, 10);
		}
    }
    paintNow();
}

void GuiImagePanel::clearImage()
{
	fileName = "";
	if (resized != NULL)
		delete resized;
	resized = NULL;
	hasImage = false;
	w = -1;
    h = -1;
    paintNow();
}

bool GuiImagePanel::createImage()
{
	if (image.GetWidth()>SSHOT_WIDTH || image.GetHeight()>SSHOT_HEIGHT) {
		//Rescale to SSHOT_HEIGHT
		double factor = (double)image.GetHeight()/(double)SSHOT_HEIGHT;
		wxLogVerbose("Scale down by factor %f", factor);
		int wid = (int)(image.GetWidth()/factor);
		image.Rescale(wid,SSHOT_HEIGHT,wxIMAGE_QUALITY_HIGH);
	}
	imgRatio = (double)image.GetWidth() / (double)image.GetHeight();
	image.SaveFile(fileName, wxBITMAP_TYPE_PNG);
	wxLogVerbose("Saved image file %s", fileName);

	if (resized != NULL)
		delete resized;
	resized = NULL;
	hasImage = true;
	w = -1;
	h = -1;
	if (!resizing) {
		resized = new wxBitmap(image);
		SetVirtualSize(image.GetWidth(), image.GetHeight());
		SetScrollRate(10, 10);
	}
	paintNow();
	return true;
}

void GuiImagePanel::selectFile()
{
	wxString fileNam="";
	//Get filename from dialog:
	wxFileDialog* fdlg = new wxFileDialog(this, "Select Screenshot", "", "",
		"PNG files (*.png)|*.png|All files (*.*)|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST|wxFD_CHANGE_DIR);
	if (fdlg->ShowModal()==wxID_OK)
	{
		fileNam = fdlg->GetPath();
	}
	fdlg->Destroy();
	if (fileNam.Length() > 0) {
		if (image.LoadFile(fileNam))
			createImage();
	}
}

wxSize GuiImagePanel::DoGetBestClientSize() const
{
	if (hasImage)
		return wxSize(image.GetWidth(),image.GetHeight());
	else
		return wxSize(250,100);
}

//Event table:
BEGIN_EVENT_TABLE(GuiImagePanel, wxPanel)
	EVT_LEFT_DCLICK(GuiImagePanel::onClick)
	EVT_RIGHT_DOWN(GuiImagePanel::onRightClick)
	EVT_PAINT(GuiImagePanel::paintEvent)
	EVT_SIZE(GuiImagePanel::OnSize)
	/*
	EVT_MOTION(wxImagePanel::mouseMoved)
	EVT_LEFT_DOWN(wxImagePanel::mouseDown)
	EVT_LEAVE_WINDOW(wxImagePanel::mouseLeftWindow)
	EVT_KEY_DOWN(wxImagePanel::keyPressed)
	EVT_KEY_UP(wxImagePanel::keyReleased)
	EVT_MOUSEWHEEL(wxImagePanel::mouseWheelMoved)
	*/
END_EVENT_TABLE()

bool GuiImagePanel::onFileDropped(const wxString &fileNam)
{
	if (fileName.Length() == 0)
		return false; //No file defined
	wxLogVerbose("Loading screenshot file %s", fileNam);
	if (image.LoadFile(fileNam)) { //wxBITMAP_TYPE_PNG
		createImage();
		return true;
	} else {
		return  false;
	}
}

void GuiImagePanel::onClick(wxMouseEvent& event)
{
	if (fileName.Length() == 0)
		return; //No file defined
	if ((!hasImage) && canSelect) {
		selectFile();
	} else if (showDialog) {
		wxString title = (resizing)? "Screenshot": "Map image";
		GuiImageDialog* gdlg = new GuiImageDialog(this, title, fileName, bitmapType, resizing);
		gdlg->ShowModal();
		gdlg->Destroy();
	}
}

void GuiImagePanel::onRightClick(wxMouseEvent& event)
{
	if (fileName.Length() == 0)
		return; //No file defined
	if (canSelect) {
		wxMenu menu;
		menu.Append(10, "Paste");
		menu.Append(20, "Select file");
		if (hasImage)
			menu.Append(30, "Delete");
		wxClientDC dc(this);
		int result = GetPopupMenuSelectionFromUser(menu, event.GetLogicalPosition(dc));
		if (result==10) {
			if (wxTheClipboard->Open()) {
				if (wxTheClipboard->IsSupported(wxDF_BITMAP)) {
					wxBitmapDataObject bmData;
					wxTheClipboard->GetData(bmData);
					wxBitmap bitmap = bmData.GetBitmap();
					wxLogVerbose("Pasted bitmap %i x %i", bitmap.GetWidth(), bitmap.GetHeight());
					image = bitmap.ConvertToImage();
					createImage();
				} else if (wxTheClipboard->IsSupported(wxDF_METAFILE)) {
					//Get this from Access
					wxMetaFileDataObject data;
					wxTheClipboard->GetData(data);
					wxMetaFile mf = data.GetMetafile();
					wxLogMessage("Pasted metafile image %i x %i",
                         mf.GetWidth(), mf.GetHeight());
					wxBitmap bm(mf.GetWidth(), mf.GetHeight(), wxBITMAP_SCREEN_DEPTH);
					wxMemoryDC temp_dc(bm);
					if (mf.IsOk()) {
						mf.Play(&temp_dc);
						temp_dc.SelectObject(wxNullBitmap);
						image = bm.ConvertToImage();
						createImage();
					}
				} else {
					wxLogVerbose("No compatible image data to paste");
				}
				wxTheClipboard->Close();
			}
		} else if (result==20) {
			selectFile();
		} else if (result==30) {
			//image.Destroy();
			wxRemoveFile(fileName);
			if (resized != NULL)
				delete resized;
			resized = NULL;
			hasImage = false;
			w = -1;
			h = -1;
			paintNow();
		}
	}
}

/*
 * Called by the system by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
void GuiImagePanel::paintEvent(wxPaintEvent & evt)
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void GuiImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we call refresh to tell the panel to draw itself again.
 * So when the user resizes the image panel the image should be resized too.
 */
void GuiImagePanel::OnSize(wxSizeEvent& event)
{
	if (resizing && hasImage) {
		Refresh();
		//skip the event.
		event.Skip();
	}
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void GuiImagePanel::render(wxDC& dc)
{
	DoPrepareDC(dc);
	dc.Clear();
    int neww, newh;
    dc.GetSize(&neww,&newh);
    if ((neww<1) || (newh<1))
		return;

    if (neww != w || newh != h) {
		if (resizing && hasImage) {
			if (resized != NULL)
				delete resized;
			double curRatio = (double)neww / (double)newh;
			int imgw = neww;
			int imgh = newh;
			if (curRatio > imgRatio) {
				//Full height
				imgw = (int)(imgh*imgRatio);
			} else {
				//Full width
				imgh = (int)(imgw/imgRatio);
			}
			if (imgw>1 && imgh>1)
				resized = new wxBitmap(image.Scale(imgw,imgh /*,wxIMAGE_QUALITY_HIGH*/));
			else
				resized = NULL;
		}
        w = neww;
        h = newh;
    }
	if (hasImage) {
		if (resized != NULL)
			dc.DrawBitmap(*resized, 0, 0, false);
    } else {
        dc.DrawText(noImageStr, 10,10);
    }
}

// some useful events
/*
 void GuiImagePanel::mouseMoved(wxMouseEvent& event) {}
 void GuiImagePanel::mouseDown(wxMouseEvent& event) {}
 void GuiImagePanel::mouseWheelMoved(wxMouseEvent& event) {}
 void GuiImagePanel::mouseReleased(wxMouseEvent& event) {}
 void GuiImagePanel::rightClick(wxMouseEvent& event) {}
 void GuiImagePanel::mouseLeftWindow(wxMouseEvent& event) {}
 void GuiImagePanel::keyPressed(wxKeyEvent& event) {}
 void GuiImagePanel::keyReleased(wxKeyEvent& event) {}
 */

//********************************************************************
//************************ GuiImageDropTarget ************************
//********************************************************************

GuiImageDropTarget::GuiImageDropTarget(GuiImagePanel* owner)
{
	this->owner = owner;
}

GuiImageDropTarget::~GuiImageDropTarget()
{
}

bool GuiImageDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.IsEmpty())
		return false;
	else
		return owner->onFileDropped(filenames.Item(0));
}

//****************************************************************
//************************ GuiImageDialog ************************
//****************************************************************

GuiImageDialog::GuiImageDialog(wxWindow* parent, const wxString& title, wxString file, wxBitmapType format, bool res)
: wxDialog(parent, -1, title, wxDefaultPosition, wxSize(640,480), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	GuiImagePanel* gip = new GuiImagePanel(this, res, false, false);
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(gip, 1, wxEXPAND);
	SetSizer(sizer);

	gip->loadImage(file, format);
}

GuiImageDialog::~GuiImageDialog()
{
}
