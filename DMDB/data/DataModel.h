/*!
* \file DataModel.h
* \author Lars Thomas Boye 2007
*
* Defines the data model for what is stored in the Doom Map Database.
* The central objects are WadEntry and MapEntry, where a WadEntry has
* a list of MapEntries. Then there are AuthorEntry for representing a
* person or group having made maps, and TagEntry representing a style
* tag which can be applied to MapEntries. These core objects are small,
* having only numbers, flags and a few short strings, to allow the
* entire core database to be held in memory. Each WadEntry object and
* each MapEntry object will use about 100 bytes each. The model also
* allows for arbitrary text about wads and maps, such as to give a
* description and review. This is managed separately, with the WadText
* struct to hold text about a wad (and its maps), so that it can be
* loaded into memory only when needed. Text descriptions for authors
* are stored together in one file, with each AuthorEntry object
* keeping an offset to this text file. Other media can also be
* associated with the core objects, such as screenshots for maps, but
* these do not need any representation here.
*
* In addition to the data types for the central model, there are
* accompanying enums and constants, including string labels for the
* wad and map fields as well as for value sets.
*/

#ifndef DATAMODEL_H //Avoid problems with multiple includes
#define DATAMODEL_H

//The std namespace is put in the global scope to freely use the standard library
using namespace std;

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <map>
#include "md5.h"


/*!
* Defines fields for wad and map entries. All fields which can be
* exposed to the user for both wads and maps are listed together
* here, for use when defining user interfaces. Corresponding
* members are found in WadEntry for the WAD_* fields and in
* MapEntry for the MAP_* fields.
*/
enum WadMapFields {
	WAD_DBID,
	WAD_FILENAME,
	WAD_FILESIZE,
	WAD_MD5DIGEST,
	WAD_EXTRAFILES,
	WAD_IDGAMES,
	WAD_TITLE,
	WAD_YEAR,
	WAD_IWAD,
	WAD_ENGINE,
	WAD_PLAYSTYLE,
	WAD_MAPS,
	WAD_WF_IWAD,
	WAD_WF_SPRITES,
	WAD_WF_TEX,
	WAD_WF_GFX,
	WAD_WF_COLOR,
	WAD_WF_SOUND,
	WAD_WF_MUSIC,
	WAD_WF_DEHBEX,
	WAD_WF_THINGS,
	WAD_WF_SCRIPT,
	WAD_WF_GLNODES,
	WAD_RATING,
	WAD_OWNRATING,
	WAD_PLAYTIME,
	WAD_OF_HAVEFILE,
	WAD_END_MAP_START, //wad.getName
	MAP_DBID,
	MAP_NAME,
	MAP_TITLE,
	MAP_BASEDON,
	MAP_AUTHOR1,
	MAP_AUTHOR2,
	MAP_SINGLE,
	MAP_COOP,
	MAP_DM,
	MAP_MODE,
	MAP_LINEDEFS,
	MAP_SECTORS,
	MAP_THINGS,
	MAP_SECRETS,
	MAP_ENEMIES,
	MAP_TOTALHP,
	MAP_AMMORAT,
	MAP_HEALTHRAT,
	MAP_ARMORRAT,
	MAP_AREA,
	MAP_LINEDEF_DENS,
	MAP_ENEMY_DENS,
	MAP_HP_DENS,
	MAP_RATING,
	MAP_MF_SPAWN,
	MAP_MF_MORESPAWN,
	MAP_MF_DIFFSET,
	MAP_MF_VOODOO,
	MAP_MF_UNKNOWN,
	MAP_MF_SAMEAS,
	MAP_MF_NOTLEVEL,
	MAP_OWNRATING,
	MAP_PLAYED,
	MAP_DIFFICULTY,
	MAP_PLAYTIME,
	MAP_END
};

/*!
* Defines string labels for the wad and map fields, for user interfaces.
* Use WadMapFields enum for index in this array.
*/
const wxString wadMapLabels[] = {
	"DBID",
	"Main file",
	"File size (bytes)",
	"MD5 hash",
	"Additional files",
	"idgames ID",
	"Wad title",
	"Year released",
	"Iwad",
	"Engine",
	"Play style",
	"Maps",
	"Is iwad",
	"Sprites",
	"Textures",
	"Other graphics",
	"Palette",
	"Sounds",
	"Music",
	"Dehacked/BEX",
	"Monsters/items",
	"Scripts",
	"GL nodes",
	"Ext.Rating",
	"Rating",
	"Total playtime",
	"File present",
	"In wad", //wad.getName
	"DBID",
	"Map",
	"Map title",
	"Based on",
	"Author 1",
	"Author 2",
	"Single player",
	"Cooperative",
	"Deathmatch",
	"Other mode",
	"Linedefs",
	"Sectors",
	"Things",
	"Secrets",
	"Enemy count",
	"Total hit points",
	"Ammo ratio",
	"Health ratio",
	"Armor ratio",
	"Area (MU)",
	"Linedef/Area",
	"Enemy/Area",
	"HitPoint/Area",
	"Ext.Rating",
	"Enemy spawning",
	"Excessive spawning",
	"Difficulty settings",
	"Voodoo doll",
	"Unknown things",
	"Same as",
	"Non-level",
	"Rating",
	"Played/won",
	"Difficulty",
	"Play time"
};

/*!
* Defines the known iwads - the games a wad can target.
* IMPORTANT: As this is persisted as an integer in the database,
* we can only add to the end of the list (before IWAD_END), not
* insert new entries before an entry already in use.
*/
enum IwadType {
	IWAD_NONE,
	IWAD_CUSTOM,
	IWAD_DOOM,
	IWAD_DOOM2,
	IWAD_TNT,
	IWAD_PLUT,
	IWAD_HERETIC,
	IWAD_HEXEN,
	IWAD_STRIFE,
	IWAD_CHEX,
	IWAD_CHEX3,
	IWAD_HACX,
	IWAD_HACX2,
	IWAD_ACTION2,
	IWAD_SQUARE,
	IWAD_HARMONY,
	IWAD_RISEWOOL,
	IWAD_DOOM64,
	IWAD_END
};

//! Names of the iwads defined by the IwadType enum
const wxString iwadNames[] = {
	" ", //None
	"Custom",
	"Doom (Ult)",
	"Doom 2",
	"TNT",
	"Plutonia",
	"Heretic",
	"Hexen",
	"Strife",
	"Chex Quest",
	"Chex Quest 3",
	"Hacx",
	"Hacx 2",
	"Action Doom 2",
	"Adventures of Square",
	"Harmony",
	"Rise Of The Wool Ball",
	"Doom 64"
};

/*!
* Defines the types of engine/port we distinguish between.
* IMPORTANT: As this is persisted as an integer in the database,
* we can only add to the end of the list, not insert new entries
* before an entry already in use.
*/
enum EngineType {
	DENG_NONE,
	DENG_CUSTOM,
	DENG_ORIGINAL,
	DENG_LIMITREM,
	DENG_BOOM,
	DENG_LEGACY,
	DENG_ZDOOM,
	DENG_GZDOOM,
	DENG_SKULLTAG,
	DENG_PLAYSTATION,
	DENG_DOOM64,
	//ETERNITY
	//DOOMSDAY
	//RISEN3D
	//EDGE
	//3DGE
	DENG_END
};

//! Names of the engines/ports defined by the EngineType enum
const wxString engineNames[] = {
	" ", //None
	"Custom",
	"Original",
	"Limit-removing",
	"Boom",
	"Legacy",
	"ZDoom",
	"GZDoom",
	"Skulltag",
	"Playstation",
	"Doom 64"
};

/*!
* WadEntry has a playstyle field, which is represented with
* an integer number. The meaning of possible values are given
* by these labels.
*/
const wxString playStyles[] = {
	"2.5D", //The original pseudo-3D
	"3D", //True 3D gameplay - jumping, mouselook
	"Special" //Other types, such as sidescroller
};

//! Labels for rating scale 0-10
const wxString rating10[] = {
	"0 - Not working",
	"1 - Barely playable",
	"2 - Very poor",
	"3 - Poor",
	"4 - Weak",
	"5 - Mediocre",
	"6 - Decent",
	"7 - Good",
	"8 - Great",
	"9 - Excellent",
	"10 - Perfect",
	"No rating"
};

//! Level of support for single-player, coop and deathmatch
const wxString gameModeSupport[] = {
	"No",
	"Starts only",
	"Should work",
	"Made for",
	" " //Unspecified
};

//! Names of possible other game modes
const wxString otherGameModes[] = {
	" ", //None
	"CTF",
	"Invasion"
};

//! Difficulty ratings
const wxString difficultyRatings[] = {
	" ", //None
	"Easy",
	"Medium",
	"Hard",
	"Ultra-Hard"
};

//! Value labels for MapEntry "Played" field
const wxString playedLabels[] = {
	"No",
	"Played",
	"Won skill 1",
	"Won skill 2",
	"Won skill 3",
	" " //Unspecified
};

const int MAXTAGS = 6; //!< Number of style keywords in a MapEntry
const int AUTHORGROUP = 12; //!< Maximum number of authors in group

//! Core part of an entry (used to specify which part is modified)
const int ENTRY_CORE = 1;

//! Text part of an entry (used to specify which part is modified)
const int ENTRY_TEXT = 2;

//! Image part of an entry (used to specify which part is modified)
const int ENTRY_PIC = 4;

/*!
* Scaling for square area values in MapEntry. The computed square unit
* area of a map is divided by this to give the value stored in MapEntry.
* The current factor means a value of 1 corresponds to an area of 1024
* by 1024 map units - what we can call a square mega-unit.
*/
const double AREA_FACTOR = 1024.0*1024.0;

/*!
* WadFlags is used to store a number of bits in a single word in wad
* entries, defining the value/position of each flag. Most of them are
* used to specify forms of additional content in the wad.
*/
enum WadFlags {
	WF_IWAD = 1, //!< Wad header is IWAD
	WF_SPRITES = 2, //!< Custom sprites
	WF_TEX = 4, //!< Custom textures (patches/flats)
	WF_GFX = 8, //!< Custom additional graphics
	WF_COLOR = 16, //!< Custom palette
	WF_SOUND = 32, //!< Custom sounds
	WF_MUSIC = 64, //!< Custom music
	WF_DEHBEX = 128, //!< DEH or BEX used (modified thing behavior)
	WF_THINGS = 256, //!< New/modified monsters/weapons/pickups
	WF_SCRIPT = 512, //!< Scripts, like ACS
	WF_GLNODES = 1024 //!< GL nodes included
};

/*!
* MapFlags is used to store a number of bits in a single word in map
* entries, defining the value/position of each flag.
*/
enum MapFlags {
	MF_SPAWN = 1, //!< Monster spawning (map will have more than starting amount)
	MF_MORESPAWN = 2, //!< Much monster spawning
	MF_DIFFSET = 4, //!< Difficulty settings are used
	MF_VOODOO = 8, //!< Has voodoo doll (additional player 1 thing)
	MF_UNKNOWN = 16, //!< Has unknown items
	MF_SAMEAS = 32, //!< If this is the same as a related map entry
	MF_NOTLEVEL = 64 //!< If not a playable level (such as placeholder or cutscene map)
};

/*!
* Flags for personal fields of the data objects. This has flags for
* specifying unsaved changes in memory.
*/
enum OwnFlags {
	OF_MAINNEW = 1, //!< New entry (not yet persisted in core DB)
	OF_MAINMOD = 2, //!< Unsaved changes in core DB
	OF_OWNNEW = 4, //!< New personal entry
	OF_OWNMOD = 8, //!< Unsaved changes in personal entry
	OF_HAVEFILE = 16, //!< File found in local file system (wad)
	OFLG_DELETE = 32 //!< Entry is marked for deletion
};

/*!
* Represents a person or other entity who is the maker of maps.
* This core database object is mainly to name the maker, which
* can include both real name (split into first and last parts)
* as well as up to two aliases. At least one form of name has
* to be provided. Textual descriptions for the AuthorEntries can
* be stored in a file, with the AuthorEntry object having the
* file offset for its entry in the file.
*
* Identity: There is no explicit uniqueness other than the
* database ID, but the application should check for exact
* duplicates of namef+namel or aliases, and at least warn against
* this.
*
* Constraints: At least one of the name fields (namef, namel,
* alias1) has to be a non-empty string. alias2 should only be
* non-empty if alias1 is non-empty.
*/
class AuthorEntry
{
	public:
	unsigned char type; //!< 0=Author, Otherwise number of entries in group
	uint32_t dbid; //!< Wad Database ID (set by counter, starts at 1)
	wxString namef; //!< Author first name, "" if unknown
	wxString namel; //!< Author last name, "" if unknown
	wxString alias1; //!< Author nickname, "" if unknown
	wxString alias2; //!< Can have second nickname, or ""
	unsigned char modified; //!< OwnFlags MAINNEW, MAINMOD, DELETE

	uint32_t textOffset; //!< File offset in text file (not persisted in author DB)

	AuthorEntry(uint32_t id); //!< Constructor
	virtual ~AuthorEntry() {}

	//! Returns namef and namel put together, or "" if both are empty
	wxString getName();

	/*!
	* Searches for the string sub in the names and aliases of the AuthorEntry,
	* returning true if it is found. The search is done with all-lower-case
	* versions of the AuthorEntry strings, so sub should also be lower-case only,
	* for a case-insenitive search. Note that that the complete sub must be
	* found in one of the AuthorEntry strings, so to search for "john romero"
	* you need to split it in two words and check both.
	*/
	bool findLowerCase(const wxString& sub);
};

/*!
* Represents a set of AuthorEntries. It is an AuthorEntry with type
* larger than 0. It should typically only use alias1 as the name of
* the group. What it adds to AuthorEntry is an array of AuthorEntries.
* It is used when a map has more than two authors, or when we only
* know that a team made a wad, not who made each individual map.
*/
class AuthorGroupEntry : public AuthorEntry
{
	public:
	uint32_t entryIds[AUTHORGROUP]; //!< Only used during loading
	AuthorEntry* pointers[AUTHORGROUP]; //!< Persisted as uint32_t dbids

	AuthorGroupEntry(uint32_t id);
	virtual ~AuthorGroupEntry() {}

	/*!
	* Adds an AuthorEntry to the group. It checks if the same entry
	* is already in the array or the array is full, returning false
	* instead of adding it in these cases.
	*/
	bool addEntry(AuthorEntry* entry);

	/*!
	* Looks for the given AuthorEntry in the group. Returns its index
	* in the array if found, otherwise -1.
	*/
	int findEntry(AuthorEntry* entry);

	/*!
	* Remove an AuthorEntry from the group, based on its index in
	* the array.
	*/
	void removeEntry(int index);
};

struct MapEntry; //Defined below

/*!
* Represents a file with maps and other resources for a Doom-engine
* game, possibly with one or several additional files with additional
* resources. One specific file is considered the primary file, with
* name, size and hash stored. Only the names of additional files are
* stored. The file could be in wad format or in other formats
* supported by some ports, such as pk3. The entry itself stores
* properties we consider common to the wad as a whole, including iwad,
* engine and the different types of content included. Then it has a
* list of MapEntry objects, with one entry for each map. As a core
* database object, no long strings are stored here. Descriptive text
* is stored separately, and represented with the WadText struct when
* in memory.
*
* Identity: The md5Digest of the primary file is considered a unique
* ID for the entry, connecting it to this file and also to additional
* content associated with the entry. Files with associated content such
* as text and screenshots will use the md5Digest (as hexadecimal string)
* in their file name. The md5Digest can be used to identify the same
* wad in another database instance. The digest will of course be
* different if the entry is based on a slightly different version of
* the file. Changing the digest is permitted, to reflect an update in
* the wad (all associated content must be updated). The title is used
* to name the entry if it is defined, otherwise the fileName is used.
*
* Constraints: dbid is 0 on a new entry before it is given an ID for
* persistence, thereafter always larger than 0. fileName and md5Digest
* must be specified, and md5Digest must be unique.
*
* ~60-100 bytes
*/
struct WadEntry
{
	uint32_t dbid; //!< Wad Database ID (set by counter)
	string fileName; //!< Name of main file, ex: "mm.wad"
	uint32_t fileSize; //!< Bytes
	unsigned char md5Digest[16]; //!< MD5 of main file - 16 bytes. Must be unique
	string extraFiles; //!< List of additional files to load, with ';' separator
	uint32_t idGames; //!< Wad id number in idGames database, for HTTP API

	string title; //!< Full title
	uint16_t year; //!< Year of (original) release, 0 for none
	unsigned char iwad; //!< What game/iwad the map is for (index in iwad list)
	unsigned char engine; //!< What engine/port is needed (index in engine list)
	unsigned char playStyle; //!< 0=2.5D, 1=3D, 2=special

	vector<MapEntry*> mapPointers; //!< Pointers to MapEntry-objects (only in memory)
	unsigned char numberOfMaps; //!< Number of entries in mapPointers
	uint16_t flags; //!< Defined by WadFlags enum (mainly resource types)
	unsigned char rating; //!< 0-100, an external rating (average?)

	// Personal fields, persisted in separate file
	unsigned char ownRating; //!< 0-100, 255=No rating
	unsigned char ownFlags; //!< Defined by OwnFlags

	/*! Constructor. id=0 if not yet stored. Specify size of mapPointers. */
	WadEntry(uint32_t id, int mapSize);

	/*! title if defined, otherwise fileName. */
	string getName() const;

	/*! Add MapEntry, up to size specified by constructor. */
	void addMap(MapEntry* mapEntry);

	/*! Get MapEntry based on map name. */
	MapEntry* getMap(string& mapName);

	/*! Remove MapEntry at the given index in mapPointers, optionally also deleting the object. */
	void removeMap(int index, bool del);

	/*! Index of map in mapPointers list, or -1. */
	int getMapIndex(MapEntry* mapEntry);

	/*! true if md5ptr is same as this wad's md5Digest. */
	bool matchMd5(unsigned char* md5ptr);

	/*! md5Digest as hexadecimal string. */
	string getMd5String() const;

	/*! true if rating field has valid value. */
	bool isRated() const {return rating<=100;}

	/*! true if ownRating field has valid value. */
	bool isOwnRated() const {return ownRating<=100;}

	/*! Returns sum of playTime in each map of the wad. */
	uint16_t getTotalPlaytime();

	/*!
	* Put common values for all maps in the wad into a template MapEntry.
	* Fields considered are author1/2, the modes (single, coop, dm and other),
	* tags, rating, ownRating, played, difficulty and playTime. Only where
	* all maps in the wad has the same value will this value be copied to
	* mapTempl, otherwise a null/undefined value will be set to indicate there
	* is no common value.
	*/
	void getCommonFields(MapEntry* mapTempl);

	/*!
	* Set values from a template MapEntry into every MapEntry of the wad.
	* This operates on the same map fields as getCommonFields, and sets the
	* values which are defined in mapTempl into every map of the wad. For
	* fields where mapTempl has a null/undefined value, the maps of the wad
	* keep their old values. For the sp/coop/dm modes, maps with value 0
	* ("No") keep this value (since this means there are no such starts in
	* the map), while if mapTempl has value 0 all non-zero maps are set to
	* 1 ("Starts only") since this is the minimal value for maps which has
	* starts for the mode. If setModFlags is true, the ownFlags of each map
	* is set to indicate modification.
	*/
	void setCommonFields(MapEntry* mapTempl, bool setModFlags=false);
};

/*!
* Textual descriptions accompanying a WadEntry. This is a set of
* unicode strings, stored on specific keys. We support one string per
* map, to describe/review that map, with the key being the map name
* (name field of MapEntry). Then there are various strings for the
* wad as a whole, with keys defined by the WADTEXT_* constants. The
* WadText is associated with a WadEntry through the MD5 hash.
*/
struct WadText
{
	string wadMd5; //!< Uniquely identifies the wad
	map<string, wxString>* entries; //!< One entry for each map, if multi-map wad
	bool changed; //!< Dirty flag

	WadText(string md5Digest);
	~WadText();
	void setText(string entry, wxString content);
	wxString getText(string entry);
	bool removeText(string entry); //!< removes and returns true if entry is found
	bool hasText(); //!< true if there are any entries
};

//! Wad textfile entry, for which resources the wad contains
const string WADTEXT_RES = "Resources";

//! Wad textfile entry, for notes on port compatibility and issues
const string WADTEXT_PORT = "PortNotes";

//! Wad textfile entry, for web links
const string WADTEXT_LINKS = "Links";

//! Wad textfile entry, for description/review
const string WADTEXT_DESC = "Description";

/*!
* Represents a map style tag, which is used to say something about
* the theme, texture style or gameplay style of a map. Tags are
* their own entities in the data model, but only have a string and
* a database ID. Maps refer to tags with the ID. The string must
* be unique.
*/
struct TagEntry
{
	uint16_t dbid; //!< Index in persisted list
	string tag; //!< Tag itself

	TagEntry(uint16_t id, string t);
};

/*!
* Represents one map within a wad, containing all the map-specific
* information. It has a pointer to the WadEntry it belongs to, and
* up to two AuthorEntries (which can be groups). TagEntries are
* referenced with their dbids. A few fields are for subjective
* information meant to be specified separately by each person, in
* his own instance of the database, while the rest are objective and
* meant to be shared.
*
* Identity: Firstly, a MapEntry is identified by the WadEntry it
* belongs to (in turn identified by the hash of the primary file).
* Each map is then identified by its name field, which is the wad
* lump name, such as "E1M1" or "MAP01". This must be unique within
* the wad. Note that we allow a WadEntry to represent multiple files,
* so it is possible to have multiple MAP01 associated with one
* WadEntry. The duplicate map names must then be given a letter
* postfix, such as "MAP01a", to make them unique within the WadEntry.
* Database resources associated to a specific map, such as a
* screenshot, use the combination of wad hash and map name for
* identification. For the user, the MapEntry can be identified by a
* title, but this is optional. If not specified, getTitle() instead
* returns wad name (which is wad title if specified, otherwise
* fileName) plus the map name. For a single-map wad, the wad title
* can also be used as map title.
*
* Constraints: dbid is 0 on a new entry before it is given an ID
* for persistence, thereafter always larger than 0. wadPointer can't
* be NULL - the map must belong to a wad. name must be specified,
* and must be unique within the wad.
*
* ~120 bytes
*/
struct MapEntry
{
	//Identity:
	uint32_t dbid; //!< Map Database ID (set by counter, starts at 1)
	WadEntry* wadPointer; //!< Pointer to WadEntry the map belongs to (wad dbid persisted)
	string name; //!< Wad lump name, like E1M1 or MAP01, can't be empty
	string title; //!< Human-readable map name
		//Use wadPointer->name if empty?
	uint32_t basedOn; //!< Database ID of a related map - base/inspiration

	AuthorEntry* author1; //!< Pointer to map designer (author dbid persisted)
	AuthorEntry* author2; //!< Additional map designer (author dbid persisted)
	//Can have AuthorGroupEntry with many authors

	//Game mode:
	unsigned char singlePlayer; //!< No / starts only / should work / made for
	unsigned char cooperative; //!< No / starts only / should work / made for
	unsigned char deathmatch; //!< No / starts only / should work / made for
	unsigned char otherMode; //!< One other mode (index in list)

	//Statistics:
	uint32_t linedefs; //!< Number of linedefs in map geometry
	uint16_t sectors; //!< Number of sectors in map geometry
	uint16_t things; //!< Number of things in map
	uint16_t secrets; //!< Number of secret sectors
	uint16_t enemies;  //!< Number of monsters present (UV)
	uint32_t totalHP; //!< Total health of all monsters (UV), with 1 bullet damage = 1
	float healthRatio; //!< Total health of pickups (UV)
	float armorRatio; //!< Total armor of pickups (UV)
	float ammoRatio; //!< Total damage of ammo pickups (UV), with 1 bullet damage = 1
	float area; //!< Total square area of all sectors

	//Descriptive:
	uint16_t tags[MAXTAGS]; //!< Style keywords, from list
	unsigned char rating; //!< 0-100, an external rating (average?)

	//! The bits of this byte store Boolean flags. The flags are defined by
	//! the enum MapFlags.
	unsigned char flags;

	//Own:
	unsigned char ownRating; //!< 0-100, 255=No rating
	unsigned char ownFlags; //!< Defined by OwnFlags
	unsigned char played; //!< Played this map: no, yes, won1, won2, won3
	unsigned char difficulty; //!< Difficulty rating (index in list)
	unsigned char playTime; //!< Minutes

	/*! Constructor - id=0 if not yet stored. */
	MapEntry(uint32_t id);

	/*! title if defined, otherwise wad name + map name. */
	string getTitle() const;

	/*! Create unique file name, from wad hash and map name. */
	wxString fileName();

	/*! true if rating field has valid value. */
	bool isRated() {return rating<=100;}

	/*! true if ownRating field has valid value. */
	bool isOwnRated() {return ownRating<=100;}

	/*! Linedefs per area (0.0 if area not specified). */
	float linedefDensity() {return (area==0.0)? 0.0: linedefs/area;}

	/*! Enemies per area (0.0 if area not specified). */
	float enemyDensity() {return (area==0.0)? 0.0: enemies/area;}

	/*! Enemy hit-points per area (0.0 if area not specified). */
	float hitpointDensity() {return (area==0.0)? 0.0: totalHP/area;}
};

#endif
