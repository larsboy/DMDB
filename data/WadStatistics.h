/*!
* \file WadStatistics.h
* \author Lars Thomas Boye 2018
*
* The WadStatistics object aggregates statistics for a set of wads,
* such as min, max and average values and counts. The values are
* defined by enum WadStatFields. The corresponding wadStatLabels gives
* short titles for each field.
*/

#ifndef WADSTATISTICS_H
#define WADSTATISTICS_H

#include "MapStatistics.h"

/*!
* Defines values to track with WadStatistics. Most are integer
* values (unsigned long), while the last two are floating-point
* (double).
*/
enum WadStatFields {
	WST_COUNT, //!< Number of items counted

	WST_YEAR_MIN, //!< Lowest (valid) wad year value
	WST_YEAR_MAX, //!< Highest (valid) wad year value
	WST_WF_IWAD,
	WST_WF_SPRITES,
	WST_WF_TEX,
	WST_WF_GFX,
	WST_WF_COLOR,
	WST_WF_SOUND,
	WST_WF_MUSIC,
	WST_WF_DEHBEX,
	WST_WF_THINGS,
	WST_WF_SCRIPT,
	WST_WF_GLNODES,
	WST_OF_HAVEFILE, //!< Count of OwnFlags HAVEFILE

	WST_MAPS,
	WST_MAPS_MIN,
	WST_MAPS_MAX,
	WST_OWNRATED, //!< Number of rated entries
	WST_OWNRATING,

	//Wad floating-point fields
	WST_MAPS_AVG,
	WST_OWNRATING_AVG,

	WST_END
};

/*!
* Short names for each field of WadStatFields, useful for column
* headers in tables.
*/
const wxString wadStatLabels[] = {
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
	"Maps",
	"Min.maps",
	"Max.maps",
	"Rated",
	"Rating",
	"Avg.maps",
	"Avg.rating%"
};

/*!
* Represents and computes statistics for a set of wads.
* Call processWad for each wad in the set, then computeResults
* to  make the final computations. The results themselves are
* kept in two arrays, for integer and floating-point values,
* with the WadStatFields enum giving the indices.
*/
class WadStatistics
{
	public:
		/*!
		* A WadStatistics object needs a name, used as a heading
		* when displaying the statistics.
		*/
		WadStatistics(wxString name);

		virtual ~WadStatistics();

		/*!
		* To compute statistics from a set of WadEntry objects,
		* call this with each wad.
		*/
		void processWad(WadEntry* wadEntry);

		/*!
		* This must be called once all wads have been processed,
		* to make final computations. Once done, the results can
		* be accessed in the arrays.
		*/
		void computeResults();

		/*!
		* Outputs the statistics through the TextReport interface.
		*/
		void printReport(TextReport* reportView);

		/*! A name for the WadStatistics, to show as a heading. */
		wxString heading;

		/*!
		* Contains the integer statistics once computeResults
		* has been called. Indices are given by WadStatFields,
		* and includes all below WST_MAPS_AVG.
		*/
		unsigned long intStats[WST_MAPS_AVG];

		/*!
		* Contains the floating-point statistics once
		* computeResults has been called. The size of the
		* array is the same as WadStatFields, but only entries
		* from WST_MAPS_AVG and up are used, as the lower
		* ones are for integer values.
		*/
		double floatStats[WST_END];

	protected:

	private:
		/*! Set all integer minimum fields to a specific value. */
		void setIntMins(unsigned long minValue);

		/*! Set all floating-point minimum fields to a specific value. */
		//void setFloatMins(double minValue);
};

#endif // WADSTATISTICS_H
