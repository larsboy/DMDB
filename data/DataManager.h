/*!
* \file DataManager.h
* \author Lars Thomas Boye 2007
*
* The DataManager manages the data objects of the database application,
* in memory and with file persistence. The core data objects are those
* defined in DataModel.h. In addition to the large DataManager class,
* associated constants and helper classes are defined here.
*
* The following types of objects are handled by DataManager:
* - People (AuthorEntry), with separate textual descriptions.
* - Wads (WadEntry), with separate textual descriptions and additional files.
* - Maps (MapEntry), with separate textual descriptions and additional files.
* - Map tags (TagEntry).
* - Wad/map filters/lists (DataFilter).
*
* For persistence, the database instance has a folder in the local file
* system. The core objects are stored in a set of files here, and additional
* files are stored in sub-folders. All core wad and map objects are kept in
* memory, with changes being persisted to file. A DataFilter is used to
* select and sort a subset of wads and maps.
*/

#ifndef DATAMANAGER_H //Avoid problems with multiple includes
#define DATAMANAGER_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/textfile.h>

#include "DataModel.h"
#include "DataFilter.h"
#include "MapStatistics.h"
#include "StatisticSet.h"
#include "../LtbUtils.h"


/*! File storing the author table of the core DB. */
const wxString FILE_AUTHORDB("people.dmdb");

/*! File storing the textual descriptions for the authors. */
const wxString FILE_AUTHORTEXT("ptext.dmdb");

/*! File storing the map tags. */
const wxString FILE_TAGS("tags.dmdb");

/*! File storing the wad table of the core DB. */
const wxString FILE_WADDB("wads.dmdb");

/*! File storing the map table of the core DB. */
const wxString FILE_MAPDB("maps.dmdb");

/*! File storing the personal fields for wad entries. */
const wxString FILE_WADOWN("mywads.dmdb");

/*! File storing the personal fields for map entries. */
const wxString FILE_MAPOWN("mymaps.dmdb");

/*! File storing DataFilters (list definitions). */
const wxString FILE_VIEWS("lists.dmdb");


/*! Version of file format for wad table. */
const unsigned char WADDB_FILEV = 1;

/*! Version of file format for map table. */
const unsigned char MAPDB_FILEV = 1;

/*! Version of file format for view table. */
const unsigned char VIEWS_FILEV = 1;

/*! Maximum bytes in author text entry. */
const int AUTHOR_DESC_LIMIT = 2048;

/*! Text entry will be whole number of blocks. */
const int AUTHOR_DESC_BLOCK = 100;

/*! Bytes per tag string. */
const unsigned char DEFAULT_TAG_LENGTH = 25;

//! Defines the ways author entries can be named and sorted in lists.
enum AuthorNaming {
	AUTHNAME_FIRST_LAST, //!< Prefer real name, start with first name
	AUTHNAME_LAST_FIRST, //!< Prefer real name, start with last name
	AUTHNAME_ALIAS //!< Prefer alias
};

/*! String representation of flag value 'true'. */
const wxString FLAGSTR_TRUE = "*";

/*! String representation of flag value 'false'. */
const wxString FLAGSTR_FALSE = "";

/*!
* Interface for DataManager listener, to be notified of
* updates in the numbers of different entries.
*/
class DataStatusListener
{
	public:
	/*! Called when the current wad filter changes. */
	virtual void onWadFilter(const wxString& name, unsigned int wadCount) = 0;

	/*! Called when the current map filter changes. */
	virtual void onMapFilter(const wxString& name, unsigned int mapCount) = 0;

	/*! Called on any change in number of objects. */
	virtual void onTotalCounts(long wads, long maps, long auths) = 0;
};


/*!
* Wrapper for the current, filtered list of wads or maps.
* We use std::list, with efficient iteration, sorting and
* deletion, but we also need to operate based on index, so
* this utility class gives us an index, kept in sync with
* an iterator.
*/
template<class T> class ListWrapper
{
	public:
	ListWrapper() : wIterIndex(-1) {
		wList = new list<T>();
	};

	~ListWrapper() {
		if (wList != NULL)
			delete wList;
	};

	void fromVector(vector<T>& vec, int amount = -1) {
		if (wList!=NULL && wList->size()>0) {
			delete wList;
			wList = new list<T>();
		}
		if (amount==-1) amount = vec.size();
		for (int n=0; n<amount; n++) {
			wList->push_back(vec.at(n));
		}
		reset();
	};

	void add(T item) { wList->push_back(item); };

	void reset() {
		if (wList->size() > 0) {
			wIter = wList->begin();
			wIterIndex = 0;
		} else {
			wIterIndex = -1;
		}
	};

	T entry() { return *wIter; };

	void erase() {
		if (wIterIndex > -1) {
			wList->erase(wIter);
			reset();
		}
	}

	bool isFirst() {
		return (wIterIndex == 0);
	};

	bool isLast() {
		return (wIterIndex>-1 && wIterIndex==(wList->size()-1));
	};

	bool previous() {
		if (wIterIndex==-1 || wIter==wList->begin())
			return false;
		wIter--;
		wIterIndex--;
		return true;
	};

	bool next() {
		if (wIterIndex==-1 || wIterIndex==(wList->size()-1))
			return false;
		wIter++;
		wIterIndex++;
		return true;
	};

	void setIndex(long index) {
		if (wIterIndex>-1 && index>=0 && index<wList->size()) {
			while (index > wIterIndex) {
				wIter++;
				wIterIndex++;
			}
			while (index < wIterIndex) {
				wIter--;
				wIterIndex--;
			}
		}
	};

	bool (*comp)(const T, const T); //Map sort

	void sort(bool rev) {
		wList->sort(*comp);
		if (rev) wList->reverse();
		reset();
	};

	unsigned int getSize() {
		if (wList==NULL) return 0;
		else return wList->size();
	};

	long getIndex() { return wIterIndex; };

	private:
	list<T>* wList; //Sorted/filtered list
	typename list<T>::iterator wIter;
	long wIterIndex;
};

/*!
* The DataManager manages the data objects of the database application,
* in memory and with file persistence. It is configured with a folder
* housing the persisted data, with the load-method initializing content
* in memory. Objects can be retrieved, added and deleted. The manager has
* a set of methods for each type of data, and the strategies for keeping
* objects in memory, and for persistence, are different for different types.
*
* Wad/map designers are represented by AuthorEntry objects, and an optional
* text description. The manager keeps a complete list of AuthorEntries, but
* the textual descriptions are loaded from file for a specific author when
* needed. The AuthorEntry mainly keeps the identity of the author, which
* has first and last name as well as aliases. DataManager keeps track of how
* to represent and sort these entries - whether we want to use first name,
* last name or alias. getAuthorString should be used whenever the author
* entry is displayed to the end user, as it resolves the name based on this
* setting.
*
* TagEntries are simple objects referenced from MapEntries, used to say
* something about the style of a map. The complete list is kept by the
* DataManager.
*
* Wads and maps are the main objects. A wad has a list of maps, with each
* map belonging to exactly one wad. The core data is represented with
* WadEntry and MapEntry objects. A part of the data in each of these objects
* are considered "personal", used by a specific user to track such things as
* their rating of the item and whether they have played and beaten it. This
* personal information is persisted separately from the rest, with the idea
* being that the non-personal data can be shared between users and the
* personal part is typically changed by the individual user without changing
* the other data. Wads and maps can have textual descriptions in addition to
* the core data. This is represented separately, with a WadText object
* keeping the text entries for a wad and its maps. Additional data, such
* as screenshots and map drawings, are stored as files in folders defined by
* DataManager.
*
* The DataManager keeps sorted and filtered lists of wads and maps, to be
* used by user interface views. A DataFilter object is used to filter items,
* selecting a subset. Filtering can be done by rules, or with a
* DataListFilter which specifies a specific set of items. DataManager keeps
* a set of DataListFilters which are persisted. DataManager also handles
* sorting of the current list, sorting on any field of the wads or maps.
*
* The DataManager can also produce statistics - a MapStatistics for all of
* the current list and StatisticSets with statistics according to various
* categories.
*/
class DataManager
{
	public:
	/*!
	* Created with a DataStatusListener to be notified of changes in content.
	*/
	DataManager(DataStatusListener* l);
	~DataManager();

	/*!
	* Sets the file folder where the data should be stored.
	* This must be set before invoking any load/save operations.
	*/
	void setFolder(wxString folder) { dbFolder=folder; }

	/*!
	* Folder for storing map drawings (sub-folder of database folder).
	*/
	wxString getMapImgFolder() { return dbFolder+wxFILE_SEP_PATH+"mapimg"; }

	/*!
	* Folder for storing screenshots (sub-folder of database folder).
	*/
	wxString getScreenshotFolder() { return dbFolder+wxFILE_SEP_PATH+"sshot"; }

	/*!
	* Folder for storing text files (sub-folder of database folder).
	*/
	wxString getTextFolder() { return dbFolder+wxFILE_SEP_PATH+"text"; }

	/*!
	* Folder for storing files temporarily while processing.
	*/
	wxString getTempFolder() { return dbFolder+wxFILE_SEP_PATH+"temp"; }

	/*!
	* Performs the necessary loading of persisted resources
	* into memory. This will typically be the full set of core
	* objects.
	*/
	void load();


	//************************ Author persistence public ************************

	/*!
	* Must be called before reading or saving author text
	* descriptions, to initiate access to the file. An initial
	* file is created if none exists. Be sure to call
	closeAuthorTextFile when access is no longer needed.
	*/
	void openAuthorTextFile();

	/*!
	* Close the author text description file.
	*/
	void closeAuthorTextFile();

	/*!
	* Get the text description of an author. This is loaded
	* from file if not already in memory.
	*/
	wxString getAuthorText(AuthorEntry* ae);

	/*!
	* Update the text entry of an author. The text will replace
	* the current entry, and the change will be persisted.
	*/
	void authorTextModified(AuthorEntry* ae, wxString text);

	/*!
	* Add a new author entry to the database. Both the core data
	* object and an optional descriptive text is provided. The
	* core data object is added to the master list in memory but
	* not persisted until saveAuthors() is called. The text is
	* persisted automatically.
	*/
	void addAuthor(AuthorEntry* newEntry, wxString text);

	/*!
	* Mark an AuthorEntry as modified. This must be called whenever
	* the names/aliases of an existing AuthorEntry have changed.
	* Note that the change is not persisted until saveAuthors() is
	* called.
	*/
	void authorModified(AuthorEntry* author);

	/*!
	* Delete an AuthorEntry from the database, including file updates.
	* Any references to the author is removed from map entries, and from
	* author group entries (any groups which become empty are also deleted).
	* The author text file as well as the core file is updated, persisting
	* the changes (the text file will be open if it was not already). Map
	* changes must be persisted separately.
	*/
	void deleteAuthor(AuthorEntry* author);

	/*!
	* Delete an AuthorEntry from the database, replacing all references to
	* it with references to authorKeep. See deleteAuthor - this is the same
	* except that references in maps and any author group entries are
	* replaced with references to authorKeep.
	*/
	void mergeAuthors(AuthorEntry* authorDelete, AuthorEntry* authorKeep);

	/*!
	* Persist any changes to the core author database to file.
	* This will append new entries to the existing file, or
	* write the whole file if there are changes in existing
	* entries.
	*/
	void saveAuthors();


	//************************ Author in-memory public ************************

	/*!
	* Controls whether real name or alias is preferred for
	* naming author entries, and whether to sort on first or last
	* name. This affects sort order and the results of getAuthorString().
	*/
	void setAuthorNaming(AuthorNaming an);

	/*!
	* Returns the name or alias, based on what is defined and
	* the author naming preference (setAuthorNaming).
	*/
	wxString getAuthorString(AuthorEntry* ae);

	/*!
	* Total number of author entries.
	*/
	unsigned int getAuthorCount() { return authorMaster->size(); }

	/*!
	* Number of maps where the author of the given id is an author
	* (including in group).
	*/
	unsigned int getMapAuthorCount(uint32_t dbid);

	/*!
	* Sorted/filtered list of author entries.
	*/
	list<AuthorEntry*>* getAuthorList() { return authorList; }

	/*!
	* Get an AuthorEntry based on index in the current sorted
	* list.
	*/
	AuthorEntry* getAuthor(long index);

	//AuthorEntry* getAuthorId(uint32_t dbid);

	/*!
	* Get the index in the current sorted list of an AuthorEntry.
	*/
	long getAuthorIndex(uint32_t dbid);

	/*!
	* Get index in the current sorted list of the first author who's
	* name (according to the current naming scheme) starts with the
	* given string, or the next name if no name starts with
	* this string. So getAuthorIndex("H") to go to H in the list.
	*/
	int getAuthorIndex(wxString nameStart);

	/*!
	* Looks for an author with either the given name, OR the
	* given alias as its primary alias.
	*/
	AuthorEntry* findAuthor(wxString namef, wxString namel, wxString alias);

	/*!
	* Filter the author list so that it only contains entries which contains
	* the filterStr. filterStr is checked against all names and aliases.
	* If the list is already filtered and the new filterStr has the old as
	* its prefix, set update=true to filter on the filtered list instead of
	* the full set of tags, for better performance. getTagList() to get the
	* new list.
	*/
	void filterAuthorList(wxString filterStr, bool update=false);

	/*!
	* Remove any filter on the author list, so that getAuthorList returns
	* all author entries.
	*/
	void clearAuthorFilter();


	//************************ Tags public ************************

	/*!
	* Add a new tag to the database. The TagEntry object is added
	* to the master list in memory but not persisted until saveTags()
	* is called.
	*/
	void addTag(TagEntry* newEntry);

	/*!
	* Should be called when a tag string has changed. Re-sorts
	* the tag list. Note that the change is not persisted until
	* saveTags() is called.
	*/
	void tagModified(TagEntry* tag);

	/*!
	* Delete tag from database, removing all references to it from map
	* entries. Note that tags are not truly deleted, they just have their
	* value set to the empty string, and this will prevent them from
	* being included in the tag list.
	*/
	void deleteTag(TagEntry* tag);

	/*!
	* Merge two tags, which means removing one and replacing all references
	* to it in maps with the other. tagDelete is deleted (see deleteTag).
	* References to tagDelete in map entries are replaced with references
	* to tagKeep (given by dbid).
	*/
	void mergeTags(TagEntry* tagDelete, uint16_t tagKeep);

	/*!
	* Store the current list of tags to file.
	*/
	void saveTags();

	/*!
	* Sorted/filtered list of tag entries.
	*/
	list<TagEntry*>* getTagList() { return tagList; }

	/*!
	* Get a tag based on ID.
	*/
	string getTagString(uint16_t dbid);

	/*!
	* Get a TagEntry based on index in the current sorted
	* list.
	*/
	TagEntry* getTagEntry(int index);

	/*!
	* Get the index in the current sorted list of a TagEntry.
	*/
	int getTagIndex(uint16_t dbid);

	/*!
	* Get index in the current sorted list of the first tag starting
	* with the given string, or the next tag if no tag starts with
	* this string. So getTagIndex("H") to go to H in the list.
	*/
	int getTagIndex(wxString tagStart);

	/*!
	* Looks for a TagEntry with the given tag string.
	*/
	TagEntry* findTag(string tag);

	/*!
	* Filter the tag list so that it only contains tags which contains
	* the filterStr. If the list is already filtered and the new filterStr
	* has the old as its prefix, set update=true to filter on the filtered
	* list instead of the full set of tags. getTagList() to get the new list.
	*/
	void filterTagList(wxString filterStr, bool update=false);

	/*!
	* Remove any filter on the tag list, so that getTagList returns
	* all tags.
	*/
	void clearTagFilter();


	//************************ Wads&Maps persistence public ************************

	/*!
	* Add a new wad entry to the database, along with all
	* its map entries. The entries are added to the wad and
	* map master lists in memory, but not persisted until
	* saveWadsMaps() is called.
	*/
	void addWad(WadEntry* newEntry);

	/*!
	* Check if a WadEntry is modified. This must be called whenever
	* the fields of an existing WadEntry may have changed, and checks
	* if the WadEntry is marked as modified by flags. It also checks
	* each MapEntry of the wad. Note that the change is not persisted
	* until saveWadsMaps() is called.
	*/
	void wadModified(WadEntry* wad);

	/*!
	* Check if a MapEntry is modified. This must be called whenever
	* the fields of an existing MapEntry may have changed, and checks
	* if the MapEntry is marked as modified by flags.  Note that the
	* change is not persisted until saveWadsMaps() is called.
	*/
	void mapModified(MapEntry* me);

	/*!
	* Returns true if there are known changes to wads or maps not yet
	* saved.
	*/
	bool unsavedWadMapChanges();

	/*!
	* Persist any changes to the core wads and maps database to file.
	* This will append new entries to the existing files, or write
	* the whole files if there are changes in existing entries.
	*/
	void saveWadsMaps();

	/*!
	* Get a WadText entry for a wad, housing descriptive text about
	* that wad and its maps. This is loaded from file if previously
	* persisted, otherwise an empty object.
	*/
	WadText* getWadText(WadEntry* wad);

	/*!
	* Persist a WadText object to file. The object should not be empty.
	*/
	void saveWadText(WadText* wadText);

	/*!
	* Delete a map entry from the database. If the wad entry of this map
	* has no other maps, the whole wad entry is deleted, as we do not
	* allow empty wads. All references to the map are removed, and
	* associated files are deleted. Include the map's index in the
	* currently sorted list, if available.
	*/
	void deleteMap(MapEntry* me, long index=-1);

	/*!
	* Delete a wad entry from the database, along with all its maps entries.
	* All references to the wad and maps are removed, and associated files
	* are deleted. Include the wad's index in the currently sorted list, if
	* available.
	*/
	void deleteWad(WadEntry* we, long index=-1);

	/*!
	* Delete the image files for a specific map entry, if found.
	*/
	void deleteMapFiles(MapEntry* me);

	/*!
	* Delete files stored in addition to the core database entry
	* for a wad, including for its maps. (images and text file).
	* This does NOT delete core wad and map entries from the database,
	* just the additional files.
	*/
	void deleteWadFiles(WadEntry* wad);

	/*!
	* Rename additional files which are identified with wad MD5 hash.
	* Used when the hash of the Wadentry is about to change, giving
	* the new hash to use for file names. newHash should not match any
	* existing wad entries.
	*/
	void renameFiles(WadEntry* wad, string newHash);


	//************************ Wads&Maps in-memory public ************************

	/*!
	* We must always have a DataFilter each for wads and maps, controlling
	* the contents and sorting of the filtered and sorted list.
	*/
	void initDataFilters(DataFilter* wadFilter, DataFilter* mapFilter);

	/*!
	* Change the DataFilter to filter either wads or maps (type is given
	* by the DataFilter object). If the given DataFilter is different
	* from that currently in effect or dataFilter->isChanged is true, the
	* sorted list is updated, and the method returns true. If it returns
	* false, there was no change.
	*/
	bool changeDataFilter(DataFilter* dataFilter);

	/*!
	* In addition to the main wad and map filters, the wad and map lists
	* can also have a string search filter, to further limit the list to
	* entries matching the filter string. The same filter string is applied
	* to both wads and maps, but only the list of the given filterType is
	* updated now.
	*/
	void setTextFilter(wxString filterStr, int filterType, bool update=false);

	/*!
	* Returns the current wad filter.
	*/
	DataFilter* getWadFilter() { return currentWadFilter; }

	/*!
	* Returns the current map filter.
	*/
	DataFilter* getMapFilter() { return currentMapFilter; }

	/*!
	* Update the sorting of the current wad filter, sorting entries
	* on the given field. Not all fields defined by WadMapFields
	* have sort algorithms (we don't sort on boolean fields). It
	* returns false if the sortField doesn't support sorting.
	*/
	bool setWadSort(WadMapFields sortField, bool reverse=false);

	/*!
	* Update the sorting of the current map filter, sorting entries
	* on the given field. Not all fields defined by WadMapFields
	* have sort algorithms (we don't sort on boolean fields). It
	* returns false if the sortField doesn't support sorting.
	*/
	bool setMapSort(WadMapFields sortField, bool reverse=false);

	/*!
	* Sorted/filtered list of wad entries.
	*/
	ListWrapper<WadEntry*>* getWadList() { return wadList; }

	/*!
	* Sorted/filtered list of map entries.
	*/
	ListWrapper<MapEntry*>* getMapList() { return mapList; }

	/*!
	* Number of entries in the sorted/filtered list of wads.
	*/
	unsigned int getWadSize() { return wadList->getSize(); }

	/*!
	* Number of entries in the sorted/filtered list of maps.
	*/
	unsigned int getMapSize() { return mapList->getSize(); }

	/*!
	* Get a WadEntry based on index in the current sorted
	* list.
	*/
	WadEntry* getWad(long index);

	/*!
	* Get index in the current sorted wad list based on the strStart.
	* The list must be sorted on a string field (title or filename),
	* and searches for the first entry who's string field starts with
	* the given string. So getWadIndex("H") when sorted on title to
	* go to H in the list. Otherwise it returns -1.
	*/
	int getWadIndex(wxString strStart);

	/*!
	* Find a WadEntry (in the master list) based on dbid.
	*/
	WadEntry* findWad(uint32_t id);

	/*!
	* Find a WadEntry (in the master list) based on md5Digest.
	*/
	WadEntry* findWad(unsigned char* md5Digest);

	/*!
	* Find a WadEntry (in the master list) based on fileName.
	*/
	WadEntry* findWad(string fileName);

	/*!
	* Get a MapEntry based on index in the current sorted
	* list.
	*/
	MapEntry* getMap(long index);

	/*!
	* Get index in the current sorted map list based on the strStart.
	* The list must be sorted on a string field (such as name or title),
	* and searches for the first entry who's string field starts with
	* the given string. So getMapIndex("H") when sorted on title to
	* go to H in the list. Otherwise it returns -1.
	*/
	int getMapIndex(wxString strStart);

	/*!
	* Find a MapEntry (in the master list) based on dbid.
	*/
	MapEntry* findMap(uint32_t id);

	/*!
	* Returns highest dbid in current wad entries.
	*/
	uint32_t getWadIdEnd() { return nextWadId-1; }

	/*!
	* Returns highest dbid in current map entries.
	*/
	uint32_t getMapIdEnd() { return nextMapId-1; }

	/*!
	* Convert the byte rating of wads and maps into the currently
	* used scale.
	*/
	int scaleRating(unsigned char rat);

	/*!
	* This is the inverse of scaleRating, converting the scaled
	* value back to the byte value stored internally.
	*/
	unsigned char ratingScale(int scaled);

	/*!
	* Get a wxString representation of one of the fields of
	* a WadEntry. Fields are WadMapFields below WAD_END_MAP_START.
	*/
	wxString getWadField(WadEntry* wad, WadMapFields field);

	/*!
	* Get a wxString representation of one of the fields of
	* a MapEntry, or the WadEntry it belongs to. WadMapFields
	* below WAD_END_MAP_START gives values from the wad, while
	* those above are the fields of the map itself.
	*/
	wxString getMapField(MapEntry* wad, WadMapFields field);

	/*!
	* List of all wads sorted by title, optionally filtered to only
	* include those with titles matching filterStr. Useful for listing
	* all wads, such as for a wad selection dialog. Include fromOld to
	* select from existing list instead of master list.
	*/
	ListWrapper<WadEntry*>* getWadTitleList(wxString filterStr, ListWrapper<WadEntry*>* fromOld=NULL);


	//************************ DataFilters public ************************

	/*!
	* Add a new DataFilter object. Based on its type, it will be
	* placed in one of the lists of the DataManager.
	*/
	void addDataFilter(DataFilter* dataFilter);

	/*!
	* Persist DataFilters to file.
	*/
	void saveDataFilters();

	/*!
	* Remove an existing DataFilter object from the lists of the
	* DataManager. Note that this does not delete the DataFilter object
	* pointed to (this should be done by the caller).
	*/
	void removeDataFilter(DataFilter* dataFilter);

	/*!
	* Remove item from current DataListFilter, based on index in the
	* sorted list. type 0 for wad, 1 for map. Returns true if the item
	* was found and removed.
	*/
	bool removeFromListFilter(int type, long index);

	/*!
	* Remove the WadEntry with the given dbid from all wad lists. Returns
	* true if any lists where modified (entry was found and removed).
	*/
	bool removeWadFromFilters(uint32_t dbid);

	/*!
	* Remove the MapEntry with the given dbid from all map lists. Returns
	* true if any lists where modified (entry was found and removed).
	*/
	bool removeMapFromFilters(uint32_t dbid);

	/*!
	* All DataListFilters for wads.
	*/
	list<DataListFilter*>* getWadLists() { return wadLists; }

	/*!
	* All DataListFilters for maps.
	*/
	list<DataListFilter*>* getMapLists() { return mapLists; }


	//************************ Statistics public ************************

	/*!
	* Generate a MapStatistics object with statistics for all maps
	* in the current map or wad list. filterType is 0 for wad list,
	* 1 for map list.
	*/
	MapStatistics* getStatistics(int filterType);

	/*! Get statistics categorized by year of wad. */
	StatisticSet* getYearStats(int filterType);

	/*! Get statistics categorized by which iwad the wad targets. */
	StatisticSet* getIwadStats(int filterType);

	/*! Get statistics categorized by which engine the wad targets. */
	StatisticSet* getEngineStats(int filterType);

	/*! Get statistics categorized by own rating of maps. */
	StatisticSet* getRatingStats(int filterType);

	/*! Get statistics categorized by difficulty rating of maps. */
	StatisticSet* getDifficultyStats(int filterType);

	/*! Get statistics categorized by wad playstyle. */
	StatisticSet* getPlaystyleStats(int filterType);

	/*! Get statistics categorized by authors. */
	StatisticSet* getAuthorStats(int filterType);

	/*! Get statistics categorized by style tags. */
	StatisticSet* getTagStats(int filterType);


	private:
		//************************ Author private ************************

		/*! Writes all author ids + text offsets to the given file. */
		void writeAuthorTextDir(wxFile* file);

		/*! Loads the textOffset entries of the author text file. */
		void loadAuthorTextDir();

		/*! Append a new author text entry at the end of the file. */
		void addAuthorText(AuthorEntry* newEntry, wxString text);

		/*! Writes a new version of the author text file for the current authorText. */
		void rewriteAuthorText();

		/*!
		* Remove the AuthorEntry from any group which has its reference,
		* replacing it with repAuth (none by default). This is done as part
		* of deletion/merging of the AuthorEntry. If removing it from a group
		* leaves the group empty, the group will be marked for deletion.
		*/
		void removeAuthorFromGroups(AuthorEntry* auth, AuthorEntry* repAuth=NULL);

		/*!
		* Does the actual deletion of author entries flagged for deletion.
		*/
		void authorDeleteImpl();

		/*! Writes a single AuthorEntry to file. */
		void writeAuthor(wxOutputStream* file, AuthorEntry* entry);

		/*! Loads the core author objects (authorMaster). */
		void loadAuthors();

		/*! Create the sorted authorList with entries from authorMaster. */
		void makeAuthorList();

		/*! Finds the author entry index in the master list, based on dbid. */
		long getAuthorMasterIndex(uint32_t id);

		/*! Finds the author entry in the master list, based on dbid. */
		AuthorEntry* getAuthorMasterEntry(uint32_t id);

		//************************ Tags private ************************

		/*! Loads the list of tags from file. */
		void loadTags();

		/*! Create the sorted tagList with entries from tagMaster. */
		void makeTagList();

		//************************ Wads&Maps private ************************

		/*! Writes a single WadEntry to file, except "own" fields. */
		void writeWad(wxOutputStream* file, WadEntry* entry);

		/*! Writes the "own" fields of a single WadEntry to file. */
		void writeWadOwn(wxOutputStream* file, WadEntry* entry);

		/*! Writes a single MapEntry to file, except "own" fields. */
		void writeMap(wxOutputStream* file, MapEntry* entry);

		/*! Writes the "own" fields of a single MapEntry to file. */
		void writeMapOwn(wxOutputStream* file, MapEntry* entry);

		/*! Loads the core wad objects, incl. personal data (wadMaster). */
		void loadWads();

		/*! Loads the core map objects, incl. personal data (mapMaster). */
		void loadMaps();

		/*! Create the sorted wadList with entries from wadMaster. */
		void makeWadList(bool update=false);

		/*! Create the sorted mapList with entries from mapMaster. */
		void makeMapList(bool update=false);

		/*! Finds the wad entry in the master list, based on dbid. */
		WadEntry* getWadMasterEntry(uint32_t id);

		/*! Remove a WadEntry from the master list. The WadEntry object is not deleted. */
		void removeWadMaster(uint32_t id);

		/*! Remove a MapEntry from the master list. The MapEntry object is not deleted. */
		void removeMapMaster(uint32_t id);

		/*! Remove the given tagId from all maps, replacing it with repId (0 by default). */
		void removeTagFromMaps(uint16_t tagId, uint16_t repId=0);

		/*! Remove the given AuthorEntry from all maps, replacing it with repAuth (NULL by default). */
		void removeAuthorFromMaps(AuthorEntry* auth, AuthorEntry* repAuth=NULL);

		/*! Remove any basedOn-references to the given dbid from map entries. */
		void removeBasedOn(uint32_t dbid);

		//************************ DataFilters private ************************

		/*! Writes a DataFilter object to file. */
		void writeDataFilter(wxOutputStream* file, DataFilter* filter);

		/*! Loads DataFilters (list definitions), or returns false if not found. */
		bool loadDataFilters();

		/*! Process each wad in current list with StatisticSet. */
		void makeStatisticsWads(StatisticSet* sset);

		/*! Process each map in current list with StatisticSet. */
		void makeStatisticsMaps(StatisticSet* sset);


	wxString dbFolder; //The files and folders are persisted here
	DataStatusListener* listener; //Call when entry counts change

	// Author core DB
	vector<AuthorEntry*>* authorMaster; //Master list
	uint32_t nextAuthorId; //Next unused id
	bool authorMod; //Unsaved changes, other than new
	long firstNewAuthor; //Index of first unsaved entry, or -1
	list<AuthorEntry*>* authorList; //Sorted/filtered list

	// For author text file
	uint32_t authorTextDir; //Offset of text file directory
	wxFile* authorTextFile; //When open
	long authorTextIndex; //Index in authorMaster for authorText
	wxString authorText; //An author description in memory

	// Tags
	unsigned char tagLength; //Max chars in tag
	vector<TagEntry*> tagMaster;
	list<TagEntry*>* tagList; //Sorted/filtered list

	// Wad core DB
	vector<WadEntry*> wadMaster; //Master list
	vector<WadEntry*>::iterator searchIter;
	uint32_t nextWadId; //Next unused id
	bool wadMod; //Unsaved changes, other than new
	bool wadOwnMod; //Unsaved changes to personal fields
	long firstNewWad; //Index of first unsaved entry, or -1
	ListWrapper<WadEntry*>* wadList; //Sorted/filtered list

	// Map core DB
	vector<MapEntry*> mapMaster; //Master list
	uint32_t nextMapId; //Next unused id
	bool mapMod; //Unsaved changes, other than new
	bool mapOwnMod; //Unsaved changes to personal fields
	long firstNewMap; //Index of first unsaved entry, or -1
	ListWrapper<MapEntry*>* mapList; //Sorted/filtered list

	// Wad/map text
	WadText* wadText;

	// DataViews
	list<DataListFilter*>* wadLists;
	list<DataListFilter*>* mapLists;
	ComboDataFilter* currentWadFilter; //main filter + search filter
	ComboDataFilter* currentMapFilter; //main filter + search filter

	TitleSearchFilter* wadTitleFilter; //To filter wadTitleList
};

#endif
