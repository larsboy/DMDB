/*!
* \file Pk3Stats.h
* \author Lars Thomas Boye 2020
*
* Pk3Stats is a specialization of WadStats, for processing zip archives
* instead of wad files.
*/

#ifndef PK3STATS_H
#define PK3STATS_H

#include <wx/zipstrm.h>
#include "WadStats.h"

/*!
* Specialization of WadStats, processing pk3 (zip) files. So in place of
* wad lumps it reads the files in the archive. Files which needs further
* processing are temporarily unpacked. The archive can contain wad files.
* Map lumps are typically placed in wad files, but any valid wad file can
* be part of an archive. Pk3Stats unpacks each wad file and processes it
* with a WadStats object. The results are merged into the Pk3Stats, so
* it reports on all content, including that found in contained wad files.
*/
class Pk3Stats : public WadStats
{
	public:
		/*!
		* In addition to the file name (full path), the Pk3Stats needs a
		* folder for temporary file cache.
		*/
		Pk3Stats(wxString file, wxString tempFolder);

		virtual ~Pk3Stats();

		/*!
		* Deletes unpacked files.
		*/
		virtual void cleanup();

		virtual void readFile(TaskProgress* progress, bool findMd5=true);

	protected:

	private:
		/*! Override for extracting included files from archive.  */
		virtual WadContentX* findUnknownLumps(vector<wxString>* inclNames, WadContentType wct, TaskProgress* progress);

		/*!
		* Searches for a DirEntry with a name matching the name argument.
		* In addition to an exact match, it tries to add file extensions
		* to name.
		*/
		DirEntry* findFileEntry(vector<DirEntry*>* dirEntries, wxString name);

		/*!
		* Process file entry in archive, categorizing it into the content
		* array. Corresponds to WadStats.processLump, but returns true if
		* the entry should be extracted from the archive.
		*/
		bool processFile(DirEntry* dir, wxZipInputStream& file);

		/*! Process DirEntry with file path, categorizing it based on path name. */
		bool processDirFile(DirEntry* dir, const wxFileName& pathName);

		/*! Process wad file extracted from the archive, adding its content/stats to the Pk3Stats. */
		void processWad(DirEntry* dir, TaskProgress* progress);

		/*!
		* Extract a file to tempPath. The wxZipEntry must be the current entry of
		* the wxZipInputStream. The complete path of the extracted file is
		* returned.
		*/
		wxString extractFile(wxZipInputStream& zip, wxZipEntry* entry, TaskProgress* progress);

		/*!
		* Extract a file to tempPath, returning the complete path.
		*/
		wxString extractFile(wxString file, TaskProgress* progress);

		wxString tempPath; //To extract files, delete later
		vector<wxString>* extracted; //Files extracted to tempFolder
};

#endif // PK3STATS_H
