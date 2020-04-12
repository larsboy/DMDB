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
		if ((ext.CmpNoCase("wad")==0) || (ext.CmpNoCase("pk3")==0)) {
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

wxString WadArchive::extractFile(wxString file)
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
		wxFileName tempName(tempPath+wxFILE_SEP_PATH+entry->GetName());
		if (!tempName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
			delete entry;
			throw new GuiError("Can't extract file", entry->GetName());
		}
		wxFileOutputStream file(tempName.GetFullPath());
		if (!file) {
			delete entry;
			throw new GuiError("Can't extract file", tempName.GetFullPath());
		}
        zip.Read(file);
        file.Close();
        wxLogVerbose("Extracted file %s", entry->GetName());
		delete entry;
		extracted->push_back(tempName.GetFullPath());
		return tempName.GetFullPath();
	} else {
		return "";
	}
}

wxString WadArchive::extractWad(int index)
{
	return extractFile(wadFiles->at(index).GetFullPath());
}

wxString WadArchive::extractTxt(int index)
{
	return extractFile(txtFiles->at(index).GetFullPath());
}

wxString WadArchive::extractDehacked()
{
	for (int i=0; i<otherFiles->size(); i++) {
		if (otherFiles->at(i).GetExt().CmpNoCase("deh")==0)
			return extractFile(otherFiles->at(i).GetFullPath());
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
