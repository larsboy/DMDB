/*!
* \file WadStats.h
* \author Lars Thomas Boye 2018
*
* WadStats handles the analysis of the main resource files of Doom-engine
* games. Such a resource file contains many "lumps", which are the
* individual resources and equivalent to files (a wad file collects all
* the little resource files into one file, which is more efficient for the
* file system). The WadStats class can be used to process a wad file, but
* is also a base class for adopting to modern resource file formats such
* as pk3. It processes the wad, categorizing all lumps and extracting
* useful information. There is no processing of maps here, this is found
* in MapStats, but we find and keep all directory entries for map lumps,
* which is the needed input to map analysis.
*
* In addition to the lumps, a wad file starts with a header and ends with a
* directory listing all lumps. The struct DirEntry holds an entry in this
* directory, with the name, size and offset within the file of the lump.
*
* All (non-empty) lumps are categorized according to the categories defined
* by the enum WadContentType. A WadContent object is created for each
* content type found, to keep some statistics, such as the number of lumps
* of this type. The exact usage depends on the type, and the extended
* sub-class WadContentX is used for types which need to retain additional
* data, such as all map directory entries for all maps in the wad.
*
* WadStats is the main class here, which both does the analysis of the file
* and keeps the results. It can produce a textual report of the analysis
* through the TextReport interface.
*/

#ifndef WADSTATS_H
#define WADSTATS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include "DataModel.h"
#include "MapinfoParser.h"
#include "IncludeParser.h"
#include "DecorateParser.h"
#include "TaskProgress.h"
#include "../gui/GuiBase.h"
#include "../TextReport.h"


/*!
* Entry in a directory of lumps or files. It is directly based on the
* lump directory in wads, naming and locating the lump in the file,
* but is also used for files in an archive (pk3, offset not used).
*/
struct DirEntry
{
   int32_t offset; //!< File offset to start of lump (not used for file in archive, -1 to not process)
   int32_t size; //!< Lump/file size in bytes, -1 to mark it as (to be) deleted
   string name; //!< Name of lump/file (max 8 characters for wad lump, full path/name for file)

   DirEntry() : offset(0), size(0), name() {}
   DirEntry(string nam) : offset(0), size(0), name(nam) {}
   DirEntry(const DirEntry& d): offset(d.offset), size(d.size), name(d.name) {}
};

/*!
* Each wad lump (or file in archive) is categroized as belonging to one
* of the categories defined by this enum. Categories are used for
* statistics, and for deciding on any further processing of the lump/file.
*/
enum WadContentType
{
	/*!
	* Map lumps. One WadContentX object for each map, with full path of
	* wad file. One DirEntry for each content lump, starting with marker
	* lump for map name.
	* THINGS, LINEDEFS, SIDEDEFS, VERTEXES, SEGS, SSECTORS, NODES
	* SECTORS, REJECT, BLOCKMAP.
	* Hexen/Zdoom: BEHAVIOR, optionally SCRIPTS / SCRIPTxx.
	* GL node lumps.
	* UDMF: TEXTMAP, DIALOGUE, ZNODES.
	* DOOM64: LEAFS, LIGHTS, MACROS
	*/
	WMAP,

	// --- Graphics ---
	WSPRITE, //!< Images between S*_START and S*_END markers
	WPATCH, //!< Images between P*_START and P*_END markers
	WFLAT, //!< Images (flats) between F*_START and F*_END markers
	WFONT, //!< Font gfx between FONT*_S and FONT*_E markers
	WTX, //!< ZDoom: Textures between TX_START and TX_END markers, D64: Textures between T_START and T_END markers
	WGFX, //!< Other Doom-format images (not in markers)
	WBITMAP, //!< Raw bitmap images, other than flats
	WPNAMES, //!< PNAMES lump listing patches, count is entries in list
	WTEXTURES, //!< Texture lists, count is total entries in lists. ZDoom: HIRESTEX
	WANIM, //!< Boom: ANIMATED, SWITCHES, Hexen/Zdoom: ANIMDEFS
	WPLAYPAL, //!< PLAYPAL lump

	/*!
	* COLORMAP lump, and engine-specific color maps and translation tables.
	* Boom: colormaps between C_START and C_END
	* Heretic: TINTTAB
	* Hexen: FOGMAP, TINTTAB, TRANTBL?
	*/
	WCOLORMAP,

	// --- Sound ---
	WMUS, //!< Music lumps (Doom format)
	WMIDI, //!< MIDI music lumps (Not original engine)
	WOTHERMUS, //!< Music of other formats
	WSFX, //!< Sound samples (Doom format)
	WWAV, //!< Sound samples, wav format
	WOTHERSND, //!< Sound samples in other formats
	WSPEAKER, //!< PC Speaker soundfx (Doom format)
	WSNDDEF, //!< Sound definition: SNDSEQ, SNDINFO, SNDCURVE (Heretic/Hexen/Zdoom)

	// --- Scripts ---
	WBEHAVIOR, //!< BEHAVIOR lumps for maps, to check if these have content
	WZSCRIPT, //!< ZSCRIPT lump and other lumps referenced by this
	WACS, //!< Other script lumps
	WDEHACKED, //!< Dehacked patch - WadContentX for each lump/file, with full path name of file
	WDECORATE, //!< DECORATE thing definitions - WadContentX for lump/file, with full path name of file
	WMAPINFO, //!< Map properties - WadContentX for lump/file, with full path name of file

	/*!
	* Other ZDoom definition files
	* ZDoom: DECALDEF, TERRAIN, GAMEINFO, SBARINFO
	*/
	WZDEF,

	WGLDEF, //!< GZDoom: GLDEFS (DOOMDEFS, HTICDEFS, HEXNDEFS, STRFDEFS)

	// --- Other ---
	WENDOOM, //!< ENDOOM lump, Boom: ENDBOOM, Strife: ENDSTRF
	WDEMOS, //!< Demos (DEMO? lumps)

	/*!
	* Text messages, Hexen: CLUS?MSG, WIN?MSG
	* Strife: T1TEXT, C1TEXT, LOG*
	* ZDoom: LANGUAGE
	*/
	WTEXT,

	/*!
	* Known lump names not matching other categories:
	* GENMIDI, DMXGUS, DMXGUSC, _DEUTEX_
	* ZDoom: KEYCONF
	* Heretic: LOADING
	* Strife: SERIAL
	*/
	WOTHER,

	/*!
	* Wad file found in archive. Not used for lumps in wad file.
	* This is a sub-file which will need to be processed separately
	* with its own WadStats. A single WadContentX object, with one
	* DirEntry for each wad file in the archive.
	*/
	WWAD,

	/*!
	* Unknown lumps - not matching any categories.
	* A single WadContentX object, with one DirEntry for each
	* unknown lump.
	*/
	WUNKNOWN,

	/*!
	* Lumps which break wad rules.
	* One WadContentX object for each error, with error message
	* and a DirEntry for the lump.
	*/
	WERROR,

	WEND //!< End of enum
};

//! Names of the WadContentType enum entries
const wxString wadContentNames[] = {
	"Maps", //WadContentX objects with marker lump names
	"Sprites",
	"Patches",
	"Flats",
	"Fonts",
	"Textures (ZDoom)",
	"Images",
	"Bitmaps",
	"PNAMES entries",
	"Texture list entries",
	"Animation",
	"PLAYPAL",
	"Colormaps/translations",
	"Music (MUS)",
	"Music (MIDI)",
	"Music (other)",
	"Sound effects",
	"Wav samples",
	"Sound (other)",
	"Speaker sounds",
	"Sound definition",
	"BEHAVIOR",
	"ZScript",
	"Other scripts",
	"Dehacked patch",
	"DECORATE",
	"MAPINFO",
	"ZDoom definitions",
	"GLDEFS",
	"ENDOOM",
	"Demos",
	"Text messages",
	"Other lumps",
	"Wad files",
	"Unknown lumps",
	"Errors" //WadContentX objects with error texts
};


/*!
* Entry for registering a type of wad (or pk3) content, with fields to
* hold some statistics, such as the number of lumps/files and the total
* number of bytes for this lump type. The exact usage of the fields
* depends on the content type.
*/
class WadContent
{
	public:
		WadContent(WadContentType t);
		virtual ~WadContent() {}

		/*! string with name and count. */
		virtual wxString getString();

		/*! Increment count, add lump size to bytes. */
		virtual void addLump(DirEntry* lump);

		/*!
		* Add contents and takes over ownership of the other WadContent
		* (it can be deleted by this call).
		*/
		virtual void mergeContent(WadContent* wc);

		WadContentType type; //!< Category
		int count; //!< Number of lumps/files of this type
		int bytes; //!< Total size of lumps/files of this type
		int markerStack; //!< Keeps track of marker lumps in wad
};

/*!
* Extended entry for wad (or archive) content, for where we need to store
* a DirEntry for each lump or multiple entries for the same lump type. It
* has a pointer to the next entry, to form a linked list, and can also
* store DirEntries. It is used for content which needs further processing,
* in which case the name must be the file name, and for other content
* which may need individual attention, such as errors.
*/
class WadContentX : public WadContent
{
	public:
		/*! Each object has its own name, and may have pointer to other object. */
		WadContentX(WadContentType t, wxString str, WadContentX* wcx=NULL);

		/*!
		* Deleting the WadContentX deletes all its DirEntry objects as well
		* as any WadContentX pointed to. To only delete this WadContentX,
		* make sure to keep the "next" object and then set the pointer to NULL.
		*/
		virtual ~WadContentX();

		virtual wxString getString();

		/*!
		* Adds DirEntry to vector, in addition to incrementing counts.
		* A copy of the DirEntry object is created, so that the original
		* object is not referenced (it can be deleted by the caller).
		*/
		virtual void addLump(DirEntry* lump);

		virtual void mergeContent(WadContent* wc);

		/*! Add WadContentX to end of linked list. */
		void addToEnd(WadContentX* wcx);

		/*!
		* Merge another sorted linked list into a sorted linked list, creating
		* one sorted linked list with all WadContentX nodes. They are sorted
		* on the name of the first DirEntry.
		*/
		void addSorted(WadContentX* wcx);

		/*! Checks if the object has a DirEntry with this name. */
		bool containsLump(string lumpName);

		wxString name; //!< Full path for file which needs further processing
		vector<DirEntry*>* lumps; //!< DirEntry for each lump
		WadContentX* next; //!< Pointer for linked list
};

/*!
* WadStats holds information about a Doom-engine resource file, and does
* the parsing of such a file to populate its data. Reading the file, it
* creates WadContent objects counting lumps of different types, with
* additional data for special lumps such as for maps. In addition it has
* a number of public fields for file stats. It can produce a textual
* report of its contents through the TextReport interface.
*
* The analysis will try to deduce which iwad and engine the file targets,
* setting values for the iwad and engine fields matching those used in the
* database data model. Note that this is only a best effort based on the
* limited lump analysis done here, and not conclusive.
*
* WadStats is used directly to process wad files, but also used as a base
* class for classes to process other resource files.
*/
class WadStats
{
	public:
		/*!
		* Constructed with file name (full path), setting filePathName
		* and fileName.
		*/
		WadStats(wxString file);

		virtual ~WadStats();

		/*!
		* Delete any resources owned by the WadStats, including
		* temporary files on disk.
		*/
		virtual void cleanup();

		/*!
		* Read and process the file, filling the WadStats with data.
		*/
		virtual void readFile(TaskProgress* progress, bool findMd5=true);

		/*!
		* If the resource file contains a MAPINFO lump, this is parsed
		* and the result returned. Doing this can also affect the
		* categorization of some other lumps, as the MAPINFO can name
		* music lumps.
		*/
		virtual MapinfoParser* getMapinfo(TaskProgress* progress);

		/*!
		* Outputs its contents through the TextReport interface.
		*/
		virtual void printReport(TextReport* reportView);

		/*!
		* Returns the first WadContentX in the linked list of
		* map entries found in the file. It has a list of
		* DirEntry objects for the map content lumps. Call
		* next on each WadContentX object to iterate.
		*/
		WadContentX* getMapContent();

		/*!
		* Returns WadContentX with lump entries found, for special
		* WadContentTypes which support it. Supported types are
		* WDEHACKED, WDECORATE, WMAPINFO and WUNKNOWN.
		*/
		WadContentX* getContent(WadContentType type);

		//Public data
		wxString filePathName; //!< Full path of file
		string fileName; //!< Just filename
		uint16_t year; //!< Year of file timestamp
		uint32_t fileSize; //!< Size of file, in bytes
		unsigned char md5Digest[16]; //!< MD5 checksum of file
		string wadType; //!< From wad header: IWAD/PWAD
		bool hexenMap; //!< Hexen map format (BEHAVIOR lump)
		bool hexenLumps; //!< Other Hexen lumps (can be Hexen or Zdoom)
		IwadType iwad; //!< Deduced iwad
		EngineType engine; //!< Deduced engine type
		int32_t numberOfLumps; //!< Lump count, from wad header
		WadContent* content[WEND]; //!< WadContent object for content types
		int priority; //!< Wad is given a priority to decide which is the main amongst several

	protected:
		/*! Generate md5Digest (checksum) of the file. */
		void makeMd5(wxFile& file);

		/*! Add a lump to the content array. */
		void addLump(DirEntry* de, WadContentType type);

		/*! Register wad error, adding to content[WERROR]. */
		void lumpError(wxString error, DirEntry* lump);

		/*! Process DirEntry of size 0. */
		void processMarkerLump(DirEntry* lump, const wxString& lname);

		/*! Process map marker lump. */
		void mapMarker(DirEntry* lump);

		/*!
		* Process DirEntry, categorizing it. Most entries are processed by
		* DirEntry alone, but for some entries we read the first few bytes for
		* additional info, from the provided stream. The stream must be seekable,
		* but need not be buffered, since we seek to the offset found in the
		* DirEntry and only read a few bytes (so a buffer will be wasted).
		*/
		void processLump(DirEntry* lump, wxInputStream* file);

		/*! Process DirEntry name, checking if lname matches special lump names. */
		bool processLumpName(DirEntry* lump, const wxString& lname);

		/*! Checks for more rare lump names, such as stuff from iwads of other games. */
		bool processLumpName2(DirEntry* lump, const wxString& lname);

		/*!
		* Check if lump is Doom-format image, adding it to stats of t.
		* The wxInputStream must be seekable.
		*/
		bool checkPicture(DirEntry* lump, wxInputStream* file, WadContentType t);

		/*! Check for patch and texture lists, from DirEntry and first 4 bytes. */
		bool checkTexLists(DirEntry* lump, const wxString& lname, char* bytes);

		/*! Check if lump is PNG image, from DirEntry and first 4 bytes. */
		bool checkPng(DirEntry* lump, char* bytes, WadContentType t);

		/*! Check if lump is JPG image, from DirEntry and first 4 bytes. */
		bool checkJpg(DirEntry* lump, char* bytes);

		/*! Check if lump is BMP image, from DirEntry and first 4 bytes. */
		bool checkBmp(DirEntry* lump, char* bytes);

		/*! Check if font lump FON1 or FON2, from DirEntry and first 4 bytes. */
		bool checkFon12(DirEntry* lump, char* bytes);

		/*! Check if lump is Doom-format music, from DirEntry and first 4 bytes. */
		bool checkMus(DirEntry* lump, char* bytes);

		/*! Check if lump is midi music, from DirEntry and first 4 bytes. */
		bool checkMidi(DirEntry* lump, char* bytes);

		/*! Check if lump is mp3 or Ogg Vorbis, from DirEntry and first 4 bytes. */
		bool checkMp3Ogg(DirEntry* lump, char* bytes);

		/*! Check if lump is Doom-format sound sample, from DirEntry and first 4 bytes. */
		bool checkSfx(DirEntry* lump, char* bytes);

		/*! Check if lump is Wav-format sound sample, from DirEntry and first 4 bytes. */
		bool checkWav(DirEntry* lump, char* bytes);

		/*! Check if lump is FLAC sound sample, from DirEntry and first 4 bytes. */
		bool checkFlac(DirEntry* lump, char* bytes);

		/*! Check if lump is PC speaker soundfx, from DirEntry and first 4 bytes. */
		bool checkPcs(DirEntry* lump, char* bytes);

		/*! Check if lump is compiled ACS script, from DirEntry and first 4 bytes. */
		bool checkAcs(DirEntry* lump, char* bytes);

		/*! Parse lump of given type for #includes, moving referenced lumps from WUNKNOWN to wct. */
		void findLumpIncludes(WadContentType wct, TaskProgress* progress);

		/*! Look for includes among unknown entries. Those found are returned as WadContentX. */
		virtual WadContentX* findUnknownLumps(vector<wxString>* inclNames, WadContentType wct, TaskProgress* progress);

		/*! Goes through map entries found, checking if their contents are valid. */
		void validateMaps();

		/*! Removes entries with size -1 from unknwon lumps. */
		void deleteUnknown();

		/*! Complete detection of IwadType and EngineType. */
		void checkIwadEngine();

		WadContentX* currentMap;
};

#endif // WADSTATS_H
