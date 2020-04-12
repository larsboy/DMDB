/*!
* \file HexenMapStats.h
* \author Lars Thomas Boye 2018
*
* MapStats subclass for the Hexen map format.
*/

#ifndef HEXENMAPSTATS_H
#define HEXENMAPSTATS_H

#include "MapStats.h"

/*!
* Specialization of MapStats, processing the map lumps of Hexen-format
* maps. In addition to the original Hexen engine, this map format is
* supported by ZDoom and derivatives, and was their primary map format
* for many years, for any type of Doom-engine game. Most lumps are the
* same as Doom's format, but THINGS and LINEDEFS have more bytes per
* entry.
*/
class HexenMapStats : public MapStats
{
	public:
		HexenMapStats(string name, EngineType eng, TaskProgress* wp);
		virtual ~HexenMapStats();

	protected:
		virtual void processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs);
		virtual void processLinedefs(wxInputStream* file, int32_t lsize);
		virtual void processSectors(wxInputStream* file, int32_t lsize);
};

#endif // HEXENMAPSTATS_H
