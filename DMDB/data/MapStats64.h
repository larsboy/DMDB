/*!
* \file HexenMapStats.h
* \author Lars Thomas Boye 2018
*
* MapStats subclass for the Hexen map format.
*/

#ifndef MAPSTATS64_H
#define MAPSTATS64_H

#include "MapStats.h"

class MapStats64 : public MapStats
{
	public:
		MapStats64(string name, EngineType eng, TaskProgress* wp);
		virtual ~MapStats64();

	protected:
		virtual void processThings(wxInputStream* file, int32_t lsize, map<int, ThingDef*>* thingDefs);
		virtual void processLinedefs(wxInputStream* file, int32_t lsize);
		virtual void processSidedefs(wxInputStream* file, int32_t lsize);
		virtual void processVertexes(wxInputStream* file, int32_t lsize);
		virtual void processSectors(wxInputStream* file, int32_t lsize);
};

#endif // MAPSTATS64_H
