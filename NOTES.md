# Doom Map Database
Source code notes

## Build notes
Currently using wxWidgets version 3.1.0.  
So far only for Windows. The code is mostly platform-independent, but there will be some details which needs fixing to build for other platforms supported by wxWidgets.  
The source includes project setup for Code::Blocks IDE: http://www.codeblocks.org/  
Set up to compile with GNU GCC.  

## Source file guide
Here is an overview of all the source code (most points are .h and .cpp file), organised in some main parts:

### Main
* resource.rc: Defines application icon.
* GuiMain: Top-level class, starts execution (creates GuiFrame).
* GuiBase: Basic definitions for Gui part, and menus and toolbars.
* GuiFrame: Main application window, connecting the main GUI elements with the data objects.
* GuiSettings: Settings, with defaults and persistance, and dialog for changing them.
* GuiMinorDialogs: "About" dialog.
* TextReport: Interface for text output.
* LtbUtils: Some general utility methods.

### Database
* DataModel: Representation of wads, maps and associated objects for the database.
* DataFilter: To select subsets of wad and map entries.
* DataManager: Manages all data objects and their file persistence.
* MapStatistics: Represents and computes statistics for a set of maps.
* WadStatistics: Statistics class for a set of wads.
* StatisticSet: Tables with MapStatistics or WadStatistics objects based on different categories.

### File analysis
* ThingDef: Defines map things, for map analysis.
* TextLumpParser: Base class for text lump parsers.
* IncludeParser: Find file references from include statements.
* DecorateParser: Create ThingDefs from ZDoom decorate.
* md5: Hash algorithm.
* MapStats: Analysis of map (Doom format lumps).
* HexenMapStats: MapStats subclass for Hexen format lumps.
* UdmfMapStats: MapStats subclass for UDMF format lumps, parsing the textual map lump.
* NodeStats: Analysis of map nodes, calculating area.
* DehackedParser: Analysis of deh file, for map names and ThingDef modification.
* MapinfoParser: Analysis of MAPINFO lump, for map names and music.
* WadArchive: Getting files from zip.
* WadStats: Analysis of wad as resource file, processing lumps.
* Pk3Stats: Analysis of zip archive as resource file, processing files.
* WadReader: Overall coordinator, getting DB entries from files.
* TaskProgress: Keeps track of state during analysis.
* GuiThingDef: List of ThingsDefs, can edit.
* GuiWadReport: Dialog for WadReader and WadStats.
* GuiMapReport: Dialog for MapStats (report).
* GuiAspectDialog: Dialog to specify which aspects to update.
* GuiProgress: Dialog showing status/progress.

### Database GUI
* GuiViewSelect: Control for selecting the current entry list (filter or list, wads or maps).
* GuiEntryList: Shows wad or map list in table view, with sortable columns.
* GuiPersonDialogs: Dialogs dealing with AuthorEntries (people and groups).
* GuiTagDialogs: Tag list dialog, and tag edit controls.
* GuiMapSelect: Dialog listing wads and maps, to select map.
* GuiImagePanel: GUI component for displaying an image from file.
* GuiWadManager: Creates and manages UI for wads.
* GuiMapManager: Creates and manages UI for maps.
* GuiMapCommon: Dialog to set common properties for all maps in a wad.
* GuiWadPanel: Panel displaying select fields for a WadEntry.
* GuiMapPanel: Panel displaying select fields for a MapEntry.
* GuiWadEntry: Window for displaying or editing all fields of wad entries.
* GuiMapEntry: Window for displaying or editing all fields of map entries.
* GuiComboEntry: Window for displaying or editing a wad entry and its map entries.
* GuiStatistics: Dialogs showing StatisticSet and Map/WadStatistics.

## Database files & folders
```
- FILE_AUTHORDB("people.dmdb"): Each AuthorEntry
	type (1B), dbid (4B)
	For each name/alias: length (2B) + UTF8 chars
	If group: dbid of each member

- FILE_AUTHORTEXT("ptext.dmdb"): Description of each person
	uint32_t: dir offset
	Text entry for each author:
		uint16_t length
		length UTF8 chars, in blocks of 100 bytes (last block padded with 0s)
	Dir - entry for each author:
		uint32_t dbid
		uint32_t textOffset - ref. text entry

- FILE_TAGS("tags.dmdb"): List of style tags
	ubyte: tagLength (chars)
	For each tag: tagLength chars (padded with 0s)
	NOTE: tag id is just index in list

- FILE_WADDB("wads.dmdb"): Each WadEntry (public part)
	ubyte: fileVersion
	Each entry: 37 bytes of fixed fields + 3 short c strings (avg. 67 bytes)
		Includes 1 byte for number of maps (but wad-map link is stored with maps)

- FILE_WADOWN("mywads.dmdb"): Private part of each WadEntry
	ubyte: fileVersion
	Each entry: 6 bytes of fixed fields

- FILE_MAPDB("maps.dmdb"): Each MapEntry (public part)
	ubyte: fileVersion
	Each entry: 54 bytes of fixed fields + 6 short c strings (avg. 100 bytes)
		Includes dbid (4B) of wad

- FILE_MAPOWN("mymaps.dmdb"): Private part of each MapEntry
	ubyte: fileVersion
	Each entry: 8 bytes of fixed fields

- FILE_VIEWS("lists.dmdb"): Wad and map lists
	ubyte: fileVersion
	For each list:
		ubyte type
		uint16_t name length
		length UTF8 chars (list name)
		ubyte sortField
		Each entry in list: uint32_t dbid
		uint32_t 0 to end the list

- Folder "\text": Textual descriptions for wads/maps
	Text file <wad_md5>.txt for each wad
	Sections tagged with $<section>: Resources, PortNotes, Links, Description, <Each map>
	Each section is UTF8 text

- Folder "\mapimg": Map drawings
	Png file <wad_md5>_<map>.png for each map

- Folder "\sshot": Map screenshots
	Image file <wad_md5>_<map>.<ext> for each map
```

## WadReader
My overview of the wad processing.
```
- initReader(filePath)
	WadArchive->readArchiveFiles()
- processWads()
	For main file, or each wad/pk3 in zip:
		WadStats->readFile()
			Categorize all lumps (unpack special ones from pk3)
			Process decorate/mapinfo/zscript, to find #include files
			Process sub-wads in pk3
		WadStats->getMapinfo()
			MapinfoParser->parseFile(filename,offs,size)
	Set iwad and engine

- getIwad(), setIwad(IwadType): Change guessed type?
- getEngine(), setEngine(EngineType): Change guessed type?

- findThingDefs()
	Set base thingDefs based on iwad/engine
	DecorateParser->parseFile(filename,offs,size) each decorate lump/file
		processLine() for each line, finding actors
	DecorateParser->extractThingDefs()
	DehackedParser->parseFile(filename,offs,size)

? addWad(file, pk3?)
	WadStats->readFile()
	WadStats->getMapinfo()
	DecorateParser->parseFile(filename,offs,size) each decorate lump/file
	DehackedParser->parseFile(filename,offs,size)
? addDeh(file)
	DehackedParser->parseFile(filename)
? removeWad(index)
	Delete one WadStats

- getMainDigest(): Check for match in DB
- getMainFileName(): Check for match in DB

- setAspects(aspects): Which features to include in DB entries
- createEntries(imgFolder, mapTempl)
	Calls updateEntries with new WadEntry, apply mapTempl
- updateEntries(wadEntry, imgFolder)
	aspects->wadFile: WadEntry fields from wadStats[]
	aspects->wadFlags: WadEntry flags from wadStats[]
	If aspects->mapAspects(), for each map:
		Create MapStats of right type (basic/hexen/udmf)
		MapStats->readFile(file, lumps, thingMap);
			Process Vertexes, Things, Linedefs, Sidedefs, Sectors, Reject, Blockmap
			Find node type, create NodeStats
			NodeStats->readFile(file, lumps)
			NodeStats->computeArea(minXY, maxXY)
		Create MapEntry, for various aspects: set MapEntry fields from MapStats
		aspects->mapImages: Draw map, save to file

- clearState()
```
