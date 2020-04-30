#include "WadArchive.h"

WadArchive::WadArchive(wxString file, wxString tempFolder)
: fileName(file), tempPath(tempFolder), year(0)
{
	wadFiles = new vector<wxFileName>();
	txtFiles = new vector<wxFileName>();
	otherFiles = new vector<wxFileName>();
	extracted = new vector<wxString>();
}

WadArchive::~WadArchive()
{
	delete wadFiles;
	delete txtFiles;
	delete otherFiles;
	delete extracted;
}

void WadArchive::readArchiveFiles()
{
	wxFFileInputStream in(fileName);
	wxZipInputStream zip(in);
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry != NULL) {
		wxFileName pathName = wxFileName(entry->GetName());
		wxString ext = pathName.GetExt();
		if (ext.CmpNoCase("zip")==0) {
			//TODO: Extract zip
		} else if ((ext.CmpNoCase("wad")==0) || (ext.CmpNoCase("pk3")==0)) {
			wadFiles->push_back(pathName);
			wxLogVerbose("Try to get file timestamp");
			wxDateTime dt = entry->GetDateTime();
			if ((dt.IsValid()) && (dt.GetYear() > year))
				year = dt.GetYear();
		} else if (ext.CmpNoCase("txt")==0) {
			txtFiles->push_back(pathName);
		} else if (!entry->IsDir()) {
			otherFiles->push_back(pathName);
		}
		delete entry;
		entry = zip.GetNextEntry();
	}
	wxLogVerbose("Found %i wad/pk3 files, %i text files and %i other files in archive %s",
		wadFiles->size(), txtFiles->size(), otherFiles->size(), fileName);
}

void WadArchive::printReport(TextReport* reportView)
{
	reportView->writeHeading(fileName);
	for (int i=0; i<wadFiles->size(); i++)
		reportView->writeLine(wadFiles->at(i).GetFullPath());
	for (int i=0; i<txtFiles->size(); i++)
		reportView->writeLine(txtFiles->at(i).GetFullPath());
	for (int i=0; i<otherFiles->size(); i++)
		reportView->writeLine(otherFiles->at(i).GetFullPath());
}

wxString WadArchive::extractFile(wxString file, TaskProgress* tp)
{
	wxFFileInputStream in(fileName);
	wxZipInputStream zip(in);
	wxString intName = wxZipEntry::GetInternalName(file);
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry!=NULL && entry->GetInternalName()!=intName) {
		delete entry;
		entry = zip.GetNextEntry();
	}
	if (entry != NULL) {
		int met = entry->GetMethod();
		if ((met!=wxZIP_METHOD_STORE) && (met!=wxZIP_METHOD_DEFLATE)) {
			tp->fatalError("Unsupported compression method");
			delete entry;
			return "";
		}
		wxString ename = entry->GetName();
		//There have been cases of wads with name starting with #,
		//and wxWidgets fails to handle these file names.
		if (ename.Find("#") !=  wxNOT_FOUND) {
			tp->fatalError("Illegal file name with #");
			delete entry;
			return "";
		}
		wxFileName tempName(tempPath+wxFILE_SEP_PATH+ename);
		if (!tempName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
			tp->fatalError(wxString::Format("Failed to mkdir %s",tempName.GetFullPath()));
			delete entry;
			return "";
		}
		wxFileOutputStream file(tempName.GetFullPath());
		if (!file) {
			tp->fatalError(wxString::Format("Can't extract file %s",tempName.GetFullPath()));
			delete entry;
			return "";
		}
        zip.Read(file);
        file.Close();
        wxLogVerbose("Extracted file %s", ename);
		delete entry;
		extracted->push_back(tempName.GetFullPath());
		return tempName.GetFullPath();
	} else {
		return "";
	}
}

wxString WadArchive::extractWad(int index, TaskProgress* tp)
{
	return extractFile(wadFiles->at(index).GetFullPath(), tp);
}

wxString WadArchive::extractTxt(int index, TaskProgress* tp)
{
	return extractFile(txtFiles->at(index).GetFullPath(), tp);
}

wxString WadArchive::extractDehacked(TaskProgress* tp)
{
	for (int i=0; i<otherFiles->size(); i++) {
		if (otherFiles->at(i).GetExt().CmpNoCase("deh")==0)
			return extractFile(otherFiles->at(i).GetFullPath(), tp);
	}
	return "";
}

void WadArchive::deleteExtracted()
{
	for (int i=0; i<extracted->size(); i++) {
		if (wxRemoveFile(extracted->at(i)))
			wxLogVerbose("Deleted %s", extracted->at(i));
		else
			wxLogVerbose("Could not delete %s", extracted->at(i));
	}
	extracted->clear();
}
