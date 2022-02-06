/*!
* \file MapStats.h
* \author Lars Thomas Boye 2018
*
* MapStats handles the analysis of maps - the level structures of
* Doom-engine games. A map is defined by its geometry, with constructs
* such as vertices, lines and sectors, and by the things/actors placed
* in the map. In addition there is a tree structure composed from the
* basic geometry - the nodes - used by the engine to efficiently render
* the map, and some other compiled structures. Everything is stored in
* a set of lumps in the wad file. We process each of the lumps to
* compute statistics.
*
* The "things" of the map are analyzed based on a set of ThingDef
* objects giving the properties of the thing types. Counts and ratios
* are aggregated in ThingStats objects. Nodes are processed by a
* NodeStats object, defined elsewhere.
*
* MapStats is the main class, running the processing and keeping the
* results.
*/

#ifndef MAPSTATS_H
#define MAPSTATS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "WadStats.h"
#include "NodeStats.h"
#include "DecorateParser.h"
#include <vector>
#include <map>
#include <list>
#include <wx/stream.h>


/*!
* Keeps track of gameplay statistics, computed from the entries
* of the THINGS list of the map. The properties of the things
* are defined by ThingDef objects - see ThingDef for documentation
* of the properties.
*/
class ThingStats
{
	public:
		ThingStats();

		/*!
		* Add the properties of a ThingDef to the stats. This is the
		* only method which modifies the state of the object.
		*/
		void addThing(ThingDef* thing);

		/*! health/monsterHP, or 0.0 if no monsters. */
		float getHealthRate();

		/*! ammo/monsterHP, or 0.0 if no monsters. */
		float getAmmoRate();

		/*! armor/monsterHP, or 0.0 if no monsters. */
		float getArmorRate();

		uint16_t monsters; //!< Number of enemies (category THING_MONSTER)
		uint32_t monsterHP; //!< Total hit-points of all things (except THING_FRIENDLY)
		bool spawners; //!< true if there are things which can spawn monsters
		uint32_t health; //!< Total health points from pickups
		uint32_t ammo; //!< Total ammo points from pickups
		uint32_t armor; //!< Total armor points from pickups
		vector<string> weapons; //!< Name of each weapon found
};

/*! Number of steps for tracking progress of processing one map. */
const int MAP_PROGRESS_STEPS = 100;

/*!
* MapStats processes the map lumps of a map, extracting information
* and computing statistics. It holds all the data it computes,
* available with get-methods and public fields. Most of it is
* simple values, but you can also get all map lines (to draw the
* map) and a full listing of all textures used, including number of
* occurrences of each. Gameplay statistics are computed for each of
* the three skill levels the map data distinguishes between.
* MapStats can produce a textual report of its contents through the
* TextReport interface.
*/
class MapStats
{
	public:
		/*!
		* Created with the map lump name (MAPx or ExMy) and engine
		* type. The WadProgress is updated with progress during
		* processing.
		*/
		MapStats(string name, EngineType eng, TaskProgress* tp);

		virtual ~MapStats();

		/*!
		* Reads the map lumps from a wad file, computing all statistics.
		* Input is the file, a list of the lumps to process, and a map
		* with ThingDef objects (keys are the IDs found in the THINGS
		* lump). thingDefs may be null, in which case all things in the
		* map are registered as THING_UNKNOWN and no gameplay statistics
		* are computed.
		*/
		virtual void readFile(wxInputStream* file, vector<DirEntry*>* lumps, map<int, ThingDef*>* thingDefs);

		/*!
		* Outputs its contents through the TextReport interface.
		*/
		void printReport(TextReport* reportView);

		/*!
		* Number of things of the given category. These are counts of
		* entries in the THINGS lump, without considering their skill
		* level or other flags.
		*/
		uint16_t getThingCount(ThingCat cat) {return thingCounts[cat];}

		/*! Total number of entries in THINGS lump (all categories). */
		uint16_t getTotalThings();

		/*! Total number of LINEDEFS. */
		uint32_t getLineCount() {return lines->size();}

		/*!
		* Gameplay statistics for a skill level. Gameplay statistics are
		* computed for each of the three skill levels the map data
		* distinguishes between. Valid level values are 0, 1 and 2.
		* Note that the first typically covers the two lowest settings
		* (skill 0 and 1), 1 is medium (HMP) and 2 is hard (UV+NM).
		*/
		ThingStats* getThingStats(int level) {return thingStats[level];}

		/*!
		* True if gameplay statistics are different for different skill
		* levels (skill flags are used).
		*/
		bool hasDifficultySetting();

		/*! Lowest x and y coordinate values in map (for bounding box). */
		Vertex getStartCorner() {return minCorner;}

		/*! Highest x and y coordinate values in map (for bounding box). */
		Vertex getEndCorner() {return maxCorner;}

		/*! All vertices in VERTEXES lump, including those added for nodes. */
		vector<Vertex>* getVertices() {return vertices;}

		/*! Number of vertices used for LINEDEFS. */
		uint32_t getVertexCount() {return lineVertices;}

		/*! All lines of map geometry (from LINEDEFS). */
		vector<MapLine>* getLines() {return lines;}

		/*! Total length of all lines of map geometry (LINEDEFS). */
		double totalLineLength() {return lineLength;}

		/*! Average length of the lines (LINEDEFS). */
		double avgLineLength() {return lineLength/lines->size();}

		/*! Number of sectors in map. */
		uint16_t getSectorCount() {return sectors;}

		/*! Number of secret sectors. */
		uint16_t getSecretCount() {return secrets;}

		/*! Total square area of map sectors (in Doom map units). */
		double getMapArea() {return area;}

		/*!
		* Average light level of sectors. Light level range is 0-255.
		* Note that this is not weighted by sector size, so it may not
		* be an accurate representation of the map light level.
		*/
		double avgLightLevel() {return lightSum/double(sectors);}

		/*! Number of different wall textures used. */
		int getTextureCount() {return textures->size();}

		/*! Alphabetical listing of all textures used, with number of uses for each. */
		map<string, int>* getTexturesAlpha() {return textures;}

		/*!
		* List of all textures used, with number of uses for each.
		* Each entry is a pair with texture name and number of uses
		* (number of SIDEDEFS entries). This list is sorted on number
		* of uses. A new list object is created on the heap, and the
		* caller is responsible for deleting this.
		*/
		list< pair<string,int> >* getTexturesCount();

		/*!
		* true if the map has nodes and they are in a GL format.
		*/
		bool hasGLNodes();

		//Public data
		string mapName; //!< Map lump name, set by constructor
		EngineType engine; //!< Engine type, set by constructor
		bool multiOnlyThings; //!< Map has multiplayer(dm+coop)-only things
		bool dmOnlyThings; //!< Map has things exclusive to deathmatch mode
		bool coopOnlyThings; //!< Map has things exclusive to coop mode

	protected:
		DirEntry* findLump(const string& name, vector<DirEntry*>* lumps);
		virtual void processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs);
		virtual void processVertexes(wxInputStream* file, int32_t lsize);
		virtual void processLinedefs(wxInputStream* file, int32_t lsize);
		virtual void processSidedefs(wxInputStream* file, int32_t lsize);
		virtual void processSectors(wxInputStream* file, int32_t lsize);
		void processReject(wxInputStream* file, int32_t lsize);
		void processBlockmap(wxInputStream* file, int32_t lsize);

		/*!
		* Factory method for creating the appropriate NodeStats object,
		* matching the format of the node lump.
		*/
		void findNodeType(wxInputStream* file, int32_t lsize);

		/*!
		* If the map has a GL_VERT lump, this method is called to
		* create the appropriate NodeStats object.
		*/
		void findGLNodeType(wxInputStream* file, int32_t lsize);

		//private data:
		TaskProgress* progress;
		uint16_t thingCounts[THING_END];
		ThingStats* thingStats[3]; //For each difficulty
		vector<Vertex>* vertices; //All vertices, including for segs
		uint32_t lineVertices; //Vertices used by linedefs
		Vertex minCorner; //Lowest x and y
		Vertex maxCorner; //Highest x and y
		vector<MapLine>* lines; //From linedefs
		double lineLength; //Total length
		uint16_t sectors; //Number of sectors
		uint16_t secrets; //Number of secret sectors
		double lightSum;
		double area;
		map<string, int>* textures;
		NodeStats* nodeStats;
		bool reject; //true if present and has non-null entries
		bool blockmap; //true if present
};

#endif // MAPSTATS_H
