/*!
* \file MapStatistics.h
* \author Lars Thomas Boye 2018
*
* A DBStatistics object aggregates statistics for a set of wads or
* maps, such as min, max and average values and counts. A large number
* of values can be tracked, all defined by enum StatFields. The
* corresponding statLabels gives short titles for each field.
* DBStatistics itself is an abstract class, with MapStatistics being
* an implementation for map statistics, implementing processing
* and report generation.
*/

#ifndef MAPSTATISTICS_H
#define MAPSTATISTICS_H

#include "DataModel.h"
#include "../TextReport.h"

/*!
* Defines values to track with DBStatistics. The first part are
* integer values (unsigned long), while the last part are
* floating-point (double).
*/
enum StatFields {
	STS_MAPS, //!< Number of maps counted
	STS_WADS, //!< Number of wads counted

	//Wad fields
	STS_YEAR_MIN, //!< Lowest (valid) wad year value
	STS_YEAR_MAX, //!< Highest (valid) wad year value
	STS_WF_IWAD,
	STS_WF_SPRITES,
	STS_WF_TEX,
	STS_WF_GFX,
	STS_WF_COLOR,
	STS_WF_SOUND,
	STS_WF_MUSIC,
	STS_WF_DEHBEX,
	STS_WF_THINGS,
	STS_WF_SCRIPT,
	STS_WF_GLNODES,
	STS_OF_HAVEFILE, //!< Count of OwnFlags HAVEFILE

	//Map integer fields
	STS_SINGLE, //!< Maps "made for" singlePlayer
	STS_COOP, //!< Maps "made for" cooperative
	STS_DM, //!< Maps "made for" deathmatch
	STS_LINEDEFS, //!< Total linedefs
	STS_LINEDEFS_MIN,
	STS_LINEDEFS_MAX,
	STS_SECTORS, //!< Total sectors
	STS_SECTORS_MIN,
	STS_SECTORS_MAX,
	STS_THINGS, //!< Total things
	STS_THINGS_MIN,
	STS_THINGS_MAX,
	STS_SECRETS, //!< Total secrets
	STS_SECRETS_MIN,
	STS_SECRETS_MAX,
	STS_AREAS, //!< Number of entries with area
	STS_GAMESTATS, //!< Number of entries for enemies and rates
	STS_ENEMIES, //!< Total enemies
	STS_ENEMIES_MIN,
	STS_ENEMIES_MAX,
	STS_TOTALHP, //!< Sum of total health of all monsters
	STS_TOTALHP_MIN,
	STS_TOTALHP_MAX,
	//rating
	STS_MF_SPAWN, //!< Counted if has gamestats
	STS_MF_MORESPAWN, //!< Counted if has gamestats
	STS_MF_DIFFSET,
	STS_MF_VOODOO,
	STS_MF_UNKNOWN,
	STS_OWNRATED, //!< Number of rated entries
	STS_OWNRATING,
	STS_PLAYED, //!< Number of played>0
	//STS_PLAYTIME,
	STS_MAPS_MIN,
	STS_MAPS_MAX,
	STS_SIZE_MIN,
	STS_SIZE_MAX,

	//Floating-point fields
	STS_LINEDEFS_AVG,
	STS_SECTORS_AVG,
	STS_THINGS_AVG,
	STS_SECRETS_AVG,
	STS_ENEMIES_AVG,
	STS_TOTALHP_AVG,
	STS_HEALTHRAT_MIN,
	STS_HEALTHRAT_MAX,
	STS_HEALTHRAT_AVG,
	STS_ARMORRAT_MIN,
	STS_ARMORRAT_MAX,
	STS_ARMORRAT_AVG,
	STS_AMMORAT_MIN,
	STS_AMMORAT_MAX,
	STS_AMMORAT_AVG,
	STS_AREA_MIN,
	STS_AREA_MAX,
	STS_AREA_AVG,
	STS_OWNRATING_AVG,
	STS_MAPS_AVG,
	STS_SIZE, //!< Total file size in bytes
	STS_SIZE_AVG,

	STS_END
};

/*!
* Short names for each field of StatFields, useful for column
* headers in tables.
*/
const wxString statLabels[] = {
	"Maps",
	"Wads",
	"From",
	"To",
	"Iwad",
	"Sprites",
	"Textures",
	"Graphics",
	"Palette",
	"Sound",
	"Music",
	"DEH",
	"Things",
	"Scripts",
	"GL nodes",
	"Have file",
	"Single",
	"Coop",
	"DM",
	"Linedefs",
	"Min.linedefs",
	"Max.linedefs",
	"Sectors",
	"Min.sectors",
	"Max.sectors",
	"Things",
	"Min.things",
	"Max.things",
	"Secrets",
	"Min.Secrets",
	"Max.Secrets",
	"Has area",
	"Gamestats",
	"Enemies",
	"Min.enemies",
	"Max.enemies",
	"Hitpoints",
	"Min.hitpoints",
	"Max.hitpoints",
	"Enemy spawn",
	"Ex.spawn",
	"DiffSet",
	"Voodoo doll",
	"Unknown things",
	"Rated",
	"Rating",
	"Played",
	"Min.maps",
	"Max.maps",
	"Min.bytes",
	"Max.bytes",
	"Avg.linedefs",
	"Avg.sectors",
	"Avg.things",
	"Avg.Secrets",
	"Avg.enemies",
	"Avg.hitpoints",
	"Min.healthratio",
	"Max.healthratio",
	"Avg.healthratio",
	"Min.armorratio",
	"Max.armorratio",
	"Avg.armorratio",
	"Min.ammoratio",
	"Max.ammoratio",
	"Avg.ammoratio",
	"Min.area",
	"Max.area",
	"Avg.area",
	"Avg.rating%",
	"Avg.maps",
	"Bytes",
	"Avg.bytes"
};

/*! Initial value for integer minimum statistics. */
const unsigned long INVALID_MIN_INT = 1000000;

/*! Initial value for floating-point minimum statistics. */
const double INVALID_MIN_FLOAT = 1000000;

/*!
* Base class for statistics objects, which compute statistics
* for maps and/or wads. It defines methods for processing data
* and generating a report, to be implemented by concrete
* classes. The use the objects, call the process methods for
* each map/wad enyty, then computeResults to make the final
* computations. The results themselves are kept in two arrays,
* for integer and floating-point values, with the StatFields
* enum giving the indices.
*/
class DBStatistics
{
	public:
		/*!
		* A DBStatistics object needs a name, used as a heading
		* when displaying the statistics.
		*/
		DBStatistics(wxString name);

		virtual ~DBStatistics() {}

		/*!
		* To compute statistics from a set of MapEntry objects,
		* call this with each map.
		*/
		virtual void processMap(MapEntry* mapEntry) = 0;

		/*!
		* To compute statistics from a set of WadEntry objects,
		* call this with each wad.
		*/
		virtual void processWad(WadEntry* wadEntry) = 0;

		/*!
		* This must be called once all maps or wads have been
		* processed, to make final computations. Once done, the
		* results can be accessed in the arrays.
		*/
		virtual void computeResults() = 0;

		/*!
		* Outputs the statistics through the TextReport interface.
		*/
		virtual void printReport(TextReport* reportView) = 0;

		/*! A name for the DBStatistics, to show as a heading. */
		wxString heading;

		//unsigned long: 32 bit? 4,294,967,295
		//uint64_t: 18,446,744,073,709,551,615
		//double: ~15 digits accuracy

		/*!
		* Contains the integer statistics once computeResults
		* has been called. Indices are given by StatFields, up
		* to (but not including) STS_LINEDEFS_AVG. Note that an
		* implementation doesn't necessarily use all fields.
		*/
		unsigned long intStats[STS_LINEDEFS_AVG];

		/*!
		* Contains the floating-point statistics once
		* computeResults has been called. The size of the array
		* is the same as StatFields, but only entries from
		* STS_LINEDEFS_AVG and up are used, as the lower ones
		* are for integer values.
		*/
		double floatStats[STS_END];

	protected:
		/*! Set all integer minimum fields to a specific value. */
		void setIntMins(unsigned long minValue);

		/*! Set all floating-point minimum fields to a specific value. */
		void setFloatMins(double minValue);
};

/*!
* Represents and computes statistics for a set of maps.
* While the majority of the fields are computed from the data
* of MapEntry objects, some come from the WadEntry (but counted
* once for each map). Either call processMap for each map in
* the set, or processWad for each wad, which will process each
* map in the wad.
*/
class MapStatistics : public DBStatistics
{
	public:
		MapStatistics(wxString name);
		virtual ~MapStatistics();

		virtual void processMap(MapEntry* mapEntry);
		virtual void processWad(WadEntry* wadEntry);
		virtual void computeResults();
		virtual void printReport(TextReport* reportView);

	protected:

	private:
		/*! Adds statistics for a WadEntry, for a specified number of map entries. */
		void processWad(WadEntry* wadEntry, int maps);

		/*! Add statistics for the geometry fields of a MapEntry. */
		void processGeometry(MapEntry* mapEntry);

		/*! Add statistics for the gameplay fields of a MapEntry. */
		void processGameplay(MapEntry* mapEntry);
};

#endif // MAPSTATISTICS_H
