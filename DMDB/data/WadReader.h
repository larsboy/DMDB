/*!
* \file WadReader.h
* \author Lars Thomas Boye 2018
*
* WadReader is responsible for the overall coordination of
* reading and processing wad files and other Doom-engine
* resource files. It can read one or more resource files,
* producing WadStats objects summarizing the content. It can
* process maps from the files, producing MapStats objects
* with detailed map statistics. If we want to enter the
* results into the database, it can produce a WadEntry for
* the resource files, with a MapEntry object for each map.
* It is responsible for drawing a map image for each MapEntry.
*/

#ifndef WADREADER_H
#define WADREADER_H

#include <vector>
#include <list>
#include <map>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include "../TextReport.h"
#include "WadArchive.h"
#include "Pk3Stats.h"
#include "ThingDef.h"
#include "MapStats.h"
#include "HexenMapStats.h"
#include "UdmfMapStats.h"
#include "DehackedParser.h"
#include "DataModel.h"

//Map drawing:
/*!
* Constant for scaling of map image, used in drawing the map.
* The Doom map units (matching pixels in the original resolution
* textures/flats) are divided by this, so the larger the number,
* the smaller the image. A multiple of 8 is recommended.
*/
const int DRAW_SCALE = 16;

const wxColour MAP_BACKGROUNDCOLOR(255,255,255); //!< Canvas background color: White
const wxColour MAP_WALL1(0,0,0); //!< Line color for single-sided map lines: Black
const wxColour MAP_WALL2(128,128,128); //!< Line color for double-sided map lines: Dark grey


/*!
* When creating/updating database entries, we use this struct
* to specify which fields to fill in. All are true by default.
* Disabling some is useful for updating certain aspects of the
* entries, leaving the rest unchanged.
*/
struct WadStatAspects
{
	bool wadFile; //!< WadEntry file data (name, size, year, engine, ...)
	bool wadFlags; //!< WadEntry content flags
	bool mapMain; //!< MapEntry title
	bool gameModes; //!< MapEntry game mode fields (sp/coop/dm)
	bool mapStats; //!< Map counts and flags (not gameplay stats)
	bool gameStats; //!< Gameplay stats (hp, ratios)
	bool mapImages; //!< Create map drawing

	/*!
	* If the application is unable to process a content file when
	* creating the DB entries, we can save a copy of the file in
	* the DB 'failed' folder.
	*/
	bool copyFailedFiles;

	/*! Created with all fields true, except copyFailedFiles. */
	WadStatAspects();

	/*! true if any MapEntry aspect is true. */
	bool mapAspects();
};


/*!
* Handles the creation of data objects from wad files. A single
* WadReader object should be reused each time. Usage starts with
* calling initReader to specify the main file to process, which
* clears any existing results. This could be a wad or pk3 file,
* or an archive with such files. Then call processWads to process
* the file. A WadStats object is created for each wad file.
* Additional resource files can then be added with methods like addWad
* and addDeh. With multiple wads, one is always designated as the main
* file.
*
* processWads will set iwad and engine properties based on what
* it finds. These are a best guess, and can be explicitly changed.
* They are mainly important for selecting the right thing
* definitions to use in map analysis. findThingDefs loads these
* ThingDefs based on iwad/engine, and applies modifications from
* Dehacked, Decorate etc. The result can be retrieved with
* getThingDefs, and it can be further modified.
*
* A report on all the resource files, not including individual
* map reports, is produced with printReport. An analysis of a
* single map, which can also print a report, is produced with
* processMap. Finally, we can create or update database entries
* based on the WadStats and other results produced. This
* processes all maps in the wad, creating/updating MapEntries
* for the main WadEntry. A WadStatAspects object is used to
* specify which aspects of the entries to update. WadReader is
* also responsible for creating line drawings of each map.
*
* Each method which processes files needs a TaskProgress object.
* Each such method represents a process which may take some time and
* can potentially fail. The TaskProgress object is used to track
* progress and log errors. Upon completion, check its hasFailed
* status. If true, the process failed and the task object has an
* error message which can be shown.
*/
class WadReader
{
	public:
	WadReader();
	~WadReader();

	/*!
	* Configure the WadReader with files defining properties for
	* map things for different types of games. So far we support:
	* 0: None/custom - no file
	* 1: Basic Doom (incl. Boom)
	* 2: ZDoom Doom (incl. GZDoom)
	* 3: Skulltag Doom
	* 4: Heretic
	*/
	void setThingFile(int type, wxString file);

	/*!
	* Specify which fields to fill in when creating or updating
	* database entries.
	*/
	void setAspects(WadStatAspects* asp);

	/*!
	* Returns a pointer to the WadStatAspects used by the WadReader
	* to control which fields to fill in when creating or updating
	* database entries.
	*/
	WadStatAspects* getAspects() { return aspects; }

	/*!
	* Folder to hold any temporary files during processing (these
	* are deleted on completion).
	*/
	void setTempFolder(wxString folder);

	/*!
	* Folder to put files we were not able to process.
	*/
	void setFailedFolder(wxString folder);

	/*!
	* Deletes any file processing results.
	*/
	void clearState();

	/*!
	* The first method to call when we want to process a file.
	* It names the initial file (full path), which could be
	* the main resource file itself, or a zip-file containing
	* the main and other resource files.
	*/
	void initReader(wxString file, TaskProgress* tp);

	/*!
	* true if the file set with initReader is a zip.
	*/
	bool hasArchive() { return archive!=NULL; }

	/*!
	* Load and analyze any resource files specified by the initial
	* file (initReader call). If the file is a wad or pk3 file, this
	* is processed, if it is an archive, all wad/pk3 files in the archive
	* are processed. One WadStats object is created for each wad/pk3
	* file. One is designated the main file, based on content, and
	* this will be first in the list. Best-guess iwad and engine is
	* set from WadStats analysis. This can be checked with getIwad/
	* Engine, and modified with setIwad/Engine.
	*/
	void processWads(TaskProgress* tp);

	/*!
	* Current iwad (game) type of the files being processed. A
	* best-guess is set by processWads, and it can be set explicitly
	* with setIwad.
	*/
	IwadType getIwad() { return iwad; }

	/*!
	* Current engine (port) type of the files being processed. A
	* best-guess is set by processWads, and it can be set explicitly
	* with setEngine.
	*/
	EngineType getEngine() { return engine; }

	/*!
	* Explicitly set iwad type. This affects processing of maps.
	* It should be called after processWads (which sets a best-guess
	* value) and before findThingDefs.
	*/
	void setIwad(IwadType iwd) { iwad=iwd; }

	/*!
	* Explicitly set engine type. This can affect processing of maps.
	* It should be called after processWads (which sets a best-guess
	* value) and before findThingDefs.
	*/
	void setEngine(EngineType eng) { engine=eng; }

	/*!
	* Load definition of map thing properties, based on iwad and engine
	* type (call setIwad/Engine first if you want to set these explicitly).
	* Any dehacked patch, decorate scripts or similar which are found in
	* the initial file are also processed at this stage, modifying the
	* iwad/engine default thing definitions.
	*/
	void findThingDefs(TaskProgress* tp);

	/*!
	* Get a reference to the full list of Thing definitions currently
	* held by the WadReader. This is the list loaded based on iwad/engine,
	* plus changes made for any dehacked patch, decorate, etc. (changed
	* ThingDefs have the modified flag set). It's possible to modify
	* ThingDefs in the list and add new ThingDefs (the modified flag
	* should be set on new/modified entries). When maps are processed,
	* creating MapStats, the current ThingDefList is used.
	*/
	ThingDefList* getThingDefs() { return thingDefs; }

	//Load a custom set of thingDefs?
	//void loadThingDefs(wxString fileName);

	/*!
	* Write information about processed files to a TextReport. This
	* includes archive contents, wad stats and dehacked patch info.
	*/
	void printReport(TextReport* reportView);

	/*!
	* Process an additional resource file, adding a WadStats object to
	* the list of analyzed wads. pk3=true for a zip archive, otherwise
	* the file must be in wad format. If reportView is non-null, the
	* result is added to the end of the current report.
	*/
	void addWad(wxString file, bool pk3, TextReport* reportView, TaskProgress* tp);

	/*!
	* Process a Dehacked patch file, adding a DehackedParser object
	* to the results. If reportView is non-null, the Dehacked
	* analysis is added to the end of the current report. Note that
	* only one Dehacked patch is supported at a time.
	*/
	void addDeh(wxString file, TextReport* reportView, TaskProgress* tp);

	/*!
	* Number of processed content files (wad/pk3 - WadStats objects).
	*/
	int numberOfWads() { return wadStatList->size(); }

	/*!
	* Get a list of the wads currently processed (WadStats objects).
	*/
	vector<wxString>* getWadNames();

	/*!
	* Remove the WadStats for one of the processed wads, based on
	* its index in the list.
	*/
	void removeWad(int index);

	/*!
	* Get MD5 hash of the main file.
	*/
	unsigned char* getMainDigest() { return wadStatList->at(0)->md5Digest; }

	/*!
	* Get name of the main file.
	*/
	string getMainFileName() { return wadStatList->at(0)->fileName; }

	/*!
	* Total number of maps in the processed wads.
	*/
	int numberOfMaps();

	/*!
	* Get a list of all maps in the analyzed wad files. If there is
	* only a single wad file with maps, the names returned are simply
	* the lump names, such as "MAP01". If there are maps in multiple
	* files, the map names are prefixed with the file names.
	*/
	vector<wxString>* getMapNames();

	/*!
	* Get the total lump size, in bytes, of a specific map (mapName
	* is lump name) in a specific wad (fileName is name without path).
	*/
	int getMapLumpSize(string fileName, string mapName);

	/*!
	* Load and process a specific map (lump name mapName) in a
	* specific wad (fileName is name without path). fileName can be
	* empty, to look for the map in all analyzed wad files.
	*/
	MapStats* processMap(string fileName, string mapName, TaskProgress* tp);

	/*!
	* Creates a new WadEntry data object with MapEntry objects
	* for each map in the wad. This is called after processWads
	* and findThingDefs, creating the WadEntry for the current
	* results. If there are multiple wads for the current entry,
	* one is always the primary one, while the rest are additional
	* files which also contribute their content to the WadEntry.
	* imgFileFolder is where to put map line drawings. A template
	* MapEntry object can be provided, giving its values to each
	* MapEntry object (for properties which may have common values,
	* such as authors and game modes).
	*/
	WadEntry* createEntries(wxString& imgFileFolder, TaskProgress* tp, MapEntry* mapTempl = NULL);

	/*!
	* Update an existing WadEntry object and its MapEntry objects, with
	* the current results from processed wad(s). MapEntry objects in the
	* existing WadEntry are matched with maps from processed wad(s) based
	* on map names. New maps not found in WadEntry will have new entries
	* added to it. Old entries with names NOT found in the current results
	* will stay in the WadEntry but have the ownFlag OFLG_DELETE set, to
	* mark them for deletion. And all entries marked for deletion will be
	* at the back of the mapPointers vector, after all entries which should
	* stay. The function returns true if any maps were marked for deletion.
	* The caller must then go through the maps at the back of the WadEntry
	* and delete maps from the database with the flag set. Note that if a
	* wad used to have just MAP01, and now has this map moved to MAP02 with
	* a new MAP01, the old MapEntry for MAP01 will be updated to reflect the
	* new MAP01 while a new entry will be added for MAP02.
	*/
	bool updateEntries(WadEntry* wadEntry, wxString& imgFileFolder, TaskProgress* progress, bool newEntry=false);

	/*!
	* Update a single MapEntry object with data from a MapStats
	* object (does not update map images).
	*/
	void updateMapEntry(MapEntry* mapEntry, MapStats* mapStats);


	private:
		/*! Creates a WadStats object with an analysis of a wad or pk3 file. */
		WadStats* processWad(wxString fileName, bool pk3, TaskProgress* tp);

		/*! Creates a DehackedParser object with an analysis of a deh file. */
		DehackedParser* processDeh(wxString fileName, TaskProgress* tp);

		/*! Creates a DehackedParser object from a Dehacked lump in a wad. */
		DehackedParser* processDeh(WadStats* wadStats, TaskProgress* tp);

		/*! Checks for DECORATE lump in a wad, parsing it if found. */
		bool processDecorate(WadStats* wadStats, TaskProgress* tp);

		/*! Creates a MapStats object from a map entry in a wad. */
		MapStats* loadMap(WadContentX* mapEntry, TaskProgress* tp);

		/*! Set WadEntry content flags from WadStats. */
		void setWadFlags(WadEntry* wadEntry, WadStats* wadStats);

		/*! Draw a line map, storing it to a PNG file. */
		void storeMapImage(MapStats* mapStats, wxString& fileName);

		/*! Does the actual drawing. */
		void drawMap(MapStats* mapStats, wxDC& dc);


	wxFileSystem* fileSystem; //Used to get wxFSFile objects for files, to get date
	wxString tempFolder; //Temporary file storage
	wxString failedFolder; //For files we can't process
	wxString thingFiles[5]; //Files to load ThingDefs

	int thingType; //Current ThingDef type: 0=None/custom, 1=Doom, 2=ZDoom, ...
	ThingDefList* thingDefs; //Current ThingDefs

	wxFileName mainFile; //Path/name of initial file (can be zip)
	IwadType iwad; //What game/iwad the resources are for
	EngineType engine; //What engine/port is needed
	WadArchive* archive; //If file is archive
	WadStatAspects* aspects; //Which aspects to provide for DB entries
	vector<WadStats*>* wadStatList; //Each processed file
	DehackedParser* dehacked; //Parsed dehacked patch
	DecorateParser* decorate; //Parsed Decorate lump
	MapinfoParser* mapinfo; //Parsed MAPINFO lump
};

#endif
