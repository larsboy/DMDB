/*!
* \file UdmfMapStats.h
* \author Lars Thomas Boye 2018
*
* MapStats subclass for the Universal Doom Map Format.
* Instead of being found in separate lumps, all primary
* map data is in a single TEXTMAP lump, in a text-based
* format. Nodes are in a ZNODES lump.
*/

#ifndef UDMFMAPSTATS_H
#define UDMFMAPSTATS_H

#include <math.h>
#include "MapStats.h"

/*!
* Specialization of MapStats, processing the lumps of maps
* in the Universal Doom Map Format.
*/
class UdmfMapStats : public MapStats
{
	public:
		UdmfMapStats(string name, EngineType eng, TaskProgress* wp);
		virtual ~UdmfMapStats();

		virtual void readFile(wxInputStream* file, vector<DirEntry*>* lumps, map<int, ThingDef*>* things);

	protected:
		/*! Parse the TEXTMAP lump. */
		void processTextmap(wxInputStream* file, int32_t lsize);

		/*! Called for each line of the TEXTMAP lump. */
		void processLine(wxString& line);

		/*! Find boolean value between '=' and ';'. */
		bool processFlag(wxString& line);

		/*! Find integer value between '=' and ';'. */
		long processInteger(wxString& line);

		/*! Find number between '=' and ';', round to nearest integer. */
		int processFloat(wxString& line);

		/*! Find string within quotes. */
		string processQuoted(wxString& line);

	private:
		int current; //1=thing,

		map<int, ThingDef*>* thingDefs;
		ThingDef* currentThing;
		uint16_t flags;
		ThingDef* unknownThing;
		ThingDef* friendlyThing;

		int16_t vertX, vertY;

		uint32_t v1, v2;
		bool twoSided;
};

#endif // UDMFMAPSTATS_H
