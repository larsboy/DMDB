#include "WadArchive.h"

WadArchive::WadArchive(wxString file, wxString tempFolder)
: fileName(file), tempPath(tempFolder), year(0)
{
	wadFiles = new vector<ArchivedFile*>();
	txtFiles = new vector<ArchivedFile*>();
	otherFiles = new vector<ArchivedFile*>();
	extracted = new vector<wxString>();
}

WadArchive::~WadArchive()
{
	for (int i=0; i<wadFiles->size(); i++)
		delete (*wadFiles)[i];
	delete wadFiles;
	for (int i=0; i<txtFiles->size(); i++)
		delete (*txtFiles)[i];
	delete txtFiles;
	for (int i=0; i<otherFiles->size(); i++)
		delete (*otherFiles)[i];
	delete otherFiles;
	delete extracted;
}

ArchivedFile* WadArchive::findWad(const wxFileName& file)
{
	for (int i=0; i<wadFiles->size(); i++)
		if ((*wadFiles)[i]->file == file) return (*wadFiles)[i];
	return NULL;
}

void WadArchive::readArchiveFiles(TaskProgress* tp)
{
	readArchive(fileName, tp);
	wxLogVerbose("Found %i wad/pk3 files, %i text files and %i other files in archive %s",
		wadFiles->size(), txtFiles->size(), otherFiles->size(), fileName);
}

void WadArchive::readArchive(wxString file, TaskProgress* tp)
{
	wxFFileInputStream in(file);
	wxZipInputStream zip(in);
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry != NULL) {
		wxFileName pathName = wxFileName(entry->GetName());
		wxString ext = pathName.GetExt();
		if (ext.CmpNoCase("zip")==0) {
			wxString extrZip = extractFile(&zip, entry, tp);
			if (extrZip.Length() > 0)
				readArchive(extrZip, tp);
		} else if ((ext.CmpNoCase("wad")==0) || (ext.CmpNoCase("pk3")==0)) {
			ArchivedFile* af = findWad(pathName);
			if (af == NULL) {
				wadFiles->push_back(new ArchivedFile(file, pathName));
				wxLogVerbose("Try to get file timestamp");
				wxDateTime dt = entry->GetDateTime();
				if ((dt.IsValid()) && (dt.GetYear() > year))
					year = dt.GetYear();
			} else {
				wxLogVerbose("Duplicate filename %s", pathName.GetFullPath());
			}
		} else if (ext.CmpNoCase("txt")==0) {
			txtFiles->push_back(new ArchivedFile(file, pathName));
		} else if (!entry->IsDir()) {
			otherFiles->push_back(new ArchivedFile(file, pathName));
		}
		delete entry;
		entry = zip.GetNextEntry();
	}
}

void WadArchive::printReport(TextReport* reportView)
{
	reportView->writeHeading(fileName);
	for (int i=0; i<wadFiles->size(); i++)
		reportView->writeLine(wadFiles->at(i)->file.GetFullPath());
	for (int i=0; i<txtFiles->size(); i++)
		reportView->writeLine(txtFiles->at(i)->file.GetFullPath());
	for (int i=0; i<otherFiles->size(); i++)
		reportView->writeLine(otherFiles->at(i)->file.GetFullPath());
}

wxString WadArchive::extractFile(wxZipInputStream* zip, wxZipEntry* entry, TaskProgress* tp)
{
	int met = entry->GetMethod();
	if ((met!=wxZIP_METHOD_STORE) && (met!=wxZIP_METHOD_DEFLATE)) {
		tp->fatalError("Unsupported compression method");
		return "";
	}
	wxString ename = entry->GetName();
	//There have been cases of wads with name starting with #,
	//and wxWidgets fails to handle these file names.
	if (ename.Find("#") !=  wxNOT_FOUND) {
		tp->fatalError("Illegal file name with #");
		return "";
	}
	wxFileName tempName(tempPath+wxFILE_SEP_PATH+ename);
	if (!tempName.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
		tp->fatalError(wxString::Format("Failed to mkdir %s",tempName.GetFullPath()));
		return "";
	}
	wxFileOutputStream file(tempName.GetFullPath());
	if (!file) {
		tp->fatalError(wxString::Format("Can't extract file %s",tempName.GetFullPath()));
		return "";
	}
	zip->Read(file);
	file.Close();
	wxLogVerbose("Extracted file %s", ename);
	extracted->push_back(tempName.GetFullPath());
	return tempName.GetFullPath();
}

wxString WadArchive::extractFile(ArchivedFile* af, TaskProgress* tp)
{
	wxFFileInputStream in(af->archive);
	wxZipInputStream zip(in);
	wxString intName = wxZipEntry::GetInternalName(af->file.GetFullPath());
	wxZipEntry* entry = zip.GetNextEntry();
	while (entry!=NULL && entry->GetInternalName()!=intName) {
		delete entry;
		entry = zip.GetNextEntry();
	}
	wxString result;
	if (entry != NULL) {
		result=extractFile(&zip, entry, tp);
		delete entry;
	} else {
		result="";
	}
	return result;
}

wxString WadArchive::extractWad(int index, TaskProgress* tp)
{
	return extractFile(wadFiles->at(index), tp);

}

wxString WadArchive::extractTxt(int index, TaskProgress* tp)
{
	return extractFile(txtFiles->at(index), tp);
}

wxString WadArchive::extractDehacked(TaskProgress* tp)
{
	for (int i=0; i<otherFiles->size(); i++) {
		if (otherFiles->at(i)->hasType("deh"))
			return extractFile(otherFiles->at(i), tp);
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
