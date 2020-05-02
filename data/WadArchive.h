/*!
* \file WadArchive.h
* \author Lars Thomas Boye 2018
*
* WadArchive represents a zip file with wad file and other
* related files. It finds contained files and extracts them
* to a temporary folder, so that they can be processed
* normally by other classes.
*/

#ifndef WADARCHIVE_H
#define WADARCHIVE_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/filefn.h>
#include "../TextReport.h"
#include "../gui/GuiBase.h"
#include "TaskProgress.h"

/*!
* Represents one relevant file found inside an archive file.
*/
struct ArchivedFile
{
	wxString archive; //!< File path and name of archive file
	wxFileName file; //!< Name of file contained in the archive

	ArchivedFile(const wxString& arch, const wxFileName& fn) : archive(arch), file(fn) {}

	/*! Returns true if the archived file has the given extension. */
	bool hasType(const wxString& ext) { return (file.GetExt().CmpNoCase(ext) == 0); }
};

/*!
* Represents the archive file, and gives access to the content. It is
* created with the path of the archive file, as well as a folder to
* hold extracted files. Call readArchiveFiles to read the archive,
* numberOf-functions to get the number of files found in the archive
* of different types, and extract-methods to extract the files to the
* tempFolder. The extracted files can then be processed by other classes.
* WadArchive can handle nested archives, finding the relevant files in
* internal archives as well. Be sure to call deleteExtracted when done
* with the files, to delete them.
*/
class WadArchive
{
	public:
	WadArchive(wxString file, wxString tempFolder);
	virtual ~WadArchive();

	/*!
	* Reads through the archive, getting the name of relevant files
	* within. If the archive contains nested archives (zip, not pk3),
	* these will be extracted and read through, to find relevant files
	* in nested archives (this can fail). Relevant files are categorized
	* as wad/pk3, txt and other. Get the number of each type with the
	* numberOf-methods, and extract specific files with the extract-methods.
	*/
	void readArchiveFiles(TaskProgress* tp);

	/*! Number of wad or pk3 files in the archive. */
	int numberOfWads() { return wadFiles->size(); }

	/*! Number of txt files in the archive. */
	int numberOfTxts() { return txtFiles->size(); }

	/*! Number of files other than wad or txt in the archive. */
	int numberOfOther() { return otherFiles->size(); }

	/*!
	* Returns the year of the file timestamp of the wad or pk3 file.
	* If no wad or pk3 file was found, it returns 0. If there are
	* multiple such files, it returns the highest value.
	*/
	uint16_t getWadYear() { return year; }

	/*!
	* Lists the contents of the archive through the TextReport
	* interface.
	*/
	void printReport(TextReport* reportView);

	/*!
	* Extract a wad or pk3 file to tempFolder. The argument is
	* the index in the list of wads in the archive, from 0 to
	* numberOfWads(). The full path of the extracted file is
	* returned.
	*/
	wxString extractWad(int index, TaskProgress* tp);

	/*!
	* Extract a txt file to tempFolder. The argument is the
	* index in the list of wads in the archive, from 0 to
	* numberOfTxts(). The full path of the extracted file is
	* returned.
	*/
	wxString extractTxt(int index, TaskProgress* tp);

	/*!
	* Extract a Dehacked file (*.deh) to tempFolder, if such
	* a file was found in the archive. The full path of the
	* extracted file is returned, or an empty string if none
	* is found. Note that if there are multiple Dehacked files,
	* this will simply extract the first it happens to find.
	*/
	wxString extractDehacked(TaskProgress* tp);

	/*!
	* Delete any files extracted to the tempFolder by this
	* WadArchive object.
	*/
	void deleteExtracted();

	protected:

	private:
		ArchivedFile* findWad(const wxFileName& file);
		void readArchive(wxString file, TaskProgress* tp);
		wxString extractFile(wxZipInputStream* zip, wxZipEntry* entry, TaskProgress* tp);
		wxString extractFile(ArchivedFile* af, TaskProgress* tp);

		wxString fileName; //Path/name of main zip file
		wxString tempPath; //To extract files, delete later
		int year; //Wad/pk3 file year
		vector<ArchivedFile*>* wadFiles; //Wad/pk3 files found in archives
		vector<ArchivedFile*>* txtFiles; //Txt files found in archives
		vector<ArchivedFile*>* otherFiles; //Other files found in archives
		vector<wxString>* extracted; //Files extracted to tempFolder
};

#endif // WADARCHIVE_H
