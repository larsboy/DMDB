/*!
* \file WadStatistics.h
* \author Lars Thomas Boye 2018
*
* WadStatistics is a DBStatistics to aggregate statistics for
* a set of wads, such as min, max and average values and counts.
*/

#ifndef WADSTATISTICS_H
#define WADSTATISTICS_H

#include "MapStatistics.h"

/*!
* Represents and computes statistics for a set of wads.
* Call processWad for each wad in the set, then computeResults
* to make the final computations. processMap is not used.
*/
class WadStatistics : public DBStatistics
{
	public:
		WadStatistics(wxString name);
		virtual ~WadStatistics();

		virtual void processMap(MapEntry* mapEntry) {}
		virtual void processWad(WadEntry* wadEntry);
		virtual void computeResults();
		virtual void printReport(TextReport* reportView);
};

#endif // WADSTATISTICS_H
